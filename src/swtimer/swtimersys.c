/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2020-2022, Tuomas Terho
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "swtimersys.h"

#include <assert.h>

/**
 * @file swtimersys.c
 *
 * @defgroup sw-timer-system-internals Internals
 * @ingroup  sw-timer-system
 *
 * @{
 */

/// Minimum bits in a timer.
#define TIMER_BITS_MIN 2u
/// Maximum bits in a timer.
#define TIMER_BITS_MAX 64u

/**
 * @brief Create a mask for the given amount of bits.
 *
 * @param[in] bits Bit count in the mask.
 *
 * @return The created mask.
 */
uint64_t create_mask(const uint8_t bits)
{
        uint64_t mask = 0;
        uint8_t bits_left = bits;

        for (; bits_left; bits_left--) {
                mask <<= 1ull;
                mask |= 1ull;
        }

        return mask;
}

/** @} sw-timer-system-internals */

void swtimersys_init(swtimersys_t *const swtimersys_p,
                            const uint64_t tick_duration_ns,
                            const uint8_t timer_width_bits,
                            const swtimersys_poll_callback_t poll_cb)
{
        assert(swtimersys_p);
        assert(tick_duration_ns);
        assert((timer_width_bits >= TIMER_BITS_MIN) && (timer_width_bits <= TIMER_BITS_MAX));

        swtimersys_p->timer_mask = create_mask(timer_width_bits);
        swtimersys_p->tick_counter = 0;
        swtimersys_p->tick_duration_ns = tick_duration_ns;
        swtimersys_p->poll_cb = poll_cb;
}

void swtimersys_tick(swtimersys_t *const swtimersys_p,
                            const uint64_t tick_count)
{
        assert(swtimersys_p);
        assert(tick_count);

        // Advance the tick counter.
        swtimersys_p->tick_counter += tick_count;

        // Emulate the width of the timer by limiting the tick counter by the
        // timer mask.
        swtimersys_p->tick_counter &= swtimersys_p->timer_mask;
}

uint64_t swtimersys_poll_timer(swtimersys_t *const swtimersys_p)
{
        assert(swtimersys_p);

        // If the poll callback has been set, use the direct hardware polling
        // mode. Otherwise, return the local tick counter.
        if (swtimersys_p->poll_cb) {
                return swtimersys_p->poll_cb(swtimersys_p->user_data_p);
        } else {
                return swtimersys_p->tick_counter;
        }
}

uint64_t swtimersys_get_tick_duration_ns(const swtimersys_t *const swtimersys_p)
{
        assert(swtimersys_p);

        return swtimersys_p->tick_duration_ns;
}

uint64_t
swtimersys_get_timer_mask(const swtimersys_t *const swtimersys_p)
{
        assert(swtimersys_p);

        return swtimersys_p->timer_mask;
}

/* EOF */