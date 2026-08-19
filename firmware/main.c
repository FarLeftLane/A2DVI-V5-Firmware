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

#include <string.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include "pico/multicore.h"
#include "hardware/vreg.h"
#include <tusb.h>
#include "hardware/structs/sysinfo.h"
#include <hardware/watchdog.h>
#include "pico/bootrom.h"

#include "dvi/a2dvi.h"
#include "applebus/abus.h"
#include "applebus/buffers.h"
#include "util/dmacopy.h"
#include "config/config.h"
#include "menu/menu.h"
#include "debug/debug.h"

#include "fonts/textfont.h"

#ifdef FEATURE_TEST
    #include "test/tests.h"
    #define FUNCTION_PROFILER
#endif

#include "debug/profiler.h"

#define VREG_VSEL         VREG_VOLTAGE_1_20

void assert_processor_type(void)
{
    io_ro_32 package = sysinfo_hw->package_sel;

    if ((package & SYSINFO_PACKAGE_SEL_BITS) != 1)      //  0 = QFN80, 1 = QFN60
    {
        //  Wrong processor
        printf("Wrong processor!\n");            
        sleep_ms(1*1000);
        
        //  Reboot into BOOTSEL
        rom_reset_usb_boot_extra(-1, 0, false);
        // does not return, otherwise we'd return true
    }
}

int main()
{
    PROFILER_INIT(boot_time);

    // enable LED etc
    debug_init();

    //  Enable to reboot without BOOTSEL button
    stdio_init_all();                               //  ??? Turn off for abus or UART?

    //  Check to make sure we are on the right processor for V5
    assert_processor_type();

#if 0                           //  ??? Disabled
    // check hardware
    a2dvi_check_hardware();
#endif

    // load config settings
    config_load();

#ifdef FEATURE_TEST
    // start testsuite, simulating some 6502 activity and
    // cycle through the test cases
    multicore_launch_core1(test_loop);
#else
    // process the Apple II bus interface on core 1
    multicore_launch_core1(abus_loop);
#endif
    boot_time = to_us_since_boot(get_absolute_time());

    // Finish copying remaining data and code from flash to RAM
    memcpy32(__ram_delayed_copy_start__, __ram_delayed_copy_source__, ((uint32_t)__ram_delayed_copy_end__) - (uint32_t) __ram_delayed_copy_start__);

    // DVI processing on core 0
    a2dvi_loop();
}
