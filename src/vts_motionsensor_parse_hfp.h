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
#include <string>
#include <vector>

#include "com/rdk/hal/sensor/motion/OperationalMode.h"
#include <ut_control_plane.h>
#include "vts_motionsensor_helper_functions.h"

using namespace com::rdk::hal::sensor::motion;

/**
 * @brief Single motion sensor entry parsed from HFP profile.
 *
 * Mirrors the structure of motionsensorConfig_t from the vcomponent helper.
 */
typedef struct motionsensorHFP {
    /* ==============================
     * Sensor id (Immutable)
     * ============================== */
    int id;

    /* ==============================
     * Capabilities (Immutable)
     * ============================== */
    struct Capabilities {
        std::string sensorName;              ///< Capabilities.sensorName
        int minSensitivity;                  ///< Capabilities.minSensitivity
        int maxSensitivity;                  ///< Capabilities.maxSensitivity
        bool supportsDeepSleepAutonomy;      ///< Capabilities.supportsDeepSleepAutonomy
    } capabilities;

    /* ==============================
     * Supported operational modes
     * ============================== */
    std::vector<OperationalMode> operationalModes;

    /* ==============================
     * Factory default StartConfig
     * ============================== */
    struct StartConfig {
        OperationalMode operationalMode;
        int noMotionSeconds;
        int activeStartSeconds;
        int activeStopSeconds;
    } defaultStartConfig;

    /* ==============================
     * Active windows (factory defaults)
     * ============================== */
    struct ActiveWindow {
        int startTimeOfDaySeconds;
        int endTimeOfDaySeconds;
    };
    std::vector<ActiveWindow> activeWindows;
    int minReactionTimeMs;
    std::vector<std::string> notes;
} motionsensorHFP_t;

/**
 * @brief Top-level HFP structure holding all parsed sensors.
 */
typedef struct motionsensorHFPFile {
    std::vector<motionsensorHFP_t> sensors;
} motionsensorHFPFile_t;

/**
 * @brief Parsed HFP global (populated by vts_motionsensor_parse_hfp).
 */
extern motionsensorHFPFile_t gMotionSensorHFP;

/**
 * @brief Parse the hardware feature profile YAML for all motion sensors.
 *
 * Populates gMotionSensorHFP.sensors with one entry per sensor found
 * under the "sensor.motion" list in the HFP profile.
 */
void vts_motionsensor_parse_hfp(void);

#endif // __VTS_MOTIONSENSOR_PARSE_HFP_H__
