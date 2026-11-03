/*
MIT License

Copyright (c) 2021 Mark Aikens
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

#include <string.h>
#include <hardware/pio.h>
#include "abus_pin_config.h"
#include "abus_setup.h"
#include "abus.pio.h"
#include "dvi/a2dvi.h"
#include "debug/debug.h"


typedef struct a2_sm
{
    uint          offset;
    pio_sm_config config;
} a2_sm_t;

static a2_sm_t s_addr_sm;
static a2_sm_t s_write_sm;


void a2dvi_check_hardware(void)
{
    //  ??? Is there anything we can check?
}

void abus_pio_setup(void)
{
    //  The addr PIO is offset to a base of 16
    pio_set_gpio_base(CONFIG_ABUS_ADDR_PIO,  16);

    //  The data PIO is base 0
    pio_set_gpio_base(CONFIG_ABUS_DATA_PIO,  0);

#if 0
    //  Invert these active low signals to be more clear in other code
    gpio_set_inover(DEVSEL_PIN, GPIO_OVERRIDE_INVERT);
    gpio_set_inover(IOSEL_PIN, GPIO_OVERRIDE_INVERT);
    gpio_set_inover(IOSTRB_PIN, GPIO_OVERRIDE_INVERT);
    gpio_set_inover(M2SEL_PIN, GPIO_OVERRIDE_INVERT);
#endif

#if APPLEIIGS
    //  Setup the addr SM for the IIgs
    s_addr_sm.offset = pio_add_program(CONFIG_ABUS_ADDR_PIO, &abus_iigs_addr_program);
    s_addr_sm.config = abus_iigs_addr_program_get_default_config(s_addr_sm.offset);
    abus_iigs_addr_program_set_config(CONFIG_ABUS_ADDR_PIO, ABUS_ADDR_SM, &s_addr_sm.config);
    pio_sm_claim(CONFIG_ABUS_ADDR_PIO, ABUS_ADDR_SM);
#else
    //  Setup the addr SM
    s_addr_sm.offset = pio_add_program(CONFIG_ABUS_ADDR_PIO, &abus_addr_program);
    s_addr_sm.config = abus_addr_program_get_default_config(s_addr_sm.offset);
    abus_addr_program_set_config(CONFIG_ABUS_ADDR_PIO, ABUS_ADDR_SM, &s_addr_sm.config);
    pio_sm_claim(CONFIG_ABUS_ADDR_PIO, ABUS_ADDR_SM);
#endif

    //  Setup the data write SM
    s_write_sm.offset = pio_add_program(CONFIG_ABUS_DATA_PIO, &abus_write_program);
    s_write_sm.config = abus_write_program_get_default_config(s_write_sm.offset);
    abus_write_program_set_config(CONFIG_ABUS_DATA_PIO, ABUS_DATA_SM, &s_write_sm.config);
    pio_sm_claim(CONFIG_ABUS_DATA_PIO, ABUS_DATA_SM);

    //  Init the SMs
    pio_sm_init(CONFIG_ABUS_ADDR_PIO, ABUS_ADDR_SM, s_addr_sm.offset, &s_addr_sm.config);
    pio_sm_init(CONFIG_ABUS_DATA_PIO, ABUS_DATA_SM, s_write_sm.offset, &s_write_sm.config);

    //  Start data and then addr
    pio_sm_set_enabled(CONFIG_ABUS_DATA_PIO, ABUS_DATA_SM, true);
    pio_sm_set_enabled(CONFIG_ABUS_ADDR_PIO, ABUS_ADDR_SM, true);

#if 0
    //  ??? Is this needed?
    // Disable input synchronization on input pins that are sampled at known stable times
    // to shave off two clock cycles of input latency
    pio->input_sync_bypass |= (0x3ff << CONFIG_PIN_APPLEBUS_DATA_BASE);
#endif
}
