/*
MIT License

Copyright (c) 2024 Thorsten Brehm

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <hardware/pio.h>

// use PIO 0 (PIO1 is used for DVI)
#define CONFIG_ABUS_ADDR_PIO pio0       //  Base 16
#define CONFIG_ABUS_DATA_PIO pio2       //  Base 0

// statemachines (0-3)
#define ABUS_ADDR_SM    0               //  PIO 0
#define ABUS_DATA_SM    0               //  PIO 2

#define abus_pio_addr_fifo_level()       (pio_sm_get_rx_fifo_level(CONFIG_ABUS_ADDR_PIO, ABUS_ADDR_SM))
#define abus_pio_addr_is_full()          (pio_sm_is_rx_fifo_full(CONFIG_ABUS_ADDR_PIO, ABUS_ADDR_SM))
#define abus_pio_addr_is_empty()         (pio_sm_is_rx_fifo_empty(CONFIG_ABUS_ADDR_PIO, ABUS_ADDR_SM))

#define abus_pio_addr_read()             (pio_sm_get(CONFIG_ABUS_ADDR_PIO, ABUS_ADDR_SM))
#define abus_pio_addr_blocking_read()    (pio_sm_get_blocking(CONFIG_ABUS_ADDR_PIO, ABUS_ADDR_SM))


#define abus_pio_data_blocking_read()    (pio_sm_get_blocking(CONFIG_ABUS_DATA_PIO, ABUS_DATA_SM))

void abus_pio_setup(void);
void abus_enable_gs(void);

