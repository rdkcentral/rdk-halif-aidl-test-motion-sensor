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
 * @defgroup MOTION_SENSOR_HALTEST_L1 Motion Sensor HAL Tests L1 File
 * @{
 * @parblock
 *
 * ### L1 Test Cases for Motion Sensor HAL :
 *
 * This module includes Level 1 functional tests (success and failure scenarios).
 * Tests verify that each API meets the operational requirements across all vendors.
 *
 * **Pre-Conditions:**  Motion Sensor HAL service must be running @n
 * **Dependencies:** IMotionSensorManager, IMotionSensor, IMotionSensorController @n
 *
 * @endparblock
 */

/**
 * @file vts_motionsensor_l1_tests.cpp
 */

#include "vts_motionsensor_parse_hfp.h"
#include "vts_motionsensor.h"

extern sp<IMotionSensorManager> gMotionSensorManagerHandle;

/* Test class registration with UT_Core */
UT_ADD_TEST_TO_GROUP(VTS_L1_MOTIONSENSOR, UT_TESTS_L1)

class VTS_L1_MOTIONSENSOR : public UTCore
{
public:
};

/* ========================================================================== */
/*  IMotionSensorManager L1 Tests                                             */
/* ========================================================================== */

/**
 * @brief L1 Test: getMotionSensorIds - success
 *
 * Verify that getMotionSensorIds() returns a non-empty list of sensor IDs
 * and the call succeeds with EX_NONE.
 *
 * **Test Steps:**
 * 1. Call getMotionSensorIds()
 * 2. Verify status.isOk() == true
 * 3. Verify sensorIds is non-empty
 *
 * **Expected Result:** Returns at least one sensor ID with EX_NONE
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetMotionSensorIds_Success)
{
    vector<int32_t> sensorIds;

    UT_LOG_INFO("In %s", __FUNCTION__);

    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMotionSensorManagerHandle, sensorIds);
    UT_ASSERT_TRUE(sensorIds.size() > 0);

    UT_LOG_INFO("GetMotionSensorIds returned [%zu] sensor(s)", sensorIds.size());
    UT_LOG_INFO("Out %s", __FUNCTION__);
}
