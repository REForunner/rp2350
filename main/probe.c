/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
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

#include <pico/stdlib.h>
#include <stdio.h>
#include <string.h>

#include <hardware/clocks.h>
#include <hardware/gpio.h>

#include "probe.h"
#include "tusb.h"
#include "rp2350.h"


#define DIV_ROUND_UP(m, n)	(((m) + (n) - 1) / (n))

// Only want to set / clear one gpio per event so go up in powers of 2
enum _dbg_pins {
    DBG_PIN_WRITE = 1,
    DBG_PIN_WRITE_WAIT = 2,
    DBG_PIN_READ = 4,
    DBG_PIN_PKT = 8,
};

CU_REGISTER_DEBUG_PINS(probe_timing)

// Uncomment to enable debug
//CU_SELECT_DEBUG_PINS(probe_timing)

#define PROBE_BUF_SIZE 8192
struct _probe {
    // PIO offset
    uint offset;
    uint initted;
};

static struct _probe probe;

void probe_set_swclk_freq(PIO pio, uint sm, uint freq_khz) {
        uint clk_sys_freq_khz = clock_get_hz(clk_sys) / 1000;
        probe_info("Set swclk freq %dKHz sysclk %dkHz\n", freq_khz, clk_sys_freq_khz);
        // Round up (otherwise fast swclks get faster)
        uint32_t divider = (((clk_sys_freq_khz + freq_khz - 1)/ freq_khz) + 3) / 4;
        if (divider == 0)
            divider = 1;
        if (divider > 65535)
            divider = 65535;

        pio_sm_set_clkdiv_int_frac(pio, sm, divider << 1, 0);
}

typedef enum probe_pio_command {
    CMD_WRITE = 0,
    CMD_SKIP,
    CMD_TURNAROUND,
    CMD_READ
} probe_pio_command_t;

static inline uint32_t fmt_probe_command(uint bit_count, bool out_en, probe_pio_command_t cmd) {
    uint cmd_addr =
        cmd == CMD_WRITE      ? probe.offset + probe_offset_write_cmd :
        cmd == CMD_SKIP       ? probe.offset + probe_offset_get_next_cmd :
        cmd == CMD_TURNAROUND ? probe.offset + probe_offset_turnaround_cmd :
                                probe.offset + probe_offset_read_cmd;
    return ((bit_count - 1) & 0xff) | ((uint)out_en << 8) | (cmd_addr << 9);
}

void probe_write_bits(PIO pio, uint sm, uint bit_count, uint32_t data_byte) {
    DEBUG_PINS_SET(probe_timing, DBG_PIN_WRITE);
    pio_sm_put_blocking(pio, sm, fmt_probe_command(bit_count, true, CMD_WRITE));
    pio_sm_put_blocking(pio, sm, data_byte);
    probe_dump("Write %d bits 0x%x\n", bit_count, data_byte);
    // Return immediately so we can cue up the next command whilst this one runs
    DEBUG_PINS_CLR(probe_timing, DBG_PIN_WRITE);
}

void probe_hiz_clocks(PIO pio, uint sm, uint bit_count) {
    pio_sm_put_blocking(pio, sm, fmt_probe_command(bit_count, false, CMD_TURNAROUND));
    pio_sm_put_blocking(pio, sm, 0);
}

uint32_t probe_read_bits(PIO pio, uint sm, uint bit_count) {
    DEBUG_PINS_SET(probe_timing, DBG_PIN_READ);
    pio_sm_put_blocking(pio, sm, fmt_probe_command(bit_count, false, CMD_READ));
    uint32_t data = pio_sm_get_blocking(pio, sm);
    uint32_t data_shifted = data;
    if (bit_count < 32) {
        data_shifted = data >> (32 - bit_count);
    }

    probe_dump("Read %d bits 0x%x (shifted 0x%x)\n", bit_count, data, data_shifted);
    DEBUG_PINS_CLR(probe_timing, DBG_PIN_READ);
    return data_shifted;
}

static void probe_wait_idle(PIO pio, uint sm) {
    pio->fdebug = 1u << (PIO_FDEBUG_TXSTALL_LSB + sm);
    while (!(pio->fdebug & (1u << (PIO_FDEBUG_TXSTALL_LSB + sm))))
        ;
}

void probe_read_mode(PIO pio, uint sm) {
    pio_sm_put_blocking(pio, sm, fmt_probe_command(0, false, CMD_SKIP));
    probe_wait_idle(pio, sm);
}

void probe_write_mode(PIO pio, uint sm) {
    pio_sm_put_blocking(pio, sm, fmt_probe_command(0, true, CMD_SKIP));
    probe_wait_idle(pio, sm);
}

void probe_init(PIO pio, uint * sm, uint pinBase) {
    if (!probe.initted) {
        probe_gpio_init(pio, pinBase);
        uint offset = pio_add_program(pio, &probe_program);
        probe.offset = offset;
        * sm = pio_claim_unused_sm(pio, true);

        pio_sm_config sm_config = probe_program_get_default_config(offset);
        probe_sm_init(pio, *sm, pinBase, &sm_config);
        pio_sm_init(pio, *sm, offset, &sm_config);

        // Set up divisor
        probe_set_swclk_freq(pio, *sm, 1000);

        // Jump SM to command dispatch routine, and enable it
        pio_sm_exec(pio, *sm, offset + probe_offset_get_next_cmd);
        pio_sm_set_enabled(pio, *sm, 1);
        probe.initted = 1;
    }
}

void probe_deinit(PIO pio, uint sm, uint pinBase)
{
  if (probe.initted) {
    probe_read_mode(pio, sm);
    pio_sm_set_enabled(pio, sm, 0);
    pio_remove_program(pio, &probe_program, probe.offset);

    // de-assert nRESET
    probe_gpio_deinit(pinBase);
    probe.initted = 0;
  }
}
