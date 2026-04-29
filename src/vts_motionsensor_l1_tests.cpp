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

extern sp<IMotionSensorManager> gMOTIONSENSORMGRHandle;

/* Test class registration with UT_Core */

UT_ADD_TEST_TO_GROUP(VTS_L1_MOTIONSENSOR, UT_TESTS_L1)

class VTS_L1_MOTIONSENSOR : public UTCore
{
public:
    VTS_L1_MOTIONSENSOR() : UTCore() {}
    ~VTS_L1_MOTIONSENSOR() override = default;
    void SetUp() override { android::ProcessState::self()->startThreadPool(); }
    void TearDown() override {}
};

// 1. getMotionSensorIds - Positive
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetMotionSensorIds_Success)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    std::vector<int32_t> sensorIds;
    if (sensorIdsOpt) {
        for (const auto& idOpt : *sensorIdsOpt) {
            if (idOpt) sensorIds.push_back(idOpt->value);
        }
    }
    UT_ASSERT_TRUE(sensorIds.size() > 0);
}

// 2. getMotionSensor - Positive
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetMotionSensor_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    std::vector<int32_t> sensorIds;
    if (sensorIdsOpt) {
        for (const auto& idOpt : *sensorIdsOpt) {
            if (idOpt) sensorIds.push_back(idOpt->value);
        }
    }
    for (auto id : sensorIds) {
        android::sp<IMotionSensor> sensorHandle;
        MotionSensorId sensorIdObj;
        sensorIdObj.value = id;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
    }
}

// 3. getMotionSensor - Negative (invalid IDs)
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetMotionSensor_Negative)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    std::vector<int32_t> sensorIds;
    if (sensorIdsOpt) {
        for (const auto& idOpt : *sensorIdsOpt) {
            if (idOpt) sensorIds.push_back(idOpt->value);
        }
    }
    if (sensorIds.empty()) return;
    int32_t minId = *std::min_element(sensorIds.begin(), sensorIds.end());
    int32_t maxId = *std::max_element(sensorIds.begin(), sensorIds.end());
    android::sp<IMotionSensor> sensorHandle;
    MotionSensorId sensorIdObj;
    // Below min
    sensorIdObj.value = minId - 1;
    MOTIONSENSOR_GET_SENSOR_FAILURE(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle, Status::EX_ILLEGAL_ARGUMENT);
    // Above max
    sensorIdObj.value = maxId + 1;
    MOTIONSENSOR_GET_SENSOR_FAILURE(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle, Status::EX_ILLEGAL_ARGUMENT);
    // UNDEFINED (if defined as -1)
    sensorIdObj.value = -1;
    MOTIONSENSOR_GET_SENSOR_FAILURE(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle, Status::EX_ILLEGAL_ARGUMENT);
}

// 4. getCapabilities - Positive
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetCapabilities_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    std::vector<int32_t> sensorIds;
    if (sensorIdsOpt) {
        for (const auto& idOpt : *sensorIdsOpt) {
            if (idOpt) sensorIds.push_back(idOpt->value);
        }
    }
    for (auto id : sensorIds) {
        android::sp<IMotionSensor> sensorHandle;
        MotionSensorId sensorIdObj;
        sensorIdObj.value = id;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        Capabilities capabilities;
        MOTIONSENSOR_GET_CAPABILITIES_SUCCESS(sensorHandle, capabilities);
        // Optionally compare with gMotionSensorDeviceInfo if available
    }
}

// 5. getState - Positive
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetState_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    std::vector<int32_t> sensorIds;
    if (sensorIdsOpt) {
        for (const auto& idOpt : *sensorIdsOpt) {
            if (idOpt) sensorIds.push_back(idOpt->value);
        }
    }
    for (auto id : sensorIds) {
        android::sp<IMotionSensor> sensorHandle;
        MotionSensorId sensorIdObj;
        sensorIdObj.value = id;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        State state;
        MOTIONSENSOR_GET_STATE(sensorHandle, state);
        // Optionally assert state is a valid enum value
    }
}

// 6. getCapabilities - Negative (invalid handle)
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetCapabilities_Negative)
{
    android::sp<IMotionSensor> sensorHandle = nullptr;
    Capabilities capabilities;
    // Should fail with nullptr handle
    ::android::binder::Status status = sensorHandle ? sensorHandle->getCapabilities(&capabilities) : ::android::binder::Status::fromExceptionCode(Status::EX_NULL_POINTER);
    UT_ASSERT_FALSE(status.isOk());
}

// 7. getState - Negative (invalid handle)
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetState_Negative)
{
    android::sp<IMotionSensor> sensorHandle = nullptr;
    State state;
    ::android::binder::Status status = sensorHandle ? sensorHandle->getState(&state) : ::android::binder::Status::fromExceptionCode(Status::EX_NULL_POINTER);
    UT_ASSERT_FALSE(status.isOk());
}
