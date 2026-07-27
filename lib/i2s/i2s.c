#include "pico/stdlib.h"

#include "i2s.h"

#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "audio_i2s.pio.h"
#include "hardware/clocks.h"

static uint8_t __attribute__((aligned(4))) buff_0[AUDIO_BUFFER_SIZE];
static uint8_t __attribute__((aligned(4))) buff_1[AUDIO_BUFFER_SIZE];
static bool active_buff;
static PIO i2s_pio;
static uint i2s_sm;
static uint i2s_offset;
static uint i2s_dma;
static i2s_callback_t i2s_callback;

static inline uint8_t *get_buffer(bool n) {
    return n ? buff_1 : buff_0;
}

static void __isr __time_critical_func(dma_handler()) {
    // Called when the dma runs out of data to transfer
    active_buff = !active_buff;
    dma_channel_transfer_from_buffer_now(i2s_dma, get_buffer(active_buff), AUDIO_BUFFER_SIZE / 4);
    if (dma_irqn_get_channel_status(0, i2s_dma)) {
        dma_irqn_acknowledge_channel(0, i2s_dma);
    }
    // Start filling the inactive buffer
    // hopefully this will finish running before this irq is triggered again...
    i2s_callback(get_buffer(active_buff), AUDIO_BUFFER_SIZE);
}


void i2s_set_sample_rate(uint32_t rate) {
    uint32_t system = clock_get_hz(clk_sys);
    // The pio outputs a bit every 2 clock cycles
    // With stereo audio, one sample is BITDEPTH * 2 bits
    
    float div = (float) system / rate / 2 / (BITDEPTH * 2); 

    pio_sm_set_clkdiv(i2s_pio, i2s_sm, div);
}

// Initializes the i2s program on the given pio.
// data_pin is DIN, clock_pin is BCK, and clock_pin+1 is WSEL
// rate is sample rate in hz, can be changed later.
// Interrupts will be occationally trigger on the core this is called on, triggering the provided callback.
void i2s_init(PIO pio, uint data_pin, uint clock_pin, i2s_callback_t callback, uint32_t rate) {
    i2s_pio = pio;
    i2s_callback = callback;
    i2s_offset = 0;
    i2s_sm = 0;
    // PIO:

    // must be at offset 0 (pio_claim_free_sm_and_add_program places it at offset 24 which causes it to not work)
    i2s_offset = pio_add_program_at_offset(pio, &audio_i2s_program, 0);
    pio_sm_claim(pio, 0);

    audio_i2s_program_init(pio, i2s_sm, i2s_offset, data_pin, clock_pin);
    
    pio_gpio_init(pio, data_pin);
    pio_gpio_init(pio, clock_pin);
    pio_gpio_init(pio, clock_pin+1);

    i2s_set_sample_rate(rate);

    pio_sm_set_enabled(pio, i2s_sm, true);

    // DMA:
    i2s_dma = dma_claim_unused_channel(true);

    dma_channel_config_t dma_config = dma_channel_get_default_config(i2s_dma);

    channel_config_set_dreq(&dma_config, pio_get_dreq(pio, i2s_sm, true));

    dma_channel_configure(
            i2s_dma,
            &dma_config,
            &pio->txf[i2s_sm], // Write to the state machine's output buffer
            NULL, // Read address
            0, // Transfer count
            false // Start immediently
    );

    dma_channel_set_irq0_enabled(i2s_dma, true);

    irq_set_exclusive_handler(dma_get_irq_num(0), dma_handler);

    irq_set_enabled(dma_get_irq_num(0), true);

    i2s_callback(get_buffer(0), AUDIO_BUFFER_SIZE);
    active_buff = 0;
    dma_handler();
}
