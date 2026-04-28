/**
 *  If not stated otherwise in this file or this component's LICENSE
 *  file the following copyright and licenses apply:
 *
 *  Copyright 2026 RDK Management
 *
 *  Licensed under the Apache License, Version 2.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * @file vts_motionsensor_parse_testconfig.h
 */

#ifndef __VTS_MOTIONSENSOR_PARSE_TESTCONFIG_H__
#define __VTS_MOTIONSENSOR_PARSE_TESTCONFIG_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief L3 test configuration for motion event tests
 */
typedef struct
{
    int32_t  operationalMode;           /**< 0=MOTION, 1=NO_MOTION */
    int32_t  noMotionSeconds;           /**< Seconds of no-motion before NO_MOTION event */
    int32_t  eventWaitTimeoutMs;        /**< Timeout to wait for event in ms */
    int32_t  activeWindowStartSeconds;  /**< Active window start (seconds of day) */
    int32_t  activeWindowEndSeconds;    /**< Active window end (seconds of day) */
    bool     useActiveWindow;           /**< Whether to configure an active window */
} MotionSensorL3TestConfig_t;

/**
 * @brief Global L3 test configuration (populated by vts_motionsensor_parse_testconfig)
 */
extern MotionSensorL3TestConfig_t gMotionSensorL3Config;

/**
 * @brief Parse the L3 YAML test configuration file
 */
void vts_motionsensor_parse_testconfig(void);

#endif // __VTS_MOTIONSENSOR_PARSE_TESTCONFIG_H__
