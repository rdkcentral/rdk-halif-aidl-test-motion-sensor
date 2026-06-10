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
 * @defgroup MOTION_SENSOR_HALTEST_INIT Motion Sensor HAL Tests Init Header
 * @{
 * @parblock
 *
 * ### Init/destroy declarations for motion sensor tests :
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 */

/**
 * @file vts_motionsensor_init.h
 */

#ifndef __VTS_MOTIONSENSOR_INIT_H__
#define __VTS_MOTIONSENSOR_INIT_H__

#include <ut.h>

/**
 * @brief Initialize the Motion Sensor VTS test suite.
 *
 * Binds to the IMotionSensorManager binder service and parses the HFP
 * profile. The binder thread pool is started per-suite in each fixture's
 * SetUp().
 *
 * @return UT_STATUS_OK on successful initialization.
 * @return UT_STATUS_FAILURE if the IMotionSensorManager service cannot be
 *         acquired.
 */
UT_status_t VTS_MOTIONSENSOR_init(void);

/**
 * @brief Destroy/cleanup the Motion Sensor VTS test suite.
 *
 * Releases the IMotionSensorManager handle.
 */
void VTS_MOTIONSENSOR_destroy(void);

#endif // __VTS_MOTIONSENSOR_INIT_H__

/** @} */ // End of MOTION_SENSOR_HALTEST_INIT
/** @} */ // End of MOTION_SENSOR_HALTEST
/** @} */ // End of MOTION_SENSOR Module
/** @} */ // End of HPK
