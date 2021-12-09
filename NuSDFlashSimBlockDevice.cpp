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

/* Nuvoton mbed enabled targets which support SD card of SD bus mode */
#if TARGET_NUVOTON

#include "NuSDFlashSimBlockDevice.h"
#include "mbed_toolchain.h"

/* Emulate flash program attribute: 0 doesn't program to 1 */
#define NU_SDH_FLASH_PROG_ATR       MBED_CONF_NUSD_FLASHSIM_PROGRAM_ATTRIBUTE

/* Emulate flash erase unit of specified size */
#define NU_SDH_FLASH_SECTOR_SIZE    MBED_CONF_NUSD_FLASHSIM_ERASE_UNIT_SIZE

/* SD card sector size */
#define SDH_SECTOR_SIZE     512

/* SDH DMA compatible buffer
 *
 * SDH DMA buffer location requires to be:
 * (1) Word-aligned
 * (2) Located in 0x2xxxxxxx/0x3xxxxxxx region. Check linker files to ensure global/static
 *     variables are placed in this region.
 *
 * SDH DMA buffer size DMA_BUF_SIZE must be a multiple of 512-byte block size.
 * Its value is estimated to trade memory footprint off against performance.
 *
 */
#define DMA_BUFF_SIZE       SDH_SECTOR_SIZE
MBED_ALIGN(4) static uint8_t dma_buff[DMA_BUFF_SIZE];

/* SDH Flash sector size must be a multiple of SDH sector size. */
static_assert((NU_SDH_FLASH_SECTOR_SIZE % SDH_SECTOR_SIZE) == 0,
              "NU_SDH_FLASH_SECTOR_SIZE must a multiple of SDH_SECTOR_SIZE");
/* DMA buffer size must be a multiple of SDH sector size. */
static_assert((DMA_BUFF_SIZE % SDH_SECTOR_SIZE) == 0,
              "DMA_BUFF_SIZE must a multiple of SDH_SECTOR_SIZE");

NuSDFlashSimBlockDevice::NuSDFlashSimBlockDevice() :
    NuSDBlockDevice()
{
}

NuSDFlashSimBlockDevice::NuSDFlashSimBlockDevice(PinName sd_dat0, PinName sd_dat1, PinName sd_dat2, PinName sd_dat3,
                                                 PinName sd_cmd, PinName sd_clk, PinName sd_cdn) :
    NuSDBlockDevice(sd_dat0, sd_dat1, sd_dat2, sd_dat3, sd_cmd, sd_clk, sd_cdn)
{
}

NuSDFlashSimBlockDevice::~NuSDFlashSimBlockDevice()
{
}

int NuSDFlashSimBlockDevice::program(const void *b, bd_addr_t addr, bd_size_t size)
{
    _lock.lock();
    int err = BD_ERROR_OK;

    /* For easy implementation, we always use SDH DMA-compatible buffer for intermediate. */
    const uint8_t *b_pos = (const uint8_t *) b;
    bd_addr_t addr_pos = addr;
    bd_size_t rmn = size;

    while (rmn) {
        uint32_t sector_offset = addr_pos % SDH_SECTOR_SIZE;
        uint32_t todo_size = DMA_BUFF_SIZE - sector_offset;
        todo_size = (todo_size >= rmn) ? rmn : todo_size;
        /* [begin_sector, end_sector) */
        uint32_t begin_sector = addr_pos / SDH_SECTOR_SIZE;
        uint32_t end_sector = (addr_pos + todo_size - 1 + SDH_SECTOR_SIZE) / SDH_SECTOR_SIZE;

        err = NuSDBlockDevice::read(dma_buff, begin_sector * SDH_SECTOR_SIZE, (end_sector - begin_sector) * SDH_SECTOR_SIZE);
        if (err != BD_ERROR_OK) {
            return err;
        }

#if NU_SDH_FLASH_PROG_ATR
        {
            const uint8_t *b_pos2 = b_pos;
            const uint8_t *b_end2 = b_pos + todo_size;
            uint8_t *dma_buff_pos = dma_buff + sector_offset;
            while (b_pos2 < b_end2) {
                *dma_buff_pos &= *b_pos2;
                dma_buff_pos ++;
                b_pos2 ++;
            }
        }
#else
        memcpy(dma_buff + sector_offset, b_pos, todo_size);
#endif

        err = NuSDBlockDevice::program(dma_buff, begin_sector * SDH_SECTOR_SIZE, (end_sector - begin_sector) * SDH_SECTOR_SIZE);
        if (err != BD_ERROR_OK) {
            return err;
        }

        b_pos += todo_size;
        addr_pos += todo_size;
        rmn -= todo_size;
    }

    _lock.unlock();

    return err;
}

