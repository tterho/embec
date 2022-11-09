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

#include "swtimersys.h"

/**
 * @file       swtimer.h
 * @defgroup   swtimer Software timer
 * @copyright  Copyright &copy; 2020-2022, Tuomas Terho. All rights reserved.
 *
 * This software timer has equipped with starvation awereness functionality.
 * That feature is used to detect situations where the underlying hardware timer
 * stops running, but the software is still polling the timer. The starvation
 * detection recognizes the situation and raises an error.
 *
 * If the starvation awereness is not needed, the functionality can be disabled
 * by adding the define DISABLE_SWTIMER_STARVATION_AWERENESS to the project
 * options. Disabling the feature saves some memory for each timer instance, and
 * has a small impact to the overall timer perfomance.
 *
 * @{
 */

/**
 * @brief Resolution of time.
 */
typedef enum swtimer_resolution_e {
        /// Timer ticks (raw counter).
        SWTIMER_TIMERTICK = 0,
        /// Nanoseconds.
        SWTIMER_NS,
        /// Microseconds.
        SWTIMER_US,
        /// Milliseconds.
        SWTIMER_MS,
        /// Seconds.
        SWTIMER_S
} swtimer_resolution_t;

#ifndef DISABLE_SWTIMER_STARVATION_AWERENESS
/**
 * @brief Timer starvation tracking data.
 */
typedef struct swtimer_starvation_tracking_s {
        /// Limit for @ref swtimer_get_time invocations to detect timer
        /// starvation when the timer system is not running.
        uint64_t invocation_limit;
        /// Invocation count for @ref swtimer_get_time invocations.
        uint64_t invocation_count;
        /// Last tick count value for detecting starvation.
        uint64_t last_tick_count;
} swtimer_starvation_tracking_t;
#endif // #ifndef DISABLE_SWTIMER_STARVATION_AWERENESS

/**
 * @brief Timer instance data.
 */
typedef struct swtimer_s {
        /// A pointer to a timer system on which this timer runs.
        const swtimersys_t *swtimersys_p;
        /// The tick counter value at timer start.
        uint64_t start_tick_count;
        /// Time tick duration (in nanoseconds), inherited from the timer
        /// system.
        uint64_t tick_duration_ns;
        /// Timer counter mask, inherited from the timer system.
        uint64_t timer_mask;
#ifndef DISABLE_SWTIMER_STARVATION_AWERENESS
        /// Timer starvation tracking.
        swtimer_starvation_tracking_t starvation_tracking;
#endif // #ifndef DISABLE_SWTIMER_STARVATION_AWERENESS
} swtimer_t;

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

/**
 * @brief Initialize a software timer.
 *
 * @param[in,out] swtimer_p A pointer to a software timer instance.
 * @param[in] swtimersys_p A pointer to a software timer system which this timer
 *      uses.
 * @param[in] invocation_limit Invocation limit for starvation awereness. Set to
 *      0 to disable the starvation tracking. This parameter has no effect if
 *      the starwation awereness has been disabled by the compiler option.
 */
void swtimer_init(swtimer_t *const swtimer_p,
                  const swtimersys_t *const swtimersys_p,
                  const uint64_t invocation_limit);

/**
 * @brief Start a timer.
 *
 * @param[in,out] swtimer_p A pointer to a software timer instance.
 */
void swtimer_start(swtimer_t *const swtimer_p);

/**
 * @brief Get the time elapsed from the timer start.
 *
 * @param[in] swtimer_p A pointer to a software timer instance.
 * @param[in] resolution The resolution of time to check.
 *
 * @return Time in the given resolution from the previous timer start.
 */
uint64_t swtimer_get_time(
        const swtimer_t *const swtimer_p,
        swtimer_resolution_t const resolution);

#ifdef __cplusplus
}
#endif // ifdef __cplusplus

/** @} swtimer */

/* EOF */
