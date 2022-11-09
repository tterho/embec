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

#include "swtimer.h"

#include <stdbool.h>
#include <assert.h>
#include <string.h>

/**
 * @defgroup swtimer-internals Internals
 * @ingroup  swtimer
 *
 * @{
 */

/// Nanoseconds in one microsecond.
#define NS_IN_ONE_US 1000u
/// Nanoseconds in one millisecond.
#define NS_IN_ONE_MS 1000000u
/// Nanoseconds in one second.
#define NS_IN_ONE_SECOND 1000000000ul

#ifndef DISABLE_SWTIMER_STARVATION_AWARENESS

/**
 * @brief Initialize the timer starvation tracking.
 *
 * @param[in,out] starvation_tracking_p A pointer to starvation tracking data.
 * @param[in] initial_tick_count Initial tick count.
 */
static void init_starvation_tracking(
        swtimer_starvation_tracking_t *const starvation_tracking_p,
        const uint64_t initial_tick_count)
{
        starvation_tracking_p->last_tick_count = initial_tick_count;
}

/**
 * @brief Check if starvation tracking is enabled.
 *
 * @param[in] starvation_tracking_p A pointer to starvation tracking data.
 *
 * @retval true Starvation tracking is enabled.
 * @retval false Starvation tracking is disabled.
 */
static bool is_starvation_tracking_enabled(
        const swtimer_starvation_tracking_t *const starvation_tracking_p)
{
        // Starvation tracking is enabled, if the invocation limit has been set
        // to a non-zero value.
        return (0u != starvation_tracking_p->invocation_limit);
}

/**
 * @brief Manage invocation count for starvation tracking.
 *
 * @param[in,out] starvation_tracking_p A pointer to starvation tracking data.
 * @param[in] tick_count Current tick count.
 */
static void manage_invocation_count(
        swtimer_starvation_tracking_t *const starvation_tracking_p,
        const uint64_t tick_count)
{
        // If the timer tick count increases between the invocations, the
        // timer system is alive and there is no starvation. Otherwise, the
        // starvation tracking system counts the time how long the timer system
        // is not responding.
        if ((starvation_tracking_p->last_tick_count == tick_count)) {
                // Stop invocation counter to the maximum value.
                if (starvation_tracking_p->invocation_count < UINT64_MAX) {
                        ++(starvation_tracking_p->invocation_count);
                }
        } else {
                starvation_tracking_p->invocation_count = 0u;
        }

        // Keep the last tick count on track.
        starvation_tracking_p->last_tick_count = tick_count;
}

/**
 * @brief Check if timer is starving.
 *
 * @param[in,out] starvation_tracking_p A pointer to starvation tracking data.
 * @param[in] tick_count Tick count from the timer system.
 *
 * @retval true Timer is starving.
 * @retval false Timer is not starving.
 */
static bool is_timer_starving(
        swtimer_starvation_tracking_t *const starvation_tracking_p,
        uint64_t const tick_count)
{
        // If the starvation tracking is disabled, the timer can't be starving.
        if (!is_starvation_tracking_enabled(starvation_tracking_p)) {
                return false;
        }

        manage_invocation_count(starvation_tracking_p, tick_count);

        // Check if the invocation count exceeds or equals to the invocation
        // limit, and return the result as a boolean value.
        return (starvation_tracking_p->invocation_count >=
                starvation_tracking_p->invocation_limit);
}

#endif // DISABLE_SWTIMER_STARVATION_AWARENESS

/**
 * @brief Get elapsed tick count from the previous start and manage possible
 *      timer wrap-arounds.
 *
 * @param[in] timer_mask Timer mask for the timer width.
 * @param[in] start_tick_count Tick count at timer start.
 * @param[in] current_tick_count Current tick count.
 *
 * @return Time in ticks.
 */
static uint64_t get_elapsed_ticks(const uint64_t timer_mask,
                                  const uint64_t start_tick_count,
                                  const uint64_t tick_count)
{
        return (start_tick_count <= tick_count) ?
                       (tick_count - start_tick_count) :
                       ((timer_mask - start_tick_count) + tick_count +
                        1);
}