int NuSDFlashSimBlockDevice::read(void *b, bd_addr_t addr, bd_size_t size)
{
    _lock.lock();
    int err = BD_ERROR_OK;

    /* For easy b, we always use SDH DMA-compatible buffer for intermediate. */
    uint8_t *b_pos = (uint8_t *) b;
    bd_addr_t addr_pos = addr;
    bd_size_t rmn = size;

    while (rmn) {
        uint32_t sector_offset = addr_pos % SDH_SECTOR_SIZE;
        uint32_t todo_size = DMA_BUFF_SIZE - sector_offset;
        todo_size = (todo_size >= rmn) ? rmn : todo_size;
        /* [begin_sector, end_sector) */
        uint32_t begin_sector = addr_pos / SDH_SECTOR_SIZE;
        uint32_t end_sector = (addr_pos + todo_size - 1 + SDH_SECTOR_SIZE) / SDH_SECTOR_SIZE;

        err = NuSDBlockDevice::read(dma_buff, begin_sector * SDH_SECTOR_SIZE, (end_sector - begin_sector) * SDH_SECTOR_SIZE);
        if (err != BD_ERROR_OK) {
            return err;
        }

        memcpy(b_pos, dma_buff + sector_offset, todo_size);

        b_pos += todo_size;
        addr_pos += todo_size;
        rmn -= todo_size;
    }

    _lock.unlock();

    return err;
}

int NuSDFlashSimBlockDevice::erase(bd_addr_t addr, bd_size_t size)
{
    _lock.lock();
    int err = BD_ERROR_OK;

    memset(dma_buff, 0xFF, SDH_SECTOR_SIZE);

    /* Simulate flash sector erase */
    bd_addr_t addr_pos = addr / NU_SDH_FLASH_SECTOR_SIZE * NU_SDH_FLASH_SECTOR_SIZE;
    bd_addr_t addr_end = (addr_pos + size + NU_SDH_FLASH_SECTOR_SIZE - 1) / NU_SDH_FLASH_SECTOR_SIZE * NU_SDH_FLASH_SECTOR_SIZE;
    for (; addr_pos < addr_end; addr_pos += SDH_SECTOR_SIZE) {
        err = NuSDBlockDevice::program(dma_buff, addr_pos, SDH_SECTOR_SIZE);
        if (err != BD_ERROR_OK) {
            return err;
        }
    }

    _lock.unlock();

    return err;
}

bd_size_t NuSDFlashSimBlockDevice::get_read_size() const
{
    return 1;
}

bd_size_t NuSDFlashSimBlockDevice::get_program_size() const
{
    return 1;
}

bd_size_t NuSDFlashSimBlockDevice::get_erase_size() const
{
    return NU_SDH_FLASH_SECTOR_SIZE;
}

bd_size_t NuSDFlashSimBlockDevice::get_erase_size(bd_addr_t addr) const
{
    return NU_SDH_FLASH_SECTOR_SIZE;
}

const char *NuSDFlashSimBlockDevice::NuSDFlashSimBlockDevice::get_type() const
{
    return "NUSD_FLASHSIM";
}

#endif  /* TARGET_NUVOTON */
