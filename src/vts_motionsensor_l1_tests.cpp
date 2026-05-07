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

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

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

/**
 * @brief Assert that a given sensor ID exists in the HFP profile.
 *
 * @param id The sensor ID to look up in gMotionSensorHFP.sensors.
 */
static void assertSensorIdInHFP(int32_t id)
{
    bool foundInHFP = false;
    for (const auto& hfpSensor : gMotionSensorHFP.sensors)
    {
        if (hfpSensor.id == id)
        {
            foundInHFP = true;
            break;
        }
    }
    UT_ASSERT_TRUE(foundInHFP);
}

/**
 * @brief Validate that getMotionSensorIds returns a non-empty list of sensor IDs.
 *
 * This test calls getMotionSensorIds on the IMotionSensorManager and asserts that
 * at least one sensor ID is present in the returned list.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 001@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds on the manager | Output: sensorIdsOpt | List of sensor IDs is returned | Should be successful |
 * | 02 | Verify at least one sensor ID is present | sensorIds vector | sensorIds.size() > 0 | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetMotionSensorIds_Success)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    std::vector<int32_t> sensorIds;
    if (sensorIdsOpt)
    {
        for (const auto& idOpt : *sensorIdsOpt)
        {
            if (idOpt)
            {
                assertSensorIdInHFP(idOpt->value);
                sensorIds.push_back(idOpt->value);
            }
        }
    }
    UT_ASSERT_TRUE(sensorIds.size() > 0);
    // Compare sensor count against HFP profile
    UT_ASSERT_EQUAL((int32_t)sensorIds.size(), (int32_t)gMotionSensorHFP.sensors.size());

}

/**
 * @brief Validate that getMotionSensor returns a valid sensor handle for each known sensor ID.
 *
 * This test iterates over all sensor IDs returned by getMotionSensorIds and calls
 * getMotionSensor for each, asserting that a non-null sensor handle is returned.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 002@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds on the manager | Output: sensorIdsOpt | List of sensor IDs is returned | Should be successful |
 * | 02 | Verify sensor count matches HFP profile | sensorIds.size() vs gMotionSensorHFP.sensors.size() | Counts are equal | Should Pass |
 * | 03 | For each sensor ID, verify it exists in the HFP profile | Input: sensorId vs gMotionSensorHFP.sensors | Each ID found in HFP profile | Should Pass |
 * | 04 | For each sensor ID, call getMotionSensor | Input: sensorId | Valid non-null sensor handle returned | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetMotionSensor_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    std::vector<int32_t> sensorIds;
    if (sensorIdsOpt)
    {
        for (const auto& idOpt : *sensorIdsOpt)
        {
            if (idOpt)
            {
                assertSensorIdInHFP(idOpt->value);
                sensorIds.push_back(idOpt->value);
            }
        }
    }
    // Compare sensor count against HFP profile
    UT_ASSERT_EQUAL((int32_t)sensorIds.size(), (int32_t)gMotionSensorHFP.sensors.size());

    for (auto id : sensorIds)
    {
        android::sp<IMotionSensor> sensorHandle;
        MotionSensorId sensorIdObj;
        sensorIdObj.value = id;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
    }
}

/**
 * @brief Validate that getMotionSensor returns EX_ILLEGAL_ARGUMENT for out-of-range and undefined sensor IDs.
 *
 * This test calls getMotionSensor with IDs below the minimum, above the maximum, and with
 * the undefined value (-1), asserting EX_ILLEGAL_ARGUMENT is returned in each case.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 003@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds to determine valid ID range | Output: sensorIdsOpt | Valid ID range obtained | Should be successful |
 * | 02 | Call getMotionSensor with minId-1 | Input: sensorId = minId-1 | Returns EX_ILLEGAL_ARGUMENT | Should Pass |
 * | 03 | Call getMotionSensor with maxId+1 | Input: sensorId = maxId+1 | Returns EX_ILLEGAL_ARGUMENT | Should Pass |
 * | 04 | Call getMotionSensor with undefined ID (-1) | Input: sensorId = -1 | Returns EX_ILLEGAL_ARGUMENT | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetMotionSensor_Negative)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    std::vector<int32_t> sensorIds;
    if (sensorIdsOpt)
    {
        for (const auto& idOpt : *sensorIdsOpt)
        {
            if (idOpt)
            {
                assertSensorIdInHFP(idOpt->value);
                sensorIds.push_back(idOpt->value);
            }
        }
    }
    if (sensorIds.empty())
    {
        UT_LOG_WARNING("No sensor IDs available from getMotionSensorIds, skipping out-of-range ID tests");
        return; // No sensors available, cannot perform out-of-range test, but also nothing to fail on valid ID test
    }

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

/**
 * @brief Validate that getCapabilities returns a valid Capabilities struct for each sensor.
 *
 * This test iterates over all available sensor IDs, retrieves each sensor handle,
 * and calls getCapabilities, asserting a successful result.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 004@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds on the manager | Output: sensorIdsOpt | List of sensor IDs returned | Should be successful |
 * | 02 | For each sensor ID, call getMotionSensor | Input: sensorId | Valid sensor handle returned | Should Pass |
 * | 03 | Call getCapabilities on each sensor handle | Output: capabilities | Capabilities struct returned successfully | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetCapabilities_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    std::vector<int32_t> sensorIds;
    if (sensorIdsOpt)
    {
        for (const auto& idOpt : *sensorIdsOpt)
        {
            if (idOpt)
            {
                assertSensorIdInHFP(idOpt->value);
                sensorIds.push_back(idOpt->value);
            }
        }
    }

    for (auto id : sensorIds)
    {
        android::sp<IMotionSensor> sensorHandle;
        MotionSensorId sensorIdObj;
        sensorIdObj.value = id;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        Capabilities capabilities;
        MOTIONSENSOR_GET_CAPABILITIES_SUCCESS(sensorHandle, capabilities);
        // Find matching HFP entry by sensor ID and compare capabilities
        const motionsensorHFP_t* hfpEntry = nullptr;
        for (const auto& sensor : gMotionSensorHFP.sensors) {
            if (sensor.id == id) {
                hfpEntry = &sensor;
                break;
            }
        }
        UT_ASSERT_NOT_EQUAL(hfpEntry, nullptr);
        if (hfpEntry)
        {
            bool match = std::string(android::String8(capabilities.sensorName).c_str()) == hfpEntry->capabilities.sensorName;
            UT_ASSERT_TRUE(match);
            UT_ASSERT_EQUAL(capabilities.minSensitivity, hfpEntry->capabilities.minSensitivity);
            UT_ASSERT_EQUAL(capabilities.maxSensitivity, hfpEntry->capabilities.maxSensitivity);
            UT_ASSERT_EQUAL(capabilities.supportsDeepSleepAutonomy, hfpEntry->capabilities.supportsDeepSleepAutonomy);
        }
    }
}

/**
 * @brief Validate that getState returns successfully for each sensor.
 *
 * This test iterates over all available sensor IDs, retrieves each sensor handle,
 * and calls getState, asserting the call succeeds.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 005@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds on the manager | Output: sensorIdsOpt | List of sensor IDs returned | Should be successful |
 * | 02 | For each sensor ID, call getMotionSensor | Input: sensorId | Valid sensor handle returned | Should Pass |
 * | 03 | Call getState on each sensor handle | Output: state | State returned successfully | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetState_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    std::vector<int32_t> sensorIds;
    if (sensorIdsOpt)
    {
        for (const auto& idOpt : *sensorIdsOpt)
        {
            if (idOpt)
            {
                assertSensorIdInHFP(idOpt->value);
                sensorIds.push_back(idOpt->value);
            }
        }
    }

    for (auto id : sensorIds)
    {
        android::sp<IMotionSensor> sensorHandle;
        assertSensorIdInHFP(id);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = id;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        State state;
        MOTIONSENSOR_GET_STATE_SUCCESS(sensorHandle, state);
        // Optionally assert state is a valid enum value
    }
}


/**
 * @brief Validate that open and close succeed for each sensor with a valid listener.
 *
 * This test iterates over all available sensors, calls open with a valid controller
 * listener, asserts a non-null controller is returned, and then calls close.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 006@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Call getMotionSensor for each sensor ID | Input: sensorId | Valid sensor handle returned | Should Pass |
 * | 03 | Call open with a valid controller listener | Input: listener | Non-null controller handle returned | Should Pass |
 * | 04 | Call close with the returned controller | Input: controllerHandle | Close returns true, status ok | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Open_Close_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Validate that open returns EX_NULL_POINTER when called with a null listener.
 *
 * This test iterates over all sensors and calls open with a null controller listener,
 * asserting that EX_NULL_POINTER is returned and no controller is created.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 007@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Call getMotionSensor for each sensor ID | Input: sensorId | Valid sensor handle returned | Should Pass |
 * | 03 | Call open with a null listener | Input: nullListener = nullptr | Returns EX_NULL_POINTER, controller is null | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Open_Negative_NullListener)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> nullListener = nullptr;
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_FAILURE(sensorHandle, nullListener, controllerHandle, Status::EX_NULL_POINTER);
    }
}

/**
 * @brief Validate that a second open call on an already-open sensor returns EX_ILLEGAL_STATE.
 *
 * This test opens each sensor successfully, then attempts a second open on the same sensor,
 * asserting that EX_ILLEGAL_STATE is returned. The sensor is then closed.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 008@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Call getMotionSensor for each sensor ID | Input: sensorId | Valid sensor handle returned | Should Pass |
 * | 03 | Call open with a valid listener | Input: listener | Controller returned successfully | Should Pass |
 * | 04 | Call open again on the same sensor | Input: listener2 | Returns EX_ILLEGAL_STATE | Should Pass |
 * | 05 | Close the sensor using the first controller | Input: controllerHandle | Close succeeds | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Open_Negative_AlreadyOpen)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        // Second open on the same sensor must fail with EX_ILLEGAL_STATE
        android::sp<IMotionSensorControllerListener> listener2 = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle2;
        MOTIONSENSOR_OPEN_FAILURE(sensorHandle, listener2, controllerHandle2, Status::EX_ILLEGAL_STATE);
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}


/**
 * @brief Validate that close returns EX_NULL_POINTER when called with a null controller.
 *
 * This test iterates over all sensors, obtains a sensor handle, then calls close
 * with a null controller, asserting EX_NULL_POINTER is returned.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 009@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Call getMotionSensor for each sensor ID | Input: sensorId | Valid sensor handle returned | Should Pass |
 * | 03 | Call close with nullController = nullptr | Input: nullptr | Returns EX_NULL_POINTER | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Close_Negative_NullController)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorController> nullController = nullptr;
        MOTIONSENSOR_CLOSE_FAILURE(sensorHandle, nullController, Status::EX_NULL_POINTER);
    }
}

/**
 * @brief Validate that close returns false when called with a controller belonging to a different sensor.
 *
 * This test requires at least 2 sensors. Both sensors are opened, then close is called on
 * sensor[0] passing sensor[1]'s controller. The result must be false with status ok (EX_NONE).
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 010@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** At least 2 motion sensors must be available@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds, verify at least 2 sensors | Output: sensorIdsOpt | At least 2 sensor IDs present | Should be successful |
 * | 02 | Get sensor handles for sensor[0] and sensor[1] | Input: id0, id1 | Both sensor handles returned | Should Pass |
 * | 03 | Open both sensors with separate valid listeners | Input: listener0, listener1 | Both controller handles returned | Should Pass |
 * | 04 | Call close on sensor[0] passing sensor[1]'s controller | Input: sensorHandle0, controller1 | Returns false, status EX_NONE | Should Pass |
 * | 05 | Close both sensors with their correct controllers | Input: controller0, controller1 | Both close calls succeed | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Close_Negative_WrongController)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->size() < 2)
    {
        UT_LOG_WARNING("Not enough sensors available");
        return;
    }
    MotionSensorId id0, id1;
    id0.value = (*sensorIdsOpt)[0]->value;
    id1.value = (*sensorIdsOpt)[1]->value;
    assertSensorIdInHFP(id0.value);
    assertSensorIdInHFP(id1.value);

    android::sp<IMotionSensor> sensorHandle0, sensorHandle1;
    MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, id0, sensorHandle0);
    MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, id1, sensorHandle1);

    android::sp<IMotionSensorControllerListener> listener0 = android::sp<MotionSensorControllerListener>::make();
    android::sp<IMotionSensorControllerListener> listener1 = android::sp<MotionSensorControllerListener>::make();
    android::sp<IMotionSensorController> controller0, controller1;
    MOTIONSENSOR_OPEN_SUCCESS(sensorHandle0, listener0, controller0);
    MOTIONSENSOR_OPEN_SUCCESS(sensorHandle1, listener1, controller1);

    // Closing sensor0 with sensor1's controller must return false, status ok
    MOTIONSENSOR_CLOSE_FAILURE(sensorHandle0, controller1, Status::EX_NONE);

    // Cleanup
    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle0, controller0);
    MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle1, controller1);
}


/**
 * @brief Validate that registerEventListener and unregisterEventListener succeed for each sensor.
 *
 * This test iterates over all sensors, registers a valid event listener, asserts
 * the call returns true, then unregisters it and asserts success.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 011@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Call getMotionSensor for each sensor ID | Input: sensorId | Valid sensor handle returned | Should Pass |
 * | 03 | Call registerEventListener with a valid listener | Input: eventListener | Returns true | Should Pass |
 * | 04 | Call unregisterEventListener with the same listener | Input: eventListener | Returns true | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Register_UnRegisterEventListener_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorEventListener> eventListener = android::sp<MotionSensorEventListener>::make();
        MOTIONSENSOR_REGISTER_EVENT_SUCCESS(sensorHandle, eventListener);
        MOTIONSENSOR_UNREGISTER_EVENT_SUCCESS(sensorHandle, eventListener);
    }
}

/**
 * @brief Validate that registering the same event listener twice returns false on the second call.
 *
 * This test registers a valid event listener, then attempts to register the same listener
 * again, asserting the second call returns false with status ok (EX_NONE).
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 012@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Call getMotionSensor for each sensor ID | Input: sensorId | Valid sensor handle returned | Should Pass |
 * | 03 | Register the event listener | Input: eventListener | Returns true | Should Pass |
 * | 04 | Register the same listener again | Input: eventListener | Returns false, status EX_NONE | Should Pass |
 * | 05 | Unregister the event listener | Input: eventListener | Returns true | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, RegisterEventListener_Negative_AlreadyRegistered)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorEventListener> eventListener = android::sp<MotionSensorEventListener>::make();
        MOTIONSENSOR_REGISTER_EVENT_SUCCESS(sensorHandle, eventListener);
        // Second registration of the same listener must return false, status ok
        MOTIONSENSOR_REGISTER_EVENT_FAILURE(sensorHandle, eventListener, Status::EX_NONE);
        MOTIONSENSOR_UNREGISTER_EVENT_SUCCESS(sensorHandle, eventListener);
    }
}


/**
 * @brief Validate that unregistering a listener that was never registered returns false.
 *
 * This test creates a fresh event listener (never registered) and calls unregisterEventListener,
 * asserting it returns false with status ok (EX_NONE).
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 013@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Call getMotionSensor for each sensor ID | Input: sensorId | Valid sensor handle returned | Should Pass |
 * | 03 | Call unregisterEventListener with a listener never registered | Input: eventListener | Returns false, status EX_NONE | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, UnregisterEventListener_Negative_NotRegistered)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        // Listener was never registered
        android::sp<IMotionSensorEventListener> eventListener = android::sp<MotionSensorEventListener>::make();
        MOTIONSENSOR_UNREGISTER_EVENT_FAILURE(sensorHandle, eventListener, Status::EX_NONE);
    }
}

/**
 * @brief Validate that start and stop succeed for each sensor with a valid configuration.
 *
 * This test opens each sensor, calls start with a valid MOTION configuration,
 * asserts success, then calls stop and close.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 014@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle and open with valid listener | Input: sensorId, listener | Controller returned | Should Pass |
 * | 03 | Call start with mode=MOTION, noMotionSeconds=30 | Input: config | Start succeeds | Should Pass |
 * | 04 | Call stop | Input: none | Stop succeeds | Should Pass |
 * | 05 | Call close | Input: controllerHandle | Close succeeds | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Start_Stop_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        StartConfig config;
        config.operationalMode = OperationalMode::MOTION;
        config.noMotionSeconds = 30;
        MOTIONSENSOR_START_SUCCESS(controllerHandle, config);
        MOTIONSENSOR_STOP_SUCCESS(controllerHandle);
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Validate that calling start while already started returns EX_ILLEGAL_STATE.
 *
 * This test starts a sensor successfully, then attempts a second start call on the
 * same controller, asserting EX_ILLEGAL_STATE is returned.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 015@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle and open with valid listener | Input: sensorId, listener | Controller returned | Should Pass |
 * | 03 | Call start with valid config | Input: mode=MOTION, noMotionSeconds=30 | Start succeeds | Should Pass |
 * | 04 | Call start again while already started | Input: same config | Returns EX_ILLEGAL_STATE | Should Pass |
 * | 05 | Stop and close | Input: controllerHandle | Stop and close succeed | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Start_Negative_AlreadyStarted)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        StartConfig config;
        config.operationalMode = OperationalMode::MOTION;
        config.noMotionSeconds = 30;
        MOTIONSENSOR_START_SUCCESS(controllerHandle, config);
        // Second start while STARTED must fail with EX_ILLEGAL_STATE
        MOTIONSENSOR_START_FAILURE(controllerHandle, config, Status::EX_ILLEGAL_STATE);
        MOTIONSENSOR_STOP_SUCCESS(controllerHandle);
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Validate that start returns EX_ILLEGAL_ARGUMENT when given an invalid noMotionSeconds value.
 *
 * This test calls start with noMotionSeconds=99999 (out of valid range), asserting
 * that EX_ILLEGAL_ARGUMENT is returned and the sensor remains in STOPPED state.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 016@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle and open with valid listener | Input: sensorId, listener | Controller returned | Should Pass |
 * | 03 | Call start with noMotionSeconds=99999 | Input: mode=MOTION, noMotionSeconds=99999 | Returns EX_ILLEGAL_ARGUMENT | Should Pass |
 * | 04 | Close the sensor | Input: controllerHandle | Close succeeds | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Start_Negative_InvalidConfig)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        StartConfig config;
        config.operationalMode = OperationalMode::MOTION;
        config.noMotionSeconds = 99999; // out of range
        MOTIONSENSOR_START_FAILURE(controllerHandle, config, Status::EX_ILLEGAL_ARGUMENT);
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Validate that stop returns EX_ILLEGAL_STATE when the sensor is already in STOPPED state.
 *
 * This test opens a sensor without starting it, then calls stop, asserting
 * that EX_ILLEGAL_STATE is returned.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 017@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle and open with valid listener | Input: sensorId, listener | Controller returned | Should Pass |
 * | 03 | Call stop without having called start | Input: none | Returns EX_ILLEGAL_STATE | Should Pass |
 * | 04 | Close the sensor | Input: controllerHandle | Close succeeds | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, Stop_Negative_AlreadyStopped)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        // Not started — must fail with EX_ILLEGAL_STATE
        MOTIONSENSOR_STOP_FAILURE(controllerHandle, Status::EX_ILLEGAL_STATE);
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Validate that getStartConfig returns the configuration passed to start.
 *
 * This test starts each sensor with noMotionSeconds=60, calls getStartConfig,
 * and asserts the retrieved noMotionSeconds matches the configured value.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 018@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle and open with valid listener | Input: sensorId, listener | Controller returned | Should Pass |
 * | 03 | Call start with mode=MOTION, noMotionSeconds=60 | Input: config | Start succeeds | Should Pass |
 * | 04 | Call getStartConfig | Output: retrieved | retrieved.noMotionSeconds == 60 | Should Pass |
 * | 05 | Stop and close | Input: controllerHandle | Stop and close succeed | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetStartConfig_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        StartConfig config;
        config.operationalMode = OperationalMode::MOTION;
        config.noMotionSeconds = 60;
        MOTIONSENSOR_START_SUCCESS(controllerHandle, config);
        StartConfig retrieved;
        MOTIONSENSOR_GET_START_CONFIG_SUCCESS(controllerHandle, retrieved);
        UT_ASSERT_EQUAL(retrieved.noMotionSeconds, config.noMotionSeconds);
        MOTIONSENSOR_STOP_SUCCESS(controllerHandle);
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Validate that getStartConfig returns EX_ILLEGAL_STATE when the sensor is stopped.
 *
 * This test opens each sensor without starting it, then calls getStartConfig,
 * asserting EX_ILLEGAL_STATE is returned.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 019@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle and open with valid listener | Input: sensorId, listener | Controller returned | Should Pass |
 * | 03 | Call getStartConfig while in STOPPED state | Output: config | Returns EX_ILLEGAL_STATE | Should Pass |
 * | 04 | Close the sensor | Input: controllerHandle | Close succeeds | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetStartConfig_Negative_Stopped)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        StartConfig config;
        MOTIONSENSOR_GET_START_CONFIG_FAILURE(controllerHandle, config, Status::EX_ILLEGAL_STATE);
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Validate that getSensitivity returns a non-negative value for each sensor.
 *
 * This test opens each sensor and calls getSensitivity on the controller,
 * asserting the returned value is >= 0.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 020@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle and open with valid listener | Input: sensorId, listener | Controller returned | Should Pass |
 * | 03 | Call getSensitivity | Output: sensitivity | sensitivity >= 0 | Should Pass |
 * | 04 | Close the sensor | Input: controllerHandle | Close succeeds | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetSensitivity_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        int32_t sensitivity;
        MOTIONSENSOR_GET_SENSITIVITY_SUCCESS(controllerHandle, sensitivity);
        UT_ASSERT_TRUE(sensitivity >= 0);
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Validate that setSensitivity succeeds for min, max, and midpoint values.
 *
 * This test reads capabilities to obtain valid sensitivity bounds, then calls
 * setSensitivity with minSensitivity, maxSensitivity, and their midpoint,
 * asserting each call succeeds.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 021@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle, open, and get capabilities | Input: sensorId, listener | Controller and caps returned | Should Pass |
 * | 03 | Call setSensitivity with caps.minSensitivity | Input: minSensitivity | Returns true | Should Pass |
 * | 04 | Call setSensitivity with caps.maxSensitivity | Input: maxSensitivity | Returns true | Should Pass |
 * | 05 | Call setSensitivity with midpoint (min+max)/2 | Input: midpoint | Returns true | Should Pass |
 * | 06 | Close the sensor | Input: controllerHandle | Close succeeds | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetSensitivity_Positive_MinMaxRange)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        Capabilities caps;
        MOTIONSENSOR_GET_CAPABILITIES_SUCCESS(sensorHandle, caps);
        MOTIONSENSOR_SET_SENSITIVITY_SUCCESS(controllerHandle, caps.minSensitivity);
        MOTIONSENSOR_SET_SENSITIVITY_SUCCESS(controllerHandle, caps.maxSensitivity);
        MOTIONSENSOR_SET_SENSITIVITY_SUCCESS(controllerHandle, (caps.minSensitivity+caps.maxSensitivity)/2);

        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}


/**
 * @brief Validate that setSensitivity returns EX_ILLEGAL_ARGUMENT for out-of-range values.
 *
 * This test calls setSensitivity with minSensitivity-1 and maxSensitivity+1,
 * asserting both calls return EX_ILLEGAL_ARGUMENT.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 022@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle, open, and get capabilities | Input: sensorId, listener | Controller and caps returned | Should Pass |
 * | 03 | Call setSensitivity with caps.minSensitivity-1 | Input: minSensitivity-1 | Returns EX_ILLEGAL_ARGUMENT | Should Pass |
 * | 04 | Call setSensitivity with caps.maxSensitivity+1 | Input: maxSensitivity+1 | Returns EX_ILLEGAL_ARGUMENT | Should Pass |
 * | 05 | Close the sensor | Input: controllerHandle | Close succeeds | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetSensitivity_OutOfRange)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        Capabilities caps;
        MOTIONSENSOR_GET_CAPABILITIES_SUCCESS(sensorHandle, caps);
        MOTIONSENSOR_SET_SENSITIVITY_FAILURE(controllerHandle, caps.minSensitivity - 1, Status::EX_ILLEGAL_ARGUMENT);
        MOTIONSENSOR_SET_SENSITIVITY_FAILURE(controllerHandle, caps.maxSensitivity + 1, Status::EX_ILLEGAL_ARGUMENT);

        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}


/**
 * @brief Validate that setSensitivity returns EX_ILLEGAL_STATE when the sensor is started.
 *
 * This test starts each sensor, then calls setSensitivity, asserting that
 * EX_ILLEGAL_STATE is returned since sensitivity cannot be changed while running.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 023@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle, open, and start | Input: sensorId, listener, config | Sensor started successfully | Should Pass |
 * | 03 | Get capabilities to obtain minSensitivity | Output: caps | Capabilities returned | Should Pass |
 * | 04 | Call setSensitivity while in STARTED state | Input: caps.minSensitivity | Returns EX_ILLEGAL_STATE | Should Pass |
 * | 05 | Stop and close | Input: controllerHandle | Stop and close succeed | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetSensitivity_Negative_WhileStarted)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        StartConfig config;
        config.operationalMode = OperationalMode::MOTION;
        config.noMotionSeconds = 30;
        MOTIONSENSOR_START_SUCCESS(controllerHandle, config);
        Capabilities caps;
        MOTIONSENSOR_GET_CAPABILITIES_SUCCESS(sensorHandle, caps);
        MOTIONSENSOR_SET_SENSITIVITY_FAILURE(controllerHandle, caps.minSensitivity, Status::EX_ILLEGAL_STATE);
        MOTIONSENSOR_STOP_SUCCESS(controllerHandle);
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Validate that isAutonomousDuringDeepSleepEnabled returns the expected value based on capabilities.
 *
 * This test opens each sensor, reads its capabilities, calls isAutonomousDuringDeepSleepEnabled,
 * and asserts the returned value matches the supportsDeepSleepAutonomy capability flag.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 024@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle, open, and get capabilities | Input: sensorId, listener | Controller and caps returned | Should Pass |
 * | 03 | Call isAutonomousDuringDeepSleepEnabled | Output: enabled | Value matches caps.supportsDeepSleepAutonomy | Should Pass |
 * | 04 | Close the sensor | Input: controllerHandle | Close succeeds | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetAutonomous_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        Capabilities caps;
        MOTIONSENSOR_GET_CAPABILITIES_SUCCESS(sensorHandle, caps);
        bool enabled;
        MOTIONSENSOR_GET_AUTONOMOUS_SUCCESS(controllerHandle, enabled, false);
        MOTIONSENSOR_SET_AUTONOMOUS_SUCCESS(controllerHandle, true);
        MOTIONSENSOR_GET_AUTONOMOUS_SUCCESS(controllerHandle, enabled, caps.supportsDeepSleepAutonomy);
        UT_ASSERT_EQUAL(enabled, caps.supportsDeepSleepAutonomy);
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Validate that setAutonomousDuringDeepSleep succeeds when the sensor is stopped.
 *
 * This test opens each sensor and calls setAutonomousDuringDeepSleep(true),
 * asserting the call succeeds with status ok.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 025@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle, open, and get capabilities | Input: sensorId, listener | Controller and caps returned | Should Pass |
 * | 03 | Call setAutonomousDuringDeepSleep(true) | Input: enabled=true | Call succeeds, status ok | Should Pass |
 * | 04 | Close the sensor | Input: controllerHandle | Close succeeds | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetAutonomous_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        Capabilities caps;
        MOTIONSENSOR_GET_CAPABILITIES_SUCCESS(sensorHandle, caps);
        MOTIONSENSOR_SET_AUTONOMOUS_SUCCESS(controllerHandle, true);
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Validate that setAutonomousDuringDeepSleep returns EX_ILLEGAL_STATE when started.
 *
 * This test starts each sensor and then calls setAutonomousDuringDeepSleep(true),
 * asserting EX_ILLEGAL_STATE is returned since the setting cannot be changed while running.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 026@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle, open, and start | Input: sensorId, listener, config | Sensor started | Should Pass |
 * | 03 | Call setAutonomousDuringDeepSleep(true) while started | Input: enabled=true | Returns EX_ILLEGAL_STATE | Should Pass |
 * | 04 | Stop and close | Input: controllerHandle | Stop and close succeed | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetAutonomous_Negative_WhileStarted)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        MotionSensorId sensorIdObj;
        assertSensorIdInHFP(idOpt->value);
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        StartConfig config;
        config.operationalMode = OperationalMode::MOTION;
        config.noMotionSeconds = 30;
        MOTIONSENSOR_START_SUCCESS(controllerHandle, config);
        MOTIONSENSOR_SET_AUTONOMOUS_FAILURE(controllerHandle, true, Status::EX_ILLEGAL_STATE);
        MOTIONSENSOR_STOP_SUCCESS(controllerHandle);
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Validate that setActiveWindows succeeds for an empty list, a normal window, and a midnight-wrapping window.
 *
 * This test calls setActiveWindows with an empty vector (24-hour monitoring),
 * then with two windows including one that wraps past midnight, asserting all calls succeed.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 027@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle and open with valid listener | Input: sensorId, listener | Controller returned | Should Pass |
 * | 03 | Call setActiveWindows with empty vector | Input: windows = {} | Returns true | Should Pass |
 * | 04 | Call setActiveWindows with 08:00-20:00 and 22:00-06:00 windows | Input: 2 windows | Returns true | Should Pass |
 * | 05 | Close the sensor | Input: controllerHandle | Close succeeds | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetActiveWindows_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        MotionSensorId sensorIdObj;
        assertSensorIdInHFP(idOpt->value);
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        std::vector<TimeWindow> windows;
        TimeWindow w;
        MOTIONSENSOR_SET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, windows);

        w.startTimeOfDaySeconds = 28800; // 08:00
        w.endTimeOfDaySeconds   = 72000; // 20:00
        windows.push_back(w);
        w.startTimeOfDaySeconds = 79200; // 22:00
        w.endTimeOfDaySeconds   = 21600; // 06:00 (wraps midnight)
        windows.push_back(w);

        MOTIONSENSOR_SET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, windows);
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Validate that setActiveWindows returns EX_ILLEGAL_ARGUMENT for out-of-range time values.
 *
 * This test calls setActiveWindows with endTimeOfDaySeconds=90000 (exceeds 86399)
 * and with startTimeOfDaySeconds=-10 (negative), asserting both fail with EX_ILLEGAL_ARGUMENT.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 028@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle and open with valid listener | Input: sensorId, listener | Controller returned | Should Pass |
 * | 03 | Call setActiveWindows with endTimeOfDaySeconds=90000 | Input: window (0 to 90000) | Returns EX_ILLEGAL_ARGUMENT | Should Pass |
 * | 04 | Call setActiveWindows with startTimeOfDaySeconds=-10 | Input: window (-10 to 21600) | Returns EX_ILLEGAL_ARGUMENT | Should Pass |
 * | 05 | Close the sensor | Input: controllerHandle | Close succeeds | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetActiveWindows_Negative_OutOfRange)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        MotionSensorId sensorIdObj;
        assertSensorIdInHFP(idOpt->value);
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        std::vector<TimeWindow> windows;
        TimeWindow w;
        w.startTimeOfDaySeconds = 0;
        w.endTimeOfDaySeconds   = 90000; // > 86399
        windows.push_back(w);
        MOTIONSENSOR_SET_ACTIVE_WINDOWS_FAILURE(controllerHandle, windows, Status::EX_ILLEGAL_ARGUMENT);
        w.startTimeOfDaySeconds = -10;
        w.endTimeOfDaySeconds   = 21600; // > 86399
        windows.push_back(w);
        MOTIONSENSOR_SET_ACTIVE_WINDOWS_FAILURE(controllerHandle, windows, Status::EX_ILLEGAL_ARGUMENT);

        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Validate that setActiveWindows returns EX_ILLEGAL_STATE when the sensor is started.
 *
 * This test starts each sensor, then calls setActiveWindows, asserting
 * EX_ILLEGAL_STATE is returned since active windows cannot be changed while running.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 029@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle, open, and start | Input: sensorId, listener, config | Sensor started | Should Pass |
 * | 03 | Call setActiveWindows with a valid window while started | Input: window (0 to 3600) | Returns EX_ILLEGAL_STATE | Should Pass |
 * | 04 | Stop and close | Input: controllerHandle | Stop and close succeed | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, SetActiveWindows_Negative_WhileStarted)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        StartConfig config;
        config.operationalMode = OperationalMode::MOTION;
        config.noMotionSeconds = 30;
        MOTIONSENSOR_START_SUCCESS(controllerHandle, config);
        std::vector<TimeWindow> windows;
        TimeWindow w;
        w.startTimeOfDaySeconds = 0;
        w.endTimeOfDaySeconds   = 3600;
        windows.push_back(w);
        MOTIONSENSOR_SET_ACTIVE_WINDOWS_FAILURE(controllerHandle, windows, Status::EX_ILLEGAL_STATE);
        MOTIONSENSOR_STOP_SUCCESS(controllerHandle);
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Validate that getActiveWindows returns the windows previously set by setActiveWindows.
 *
 * This test sets one active window (08:00-20:00), calls getActiveWindows, and asserts
 * the retrieved windows match the configured values.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 030@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle and open with valid listener | Input: sensorId, listener | Controller returned | Should Pass |
 * | 03 | Call setActiveWindows with one window (28800 to 72000) | Input: window | Returns true | Should Pass |
 * | 04 | Call getActiveWindows | Output: retrieved | retrieved.size()==1, times match configured values | Should Pass |
 * | 05 | Close the sensor | Input: controllerHandle | Close succeeds | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, GetActiveWindows_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        assertSensorIdInHFP(idOpt->value);
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        std::vector<TimeWindow> windows;
        TimeWindow w;
        w.startTimeOfDaySeconds = 28800;
        w.endTimeOfDaySeconds   = 72000;
        windows.push_back(w);
        MOTIONSENSOR_SET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, windows);
        std::vector<TimeWindow> retrieved;
        MOTIONSENSOR_GET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, retrieved);
        UT_ASSERT_EQUAL(retrieved.size(), windows.size());
        UT_ASSERT_EQUAL(retrieved[0].startTimeOfDaySeconds, w.startTimeOfDaySeconds);
        UT_ASSERT_EQUAL(retrieved[0].endTimeOfDaySeconds,   w.endTimeOfDaySeconds);
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Validate that clearActiveWindows removes all previously set windows.
 *
 * This test sets one active window, calls clearActiveWindows, then calls
 * getActiveWindows and asserts the returned list is empty.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 031@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle and open with valid listener | Input: sensorId, listener | Controller returned | Should Pass |
 * | 03 | Call setActiveWindows with one window (28800 to 72000) | Input: window | Returns true | Should Pass |
 * | 04 | Call clearActiveWindows | Input: none | Returns true | Should Pass |
 * | 05 | Call getActiveWindows and verify empty | Output: retrieved | retrieved is empty | Should Pass |
 * | 06 | Close the sensor | Input: controllerHandle | Close succeeds | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, ClearActiveWindows_Positive)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        assertSensorIdInHFP(idOpt->value);
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        std::vector<TimeWindow> windows;
        TimeWindow w;
        w.startTimeOfDaySeconds = 28800;
        w.endTimeOfDaySeconds   = 72000;
        windows.push_back(w);
        MOTIONSENSOR_SET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, windows);
        MOTIONSENSOR_CLEAR_ACTIVE_WINDOWS_SUCCESS(controllerHandle);
        std::vector<TimeWindow> retrieved;
        MOTIONSENSOR_GET_ACTIVE_WINDOWS_SUCCESS(controllerHandle, retrieved);
        UT_ASSERT_TRUE(retrieved.empty());
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}

/**
 * @brief Validate that clearActiveWindows returns EX_ILLEGAL_STATE when the sensor is started.
 *
 * This test starts each sensor, then calls clearActiveWindows, asserting
 * EX_ILLEGAL_STATE is returned since windows cannot be cleared while the sensor is running.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 032@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call getMotionSensorIds and iterate all sensors | Output: sensorIdsOpt | Sensor IDs list obtained | Should be successful |
 * | 02 | Get sensor handle, open, and start | Input: sensorId, listener, config | Sensor started | Should Pass |
 * | 03 | Call clearActiveWindows while in STARTED state | Input: none | Returns EX_ILLEGAL_STATE | Should Pass |
 * | 04 | Stop and close | Input: controllerHandle | Stop and close succeed | Should Pass |
 */
