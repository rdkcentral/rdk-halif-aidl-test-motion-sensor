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
 * @addtogroup HPK Hardware Porting Kit
 * @{
 */

/**
 * @addtogroup MOTION_SENSOR Motion Sensor Module
 * @{
 */

/**
 * @addtogroup MOTION_SENSOR_HALTEST Motion Sensor HAL Tests
 * @{
 */

/**
 * @defgroup MOTION_SENSOR_HALTEST_HELPER Motion Sensor HAL Tests Helper Functions Header
 * @{
 * @parblock
 *
 * ### Helper function declarations for motion sensor tests :
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 */

/**
 * @file vts_motionsensor_helper_functions.h
 */

#ifndef __VTS_MOTIONSENSOR_HELPER_FUNCTIONS_H__
#define __VTS_MOTIONSENSOR_HELPER_FUNCTIONS_H__

#include <string>
#include <vector>
#include <ut_control_plane.h>

/* Exception code map table for motion sensor status */
extern ut_control_keyStringMapping_t statusMotionSensorException_mapTable[];

/* Boolean string map table */
extern ut_control_keyStringMapping_t bool_mapTable_motionsensor[];

/* OperationalMode string map table */
extern const ut_control_keyStringMapping_t operationalMode_mapTable[];

/**
 * @brief Wait for a motion event with timeout
 * @param eventListener   Pointer to the event listener
 * @param expectedMode    Expected OperationalMode (MOTION or NO_MOTION)
 * @param timeoutMs       Timeout in milliseconds
 * @return true if event received, false on timeout
 */
bool waitForMotionEvent(void* eventListener, int expectedMode, int timeoutMs);

/**
 * @brief Poll state with retries until desired state is reached or timeout
 * @param sensorHandle  IMotionSensor handle
 * @param desiredState  Desired state enum value
 * @param timeoutMs     Timeout in milliseconds
 * @return true if desired state reached
 */
bool pollForState(void* sensorHandle, int desiredState, int timeoutMs);

#endif // __VTS_MOTIONSENSOR_HELPER_FUNCTIONS_H__

/** @} */ // End of MOTION_SENSOR_HALTEST_HELPER
/** @} */ // End of MOTION_SENSOR_HALTEST
/** @} */ // End of MOTION_SENSOR Module
/** @} */ // End of HPK
