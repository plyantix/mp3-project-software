// Pins for MiniMusicBox v0.1

// Battery progress
// Input
// Must be pulled up
// Low: battery is charging
// High: battery is not charging (either no battery present, thermal shutdown, or battery fully charged)
#define BATTERY_STAT 26

// Battery monitor
// Analog in
// Voltage of the battery through a 4.2:3 voltage divider
#define BATTERY_MONITOR 27

// Switch 2
// Left button
#define SW_2 19

// Switch 3
// Middle button
#define SW_3 15

// Switch 4
// Right button
#define SW_4 14

// Switch 5
// Bottom side button
#define SW_5 22

// Switch 6
// Top side button
#define SW_6 21

// SPI interface for micro sd card
#define SD_SPI spi0
#define SD_RX 4
#define SD_CS 5
#define SD_SCK 6
#define SD_TX 7

// SPI interface for display
#define LCD_SPI spi1
#define LCD_RX 8
#define LCD_CS 9
#define LCD_SCK 10
#define LCD_TX 11
// Additional inputs for display
// Function select (issue command or write to ddram)
#define LCD_A0 12
// Active low reset
#define LCD_RST 13

// Display backlight
// Digital/PWM output
#define LCD_BACKLIGHT 20

// PTH connections for debugging/expansions
// Pin 1 is +3V3 and pin 6 is ground
#define BREAKOUT_2 23
#define BREAKOUT_3 24
#define BREAKOUT_4 25
#define BREAKOUT_5 29

// I2S connection to the DAC
#define AUDIO_DATA 1
#define AUDIO_BCK 2
#define AUDIO_LRCLK 3

// Active low mute signal for DAC and amp
#define MUTE 0