/**
 * @brief Get time for the tick count.
 *
 * @param[in] tick_duration_ns Duration of one tick in nanoseconds.
 * @param[in] tick_count Tick count.
 * @param[in] resolution The resolution of time.
 *
 * @return Time in the given resolution.
 */
static uint64_t get_time_for_tick_count(
        const uint64_t tick_duration_ns, const uint64_t tick_count,
        const swtimer_resolution_t resolution)
{
        uint64_t time;

        // Calculate the time lapse based on the timer system and the requested
        // resolution of time.
        switch (resolution) {
        case SWTIMER_NS:
                // Calculate time in nanoseconds.
                time = (tick_count * tick_duration_ns);
                break;

        case SWTIMER_US:
                // Calculate time in microseconds.
                time = (tick_count * tick_duration_ns) / NS_IN_ONE_US;
                break;

        case SWTIMER_MS:
                // Calculate time in milliseconds.
                time = (tick_count * tick_duration_ns) / NS_IN_ONE_MS;
                break;

        case SWTIMER_S:
                // Calculate time in seconds.
                time = (tick_count * tick_duration_ns) / NS_IN_ONE_SECOND;
                break;

        case SWTIMER_TIMERTICK:
        default:
                // The requested resolution is in the same resolution than one
                // timer tick, or the resolution is undefined.
                time = tick_count;
                break;
        }

        return time;
}

/** @} swtimer-internals */

void swtimer_init(swtimer_t *const swtimer_p,
                  const swtimersys_t *const swtimersys_p,
                  const uint64_t invocation_limit)
{
        assert(swtimer_p);
        assert(swtimersys_p);

        // Reset all timer data (this resets also the starvation tracking if
        // that feature is enabled).
        memset(swtimer_p, 0, sizeof(swtimer_t));
        // Link the timer with the timer system.
        swtimer_p->swtimersys_p = swtimersys_p;
        // Get the tick duration and the timer mask from the timer system.
        swtimer_p->tick_duration_ns = swtimersys_get_tick_duration_ns(swtimersys_p);
        swtimer_p->timer_mask = swtimersys_get_timer_mask(swtimersys_p);

#ifndef DISABLE_SWTIMER_STARVATION_AWERENESS
        swtimer_p->starvation_tracking.invocation_limit = invocation_limit;
#endif // DISABLE_SWTIMER_STARVATION_AWARENESS
}

void swtimer_start(swtimer_t *const swtimer_p)
{
        assert(swtimer_p);

        swtimer_p->start_tick_count =
                swtimersys_poll_timer(swtimer_p->swtimersys_p);

#ifndef DISABLE_SWTIMER_STARVATION_AWARENESS
        init_starvation_tracking(&(swtimer_p->starvation_tracking),
                                  swtimer_p->start_tick_count);
#endif // DISABLE_SWTIMER_STARVATION_AWARENESS
}

uint64_t swtimer_get_time(
        const swtimer_t *const swtimer_p,
        swtimer_resolution_t const resolution)
{
        uint64_t tick_count;

        assert(swtimer_p);

        // Poll the current tick count from the timer system.
        tick_count = swtimersys_poll_timer(swtimer_p->swtimersys_p);

        // Calculate elapsed ticks from the start.
        tick_count = get_elapsed_ticks(swtimer_p->timer_mask,
                                       swtimer_p->start_tick_count,
                                       tick_count);

#ifndef DISABLE_SWTIMER_STARVATION_AWARENESS
        assert(!is_timer_starving(&(swtimer_p->starvation_tracking),
                                  tick_count));
#endif // DISABLE_SWTIMER_STARVATION_AWARENESS

        // Calculate elapsed time from the tick count.
        return get_time_for_tick_count(swtimer_p->tick_duration_ns, tick_count,
                                        resolution);
}

/* EOF */