UT_ADD_TEST(VTS_L1_MOTIONSENSOR, ClearActiveWindows_Negative_WhileStarted)
{
    std::optional<std::vector<std::optional<IMotionSensor::Id>>> sensorIdsOpt;
    MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdsOpt);
    if (!sensorIdsOpt || sensorIdsOpt->empty())
    {
        UT_LOG_WARNING("No sensors available");
        return;
    }
    for (const auto& idOpt : *sensorIdsOpt)
    {
        if (!idOpt)
            continue;
        MotionSensorId sensorIdObj;
        sensorIdObj.value = idOpt->value;
        assertSensorIdInHFP(idOpt->value);
        android::sp<IMotionSensor> sensorHandle;
        MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, sensorIdObj, sensorHandle);
        android::sp<IMotionSensorControllerListener> listener = android::sp<MotionSensorControllerListener>::make();
        android::sp<IMotionSensorController> controllerHandle;
        MOTIONSENSOR_OPEN_SUCCESS(sensorHandle, listener, controllerHandle);
        StartConfig config;
        config.operationalMode = OperationalMode::MOTION;
        config.noMotionSeconds = 30;
        MOTIONSENSOR_START_SUCCESS(controllerHandle, config);
        MOTIONSENSOR_CLEAR_ACTIVE_WINDOWS_FAILURE(controllerHandle, Status::EX_ILLEGAL_STATE);
        MOTIONSENSOR_STOP_SUCCESS(controllerHandle);
        MOTIONSENSOR_CLOSE_SUCCESS(sensorHandle, controllerHandle);
    }
}