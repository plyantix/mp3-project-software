/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

 // \.pico-sdk\sdk\2.2.0\lib\tinyusb\src\class\msc\msc_device.h

#include "bsp/board_api.h"
#include "tusb.h"
#include "hw_config.h"
#include "ff.h"
// #include "gfx.h"
#define GFX_printf(...) (void) 0

#define DISK_BLOCK_SIZE 512 // should probably replace with a way to dynamically detect but should usually be 512

// whether host does safe-eject
static bool ejected = false;


// Invoked when received SCSI_CMD_INQUIRY, v2 with full inquiry response
// Some inquiry_resp's fields are already filled with default values, application can update them
// Return length of inquiry response, typically sizeof(scsi_inquiry_resp_t) (36 bytes), can be longer if included vendor data.
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]) {
    (void)lun;
    const char vid[] = "TinyUSB";
    const char pid[] = "Mass Storage";
    const char rev[] = "1.0";

    (void)strncpy((char*)vendor_id, vid, 8);
    (void)strncpy((char*)product_id, pid, 16);
    (void)strncpy((char*)product_rev, rev, 4);

    GFX_printf("'tud_msc_inquiry_cb' called!\n");
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun) {
    (void)lun;
    GFX_printf("'tud_msc_test_unit_ready_cb' called. Fetching disk status.\n");

    DSTATUS status = disk_status(0);

    GFX_printf("Disk status: %x\n", status);

    // RAM disk is ready until ejected
    if (status != RES_OK || ejected) {
        // Additional Sense 3A-00 is NOT_FOUND
        return tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00);
    }

    return true;
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size) {
    (void)lun;

    GFX_printf("'tud_msc_capacity_cb' called. Fetching disk capacity info.\n");

    int err;
    err = disk_ioctl(0, GET_SECTOR_COUNT, block_count);
    GFX_printf("Sector count error: %x, count: %d\n", err, block_count);

    // err = disk_ioctl(0, GET_SECTOR_SIZE, &sector_size);
    // GFX_printf("Sector size error: %x, size: %d\n", err, sector_size);

    // For some reason GET_SECTOR_SIZE isn't implemented despite existing in the documentation and the enum
    // So we will just assume it's 512 (it's usually 512). 
    *block_size = 512;
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject) {
    (void)lun;
    (void)power_condition;

    if (load_eject) {
        if (start) {
            // load disk storage
        }
        else {
            // unload disk storage
            // f_unmount("");
            ejected = true;
        }
    }
    GFX_printf("'tud_msc_start_stop_cb' called with start=%d and load_eject=%d\n", start, load_eject);

    return true;
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    (void)lun;
    // GFX_printf("'tud_msc_read10_cb' called at lba=%d, offset=%d, bufsize=%d\n", lba, offset, bufsize);


    // who needs overflow protection anyways tbh
    // // out of ramdisk
    // if (lba >= DISK_BLOCK_NUM) {
    //     return -1;
    // }

    // // Check for overflow of offset + bufsize
    // if (lba * DISK_BLOCK_SIZE + offset + bufsize > DISK_BLOCK_NUM * DISK_BLOCK_SIZE) {
    //     return -1;
    // }

    
    // uint8_t read_buffer[DISK_BLOCK_SIZE];
    // should probably check if this returns an error
    disk_read(0, buffer, lba, bufsize / DISK_BLOCK_SIZE);

    // uint8_t const* addr = read_buffer + offset;
    // memcpy(buffer, addr, DISK_BLOCK_SIZE - offset);

    // for (int i = 0; i < DISK_BLOCK_SIZE; i++) {
    //     GFX_printf("%02x ", ((char*) buffer)[i]);
    // }
    GFX_printf("\n");

    return (bufsize / DISK_BLOCK_SIZE) * DISK_BLOCK_SIZE;
}

bool tud_msc_is_writable_cb(uint8_t lun) {
    (void)lun;

    return true;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    (void)lun;
    // GFX_printf("'tud_msc_write10_cb' called at lba=%d, offset=%d, bufsize=%d\n", lba, offset, bufsize);

    // // out of ramdisk
    // if (lba >= DISK_BLOCK_NUM) {
    //     return -1;
    // }

    // I ain't implementing the logic for non-zero offsets
    // If project is configured correctly that should never happen 
    if (offset != 0) {
        return -1;
    }

    disk_write(0, buffer, lba, bufsize / DISK_BLOCK_SIZE);

    return (bufsize / DISK_BLOCK_SIZE) * DISK_BLOCK_SIZE;
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize) {
    (void)lun;
    (void)scsi_cmd;
    (void)buffer;
    (void)bufsize;

    // currently no other commands are supported

    // Set Sense = Invalid Command Operation
    (void)tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

    return -1; // stall/failed command request;
}