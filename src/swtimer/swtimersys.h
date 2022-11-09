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

#pragma once

#include <stdint.h>

/**
 * @file       swtimersys.h
 * @defgroup   sw-timer-system Software timer system
 * @copyright  Copyright &copy; 2020-2022, Tuomas Terho. All rights reserved.
 *
 * The software timer system enables one hardware timer being used for multiple
 * software timers. It provides an interface and configuration for software
 * timers using the system.
 *
 * The software timer system can be used by two ways depending on the behavior
 * of the hardware timer: by using the @ref swtimersys_tick function e.g. from
 * an interrupt handler, or by providing a callback for polling a free-running
 * hardware timer.
 *
 * This implementation supports timers up to 64 bits. The actual timer width can
 * be adjusted by providing a bit count for the initialization function.
 *
 * @{
 */

/**
 * @brief A callback function type for hardware timer polling.
 *
 * @param[in] user_data_p A pointer to user specified data.
 *
 * @return Current hardware timer value.
 */
typedef uint64_t (*swtimersys_poll_callback_t)(const void *const user_data_p);

/**
 * @brief Software timer system instance data.
 */
typedef struct swtimersys_s {
        /// Hardware timer polling callback.
        swtimersys_poll_callback_t poll_cb;
        /// Timer tick counter.
        uint64_t tick_counter;
        /// One timer tick duration (in nanoseconds).
        uint64_t tick_duration_ns;
        /// Timer width in bits.
        uint8_t timer_width_bits;
        /// Timer mask.
        uint64_t timer_mask;
        /// A pointer to user specified data.
        const void *user_data_p;
} swtimersys_t;

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

/**
 * @brief Initialize a software timer system.
 *
 * @param[in,out] swtimersys_p A pointer to a timer system instance.
 * @param[in] tick_duration_ns Duration of one timer tick in nanoseconds.
 * @param[in] timer_width_bits Timer width in bits from 2 to 64.
 * @param[in] poll_cb A callback for polling a hardware timer (optional).
 * @param[in] user_data_p A pointer to user specified data.
 */
void swtimersys_init(swtimersys_t *const swtimersys_p,
                            const uint64_t tick_duration_ns,
                            const uint8_t timer_width_bits,
                            const swtimersys_poll_callback_t poll_cb,
                            const void *const user_data_p);

/**
 * @brief Advance the timer system by the given timer tick count.
 *
 * @param[in,out] swtimersys_p A pointer to a timer system instance.
 * @param[in] tick_count Count of ticks to advance the timer.
 */
void swtimersys_tick(swtimersys_t *const swtimersys_p,
                            const uint64_t tick_count);

/**
 * @brief Poll the timer to get the current timer value.
 *
 * @param[in,out] swtimersys_p A pointer to a timer system instance.
 *
 * @return Current timer value.
 *
 * This function returns either an internal counter value advanced by the
 * @ref swtimersys_tick function, or an external value returned by the poll
 * callback function, depending on whether the callback is set or not.
 */
uint64_t swtimersys_poll_timer(swtimersys_t *const swtimersys_p);

/**
 * @brief Get tick duration.
 *
 * @param[in,out] swtimersys_p A pointer to a timer system instance.
 *
 * @return Tick duration in nanoseconds.
 */
uint64_t swtimersys_get_tick_duration_ns(const swtimersys_t *const swtimersys_p);

/**
 * @brief Get timer mask.
 *
 * @param[in] swtimersys_p A pointer to a timer system instance.
 *
 * @return Timer mask.
 */
uint64_t
swtimersys_get_timer_mask(const swtimersys_t *const swtimersys_p);

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

/** @} sw-timer-system */

/* EOF */