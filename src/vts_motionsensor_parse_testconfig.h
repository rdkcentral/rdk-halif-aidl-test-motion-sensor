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
#include <string>
#include <ut.h>

#define VTS_MOTIONSENSOR_TEST_CONFIG_PATH "./test_configuration/vts_l3_MotionSensor.yaml"

/**
 * @brief L3 config for motion/no-motion event detection tests
 */
typedef struct motionSensorL3EventTestConfig
{
    int32_t  sensorIndex;           /**< Index into sensor list (0-based) */
    int32_t  operationalMode;       /**< 0=MOTION, 1=NO_MOTION */
    int32_t  noMotionSeconds;       /**< Seconds of no-motion threshold */
    int32_t  activeStartSeconds;    /**< Delay before monitoring activates */
    int32_t  activeStopSeconds;     /**< Duration of monitoring (0=indefinite) */
    int32_t  eventTimeoutMs;        /**< Timeout waiting for each event from host */
    int32_t  numEvents;             /**< Number of events to expect (N), set by Python */
} motionSensorL3EventTestConfig_t;

/**
 * @brief L3 config for active window behavior test
 */
typedef struct motionSensorL3ActiveWindowTestConfig
{
    int32_t  sensorIndex;
    int32_t  windowStartSeconds;    /**< Active window start (seconds of day) */
    int32_t  windowEndSeconds;      /**< Active window end (seconds of day) */
    int32_t  eventTimeoutMs;
} motionSensorL3ActiveWindowTestConfig_t;

/**
 * @brief L3 config for deep sleep autonomy test
 */
typedef struct motionSensorL3DeepSleepTestConfig
{
    int32_t  sensorIndex;
    int32_t  deepSleepDurationSeconds;
    int32_t  eventTimeoutMs;
} motionSensorL3DeepSleepTestConfig_t;

/**
 * @brief Parse event detection test config (MotionEventDetection / NoMotionEventDetection)
 */
UT_status_t vts_motionsensor_parse_eventtest_config(
    const char *configFile,
    const char *module,
    const char *testGroup,
    const char *testName,
    motionSensorL3EventTestConfig_t& config);

/**
 * @brief Parse active window test config
 */
UT_status_t vts_motionsensor_parse_activewindow_config(
    const char *configFile,
    const char *module,
    const char *testGroup,
    const char *testName,
    motionSensorL3ActiveWindowTestConfig_t& config);

/**
 * @brief Parse deep sleep autonomy test config
 */
UT_status_t vts_motionsensor_parse_deepsleep_config(
    const char *configFile,
    const char *module,
    const char *testGroup,
    const char *testName,
    motionSensorL3DeepSleepTestConfig_t& config);

#endif // __VTS_MOTIONSENSOR_PARSE_TESTCONFIG_H__
