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
 * @file vts_motionsensor_parse_hfp.h
 */

#ifndef __VTS_MOTIONSENSOR_PARSE_HFP_H__
#define __VTS_MOTIONSENSOR_PARSE_HFP_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Structure holding parsed device capabilities from HFP profile
 */
typedef struct
{
    int32_t  numSensors;                  /**< Number of motion sensors available */
    int32_t  minSensitivity;             /**< Minimum sensitivity level (0 = unsupported) */
    int32_t  maxSensitivity;             /**< Maximum sensitivity level (0 = unsupported) */
    bool     supportsMotionMode;         /**< Supports MOTION operational mode */
    bool     supportsNoMotionMode;       /**< Supports NO_MOTION operational mode */
    bool     supportsAutonomousDeepSleep;/**< Supports autonomous operation during deep sleep */
    bool     supportsActiveWindows;      /**< Supports time-based active windows */
    int32_t  maxActiveWindows;           /**< Maximum number of active windows */
    int32_t  minNoMotionSeconds;         /**< Minimum valid noMotionSeconds value */
    int32_t  maxNoMotionSeconds;         /**< Maximum valid noMotionSeconds value */
} MotionSensorDeviceInfo_t;

/**
 * @brief Parsed device info global (populated by vts_motionsensor_parse_hfp)
 */
extern MotionSensorDeviceInfo_t gMotionSensorDeviceInfo;

/**
 * @brief Parse the hardware feature profile YAML for motion sensor capabilities
 */
void vts_motionsensor_parse_hfp(void);

#endif // __VTS_MOTIONSENSOR_PARSE_HFP_H__
