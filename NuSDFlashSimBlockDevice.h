/*
 * Copyright (c) 2021, Nuvoton Technology Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __NU_SD_FLASH_SIM_BLOCK_DEVICE_H__
#define __NU_SD_FLASH_SIM_BLOCK_DEVICE_H__

#if TARGET_NUVOTON

#include "mbed.h"
#include "NuSDBlockDevice.h"
/*
 * Since mbed-os 5.14.0, rtos::Mutex has enabled to support both mbed-os
 * and mbed-baremetal, and PlatformMutex becomes unneeded.
 */
#if MBED_MAJOR_VERSION >= 6
#include "rtos/Mutex.h"
#else
#include "platform/PlatformMutex.h"
#endif

/* Simulate, based on NuSDBlockDevice, as flash block device
 *
 * 1. Support not program 0 to 1
 * 2. Erase actually takes effect to 0xFF
 * 3. Support configuring erase unit size (default to 4096)
 * 4. Support 1-byte program unit
 * 5. Support 1-byte read unit
 */
class NuSDFlashSimBlockDevice : public NuSDBlockDevice {
public:
    /** Lifetime of an SD card
     */
    NuSDFlashSimBlockDevice();
    NuSDFlashSimBlockDevice(PinName sd_dat0, PinName sd_dat1, PinName sd_dat2, PinName sd_dat3,
        PinName sd_cmd, PinName sd_clk, PinName sd_cdn);
    virtual ~NuSDFlashSimBlockDevice();

    /** Read blocks from a block device
     *
     *  @param buffer   Buffer to write blocks to
     *  @param addr     Address of block to begin reading from
     *  @param size     Size to read in bytes, must be a multiple of read block size
     *  @return         0 on success, negative error code on failure
     */
    virtual int read(void *buffer, bd_addr_t addr, bd_size_t size);

    /** Program blocks to a block device
     *
     *  The blocks must have been erased prior to being programmed
     *
     *  @param buffer   Buffer of data to write to blocks
     *  @param addr     Address of block to begin writing to
     *  @param size     Size to write in bytes, must be a multiple of program block size
     *  @return         0 on success, negative error code on failure
     */
    virtual int program(const void *buffer, bd_addr_t addr, bd_size_t size);

    /** Erase blocks on a block device
     *
     *  The state of an erased block is undefined until it has been programmed
     *
     *  @param addr     Address of block to begin erasing
     *  @param size     Size to erase in bytes, must be a multiple of erase block size
     *  @return         0 on success, negative error code on failure
     */
    virtual int erase(bd_addr_t addr, bd_size_t size);

    /** Get the size of a readable block
     *
     *  @return         Size of a readable block in bytes
     */
    virtual bd_size_t get_read_size() const;

    /** Get the size of a programable block
     *
     *  @return         Size of a programable block in bytes
     *  @note Must be a multiple of the read size
     */
    virtual bd_size_t get_program_size() const;

    /** Get the size of an erasable block
     *
     *  @return         Size of an erasable block in bytes
     *  @note Must be a multiple of the program size
     */
    virtual bd_size_t get_erase_size() const;

    /** Get the size of an erasable block given address
     *
     *  @param addr     Address within the erasable block
     *  @return         Size of an erasable block in bytes
     *  @note Must be a multiple of the program size
     */
    virtual bd_size_t get_erase_size(bd_addr_t addr) const;

    /** Get the BlockDevice class type.
     *
     *  @return         A string representation of the BlockDevice class type.
     */
    virtual const char *get_type() const;

private:
#if MBED_MAJOR_VERSION >= 6
    rtos::Mutex _lock;
#else
    PlatformMutex _lock;
#endif
};

#endif  /* TARGET_NUVOTON */
#endif  /* __NU_SD_FLASH_SIM_BLOCK_DEVICE_H__ */
