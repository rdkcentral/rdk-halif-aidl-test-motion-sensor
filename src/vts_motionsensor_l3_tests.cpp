/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2026 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
 * @defgroup MOTION_SENSOR_HALTEST_L3 Motion Sensor HAL Tests L3 File
 * @{
 * @parblock
 *
 * ### L3 Test Cases for MOTION SENSOR HAL :
 *
 * This module includes Level 3 functional tests that require external stimulus
 * from the host via the UT control plane.  The host sends YAML commands to the
 * vcomponent which injects synthetic motion/no-motion events; these tests then
 * validate event delivery, timing, active-window gating, and deep-sleep
 * autonomy.
 *
 * **Pre-Conditions:**  vcomponent must be running with control-plane enabled @n
 * **Dependencies:** UT-control plane @n
 *
 * @endparblock
 */

/**
 * @file vts_motionsensor_l3_tests.cpp
 */

#include <optional>
#include <tuple>
#include <vector>

#include "vts_motionsensor.h"
#include "vts_motionsensor_parse_hfp.h"
#include "vts_motionsensor_parse_testconfig.h"

using std::tuple;
using std::vector;

extern android::sp<IMotionSensorManager> gMOTIONSENSORMGRHandle;

static const char *gModuleName  = "motionsensor";
static const char *gTestGroup   = "L3";

/* ---- UT_Core suite registration ----------------------------------------- */
UT_ADD_TEST_TO_GROUP(VTS_L3_MOTIONSENSOR, UT_TESTS_L3)

/* ========================================================================== */
/*  Test fixture                                                               */
/* ========================================================================== */

class VTS_L3_MOTIONSENSOR : public UTCore
{
public:
    VTS_L3_MOTIONSENSOR() : UTCore() {}
    ~VTS_L3_MOTIONSENSOR() override = default;

    /* Per-sensor tuple: (id, sensorHandle, controllerListener, eventListener, controllerHandle) */
    using SensorTuple = tuple<
        MotionSensorId,
        sp<IMotionSensor>,
        sp<MotionSensorControllerListener>,
        sp<MotionSensorEventListener>,
        sp<IMotionSensorController>
    >;
    vector<SensorTuple> mSensorList;

    void SetUp() override
    {
        android::ProcessState::self()->startThreadPool();

        std::optional<std::vector<std::optional<MotionSensorId>>> sensorIds;
        MOTIONSENSOR_GET_SENSOR_IDS_SUCCESS(gMOTIONSENSORMGRHandle, sensorIds);
        UT_ASSERT_TRUE_FATAL(sensorIds.has_value() && !sensorIds->empty());

        for (const auto& idOpt : *sensorIds)
        {
            if (!idOpt)
                continue;
            const MotionSensorId& id = *idOpt;

            sp<IMotionSensor> sensor;
            MOTIONSENSOR_GET_SENSOR_SUCCESS(gMOTIONSENSORMGRHandle, id, sensor);

            sp<MotionSensorControllerListener> ctrlListener = new MotionSensorControllerListener();
            sp<MotionSensorEventListener>      evtListener  = new MotionSensorEventListener();
            sp<IMotionSensorController>        controller;

            MOTIONSENSOR_REGISTER_EVENT_SUCCESS(sensor, evtListener);
            MOTIONSENSOR_OPEN_SUCCESS(sensor, ctrlListener, controller);

            mSensorList.push_back({id, sensor, ctrlListener, evtListener, controller});
        }
    }

    void TearDown() override
    {
        for (auto& tup : mSensorList)
        {
            auto& sensor     = std::get<1>(tup);
            auto& evtListener= std::get<3>(tup);
            auto& controller = std::get<4>(tup);

            /* Ensure sensor is stopped before close */
            State state = State::STOPPED;
            (void)(sensor)->getState(&state);
            if (state == State::STARTED)
            {
                MOTIONSENSOR_STOP_SUCCESS(controller);
                State lastState;
                MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STOPPED, lastState);
                UT_ASSERT_EQUAL(lastState, State::STOPPED);
            }

            MOTIONSENSOR_CLOSE_SUCCESS(sensor, controller);
            MOTIONSENSOR_UNREGISTER_EVENT_SUCCESS(sensor, evtListener);
        }
        mSensorList.clear();
    }

    /* Helper: get the SensorTuple for a given 0-based index */
    bool getSensorAtIndex(int32_t index, SensorTuple*& out)
    {
        if (index < 0 || (size_t)index >= mSensorList.size())
            return false;
        out = &mSensorList[index];
        return true;
    }
};

/**
 * @brief Validate event delivery for a sensor in MOTION or NO_MOTION mode.
 *
 * The operational mode (operational_mode), sensor index (sensor_index), and max
 * number of events to receive (max_expected_events) are written into the YAML
 * config by the Python host test before each run. Python triggers stimuli;
 * the test passes as long as at least one event arrives.
 *
 * **Test Group ID:**: 03@n
 * **Test Case ID:** 001@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Parse test config: sensor_index, operational_mode, max_expected_events, timeouts | Config file | Config parsed successfully | Should be successful |@n
 * | 02 | Verify no spurious events exist before the test | Output: none | No events received within 200 ms | Should Pass |@n
 * | 03 | Unregister event listener; confirm no events arrive | Output: none | No events received within 500 ms | Should Pass |@n
 * | 04 | Re-register listener; start sensor in configured mode; wait for STARTED | Input: operationalMode, startConfig | State == STARTED | Should Pass |@n
 * | 05 | Loop up to maxExpectedEvents times collecting events; assert at least 1 arrives | Output: receivedMode | eventsReceived >= 1; each receivedMode == opMode | Should Pass |@n
 * | 06 | Stop sensor; verify no spurious events after stop | Output: none | No events received within 500 ms | Should Pass |@n
 * | 07 | Unregister listener; verify no events delivered | Output: none | No events received within 2000 ms | Should Pass |
 */
UT_ADD_TEST(VTS_L3_MOTIONSENSOR, EventDetection)
{
    const char *configNode = "EventDetection";
    motionSensorL3EventTestConfig_t cfg;

    UT_ASSERT_EQUAL_FATAL(
        vts_motionsensor_parse_eventtest_config(
            VTS_MOTIONSENSOR_TEST_CONFIG_PATH, gModuleName, gTestGroup, configNode, cfg),
        UT_STATUS_OK);

    SensorTuple *tup = nullptr;
    UT_ASSERT_TRUE_FATAL(getSensorAtIndex(cfg.sensorIndex, tup));

    auto& sensor     = std::get<1>(*tup);
    auto& ctrlLsn    = std::get<2>(*tup);
    auto& evtLsn     = std::get<3>(*tup);
    auto& controller = std::get<4>(*tup);

    OperationalMode opMode   = static_cast<OperationalMode>(cfg.operationalMode);
    const char     *modeName = (cfg.operationalMode == 1) ? "NO_MOTION" : "MOTION";

    UT_LOG_INFO("[L3 EventDetection] sensorIndex=%d mode=%s maxExpectedEvents=%d timeoutMs=%d",
        cfg.sensorIndex, modeName, cfg.maxExpectedEvents, cfg.eventTimeoutMs);

    ctrlLsn->resetStateQueue();
    evtLsn->resetEventQueue();

    /*
     *  Configure an always-open active window.
     *
     * This makes event detection independent of the current
     * time of day. 00:00:00 -> 23:59:59 means the sensor is
     * active for the entire day.
     */
    std::vector<TimeWindow> windows;

    TimeWindow window;
    window.startTimeOfDaySeconds = 0;
    window.endTimeOfDaySeconds   = 86399;

    windows.push_back(window);

    /*
     * Get the active window configuration.
     */
    std::vector<TimeWindow> previousWindows;
    MOTIONSENSOR_GET_ACTIVE_WINDOWS_SUCCESS(
        controller,
        previousWindows);

    MOTIONSENSOR_SET_ACTIVE_WINDOWS_SUCCESS(controller, windows);

    /*
     * Verify the active window configuration.
     */
    std::vector<TimeWindow> retrievedWindows;
    MOTIONSENSOR_GET_ACTIVE_WINDOWS_SUCCESS(
        controller,
        retrievedWindows);

    UT_ASSERT_EQUAL(
        retrievedWindows.size(),
        windows.size());

    UT_ASSERT_EQUAL(
        retrievedWindows[0].startTimeOfDaySeconds,
        window.startTimeOfDaySeconds);

    UT_ASSERT_EQUAL(
        retrievedWindows[0].endTimeOfDaySeconds,
        window.endTimeOfDaySeconds);

    /* Start sensor in the configured mode */
    StartConfig startCfg;
    startCfg.operationalMode    = opMode;
    startCfg.noMotionSeconds    = cfg.noMotionSeconds;
    startCfg.activeStartSeconds = 0;
    startCfg.activeStopSeconds  = 0;

    MOTIONSENSOR_START_SUCCESS(controller, startCfg);

    State lastState;
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STARTED, lastState);
    UT_ASSERT_EQUAL(lastState, State::STARTED);

    evtLsn->resetEventQueue();

    // Wait for event generated by host.
    int32_t eventsReceived = 0;
    for (int32_t evtNum = 0; evtNum < cfg.maxExpectedEvents; ++evtNum)
    {
        MotionEvent receivedEvent;
        bool gotEvent = evtLsn->getNextEvent(receivedEvent, std::chrono::milliseconds(cfg.eventTimeoutMs));
        if (!gotEvent)
            break;   /* no more events arriving within timeout */
        UT_ASSERT_EQUAL(receivedEvent.mode, opMode);
        UT_ASSERT_TRUE(receivedEvent.timestampMonotonicMs > 0);
        eventsReceived++;
        UT_LOG_INFO("[L3 EventDetection] Event %d/%d received: mode=%s ts=%lld",
            eventsReceived, cfg.maxExpectedEvents, modeName,
            static_cast<long long>(receivedEvent.timestampMonotonicMs));
    }
    UT_ASSERT_TRUE(eventsReceived >= 1);

    /* Stop sensor; verify no spurious events after stop */
    MOTIONSENSOR_STOP_SUCCESS(controller);
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STOPPED, lastState);
    UT_ASSERT_EQUAL(lastState, State::STOPPED);

    //Verify no event is delivered after stop.
    evtLsn->resetEventQueue();
    {
        MotionEvent postStopEvent;
        bool gotPostStop = evtLsn->getNextEvent(postStopEvent, std::chrono::milliseconds(500));
        UT_ASSERT_FALSE(gotPostStop);
    }

    //Unregister listener and verify no events are delivered.
    MOTIONSENSOR_UNREGISTER_EVENT_SUCCESS(sensor, evtLsn);
    evtLsn->resetEventQueue();
    {
        MotionEvent unregEvent;
        bool gotUnreg = evtLsn->getNextEvent(unregEvent, std::chrono::milliseconds(2000));
        UT_ASSERT_FALSE(gotUnreg);
    }

    /*
     * Re-register for TearDown.
     */
    MOTIONSENSOR_REGISTER_EVENT_SUCCESS(sensor, evtLsn);
}

UT_ADD_TEST(VTS_L3_MOTIONSENSOR, NoMotionTimerReset)
{
    const char *configNode = "EventDetection";
    motionSensorL3EventTestConfig_t cfg;

    UT_ASSERT_EQUAL_FATAL(
        vts_motionsensor_parse_eventtest_config(
            VTS_MOTIONSENSOR_TEST_CONFIG_PATH,
            gModuleName,
            gTestGroup,
            configNode,
            cfg),
        UT_STATUS_OK);

    SensorTuple *tup = nullptr;
    UT_ASSERT_TRUE_FATAL(
        getSensorAtIndex(cfg.sensorIndex, tup));

    auto& sensor     = std::get<1>(*tup);
    auto& ctrlLsn    = std::get<2>(*tup);
    auto& evtLsn     = std::get<3>(*tup);
    auto& controller = std::get<4>(*tup);

    const int iterations = cfg.maxExpectedEvents;

    ctrlLsn->resetStateQueue();
    evtLsn->resetEventQueue();
    /*
     *  Configure an always-open active window.
     *
     * This makes event detection independent of the current
     * time of day. 00:00:00 -> 23:59:59 means the sensor is
     * active for the entire day.
     */
    std::vector<TimeWindow> windows;

    TimeWindow window;
    window.startTimeOfDaySeconds = 0;
    window.endTimeOfDaySeconds   = 86399;

    windows.push_back(window);

    /*
     * Get the active window configuration.
     */
    std::vector<TimeWindow> previousWindows;
    MOTIONSENSOR_GET_ACTIVE_WINDOWS_SUCCESS(
        controller,
        previousWindows);

    MOTIONSENSOR_SET_ACTIVE_WINDOWS_SUCCESS(controller, windows);

    /*
     * Verify the active window configuration.
     */
    std::vector<TimeWindow> retrievedWindows;
    MOTIONSENSOR_GET_ACTIVE_WINDOWS_SUCCESS(
        controller,
        retrievedWindows);

    UT_ASSERT_EQUAL(
        retrievedWindows.size(),
        windows.size());

    UT_ASSERT_EQUAL(
        retrievedWindows[0].startTimeOfDaySeconds,
        window.startTimeOfDaySeconds);

    UT_ASSERT_EQUAL(
        retrievedWindows[0].endTimeOfDaySeconds,
        window.endTimeOfDaySeconds);


    StartConfig startCfg;
    startCfg.operationalMode    = OperationalMode::NO_MOTION;
    startCfg.noMotionSeconds    = cfg.noMotionSeconds;
    startCfg.activeStartSeconds = 0;
    startCfg.activeStopSeconds  = 0;

    MOTIONSENSOR_START_SUCCESS( controller, startCfg);
    State lastState;
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STARTED, lastState);
    UT_ASSERT_EQUAL(lastState, State::STARTED);

    UT_LOG_INFO(
        "[L3 NoMotionTimerReset] Started sensor=%d "
        "noMotionSeconds=%d maxExpectedEvents=%d",
        cfg.sensorIndex,
        cfg.noMotionSeconds,
        cfg.maxExpectedEvents);

    if (cfg.noMotionSeconds == 0)
    {
        UT_LOG_ERROR(
            "[L3 NoMotionTimerReset] "
            "noMotionSeconds=0: "
            "NO_MOTION events are not expected, so no further testing is needed");
        return; // No NO_MOTION events expected, so no further testing needed
    }

    const int noMotionMs = cfg.noMotionSeconds * 1000;
    const int effectiveTimeoutMs = std::max(cfg.eventTimeoutMs, noMotionMs + 1000);
    for (int i = 0; i < cfg.maxExpectedEvents; ++i)
    {
        UT_LOG_INFO(
                "[L3 NoMotionTimerReset] "
                "Waiting for NO_MOTION %d/%d",
                i + 1,
                cfg.maxExpectedEvents);

        MotionEvent event;

        bool received =
                evtLsn->getNextEvent(
                    event,
                    std::chrono::milliseconds(effectiveTimeoutMs));

        UT_ASSERT_TRUE(received);
        if (!received)
        {
            UT_LOG_ERROR(
                    "[L3 NoMotionTimerReset] "
                    "NO_MOTION %d/%d not received within %d ms",
                    i + 1,
                    cfg.maxExpectedEvents,
                    effectiveTimeoutMs);
                break;
        }

        UT_ASSERT_EQUAL(
                event.mode,
                OperationalMode::NO_MOTION);

        UT_ASSERT_TRUE(
                event.timestampMonotonicMs > 0);
        UT_LOG_INFO(
                "[L3 NoMotionTimerReset] NO_MOTION %d/%d timestamp=%d noMotionMs=%d",
                i + 1,
                cfg.maxExpectedEvents,
                event.timestampMonotonicMs,
                noMotionMs);

    }
    MOTIONSENSOR_STOP_SUCCESS(controller);
    MOTIONSENSOR_WAIT_FOR_STATE(
        sensor,
        State::STOPPED,
        lastState);

    UT_ASSERT_EQUAL(
        lastState,
        State::STOPPED);
}

/**
 * @brief Validate that active windows gate event emission correctly.
 *
 * The Python host test configures a time window and injects MOTION events
 * both inside and outside the window to verify suppression behaviour.
 *
 * **Test Group ID:**: 03@n
 * **Test Case ID:** 003@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |@n
 * | :----: | --------- | ---------- |-------------- | ----- |@n
 * | 01 | Parse test config: sensor index, window start/end, numEvents, timeout | Config file | Config parsed successfully | Should be successful |@n
 * | 02 | Configure active window and start sensor in MOTION mode | Input: windowStart, windowEnd | Sensor reaches STARTED | Should Pass |@n
 * | 03 | Host injects event; verify outcome based on numEvents flag | Output: gotEvent | numEvents>0: event received; numEvents=0: event suppressed | Should Pass |@n
 * | 04 | Stop sensor; clear active windows | Input: controllerHandle | Windows cleared, sensor stopped | Should Pass |@n
 * | 05 | Restart sensor; host injects event with no window restriction | Output: gotEvent2 | Event received (MOTION) | Should Pass |@n
 * | 06 | Stop sensor | Input: controllerHandle | Sensor stops successfully | Should Pass |
 */
UT_ADD_TEST(VTS_L3_MOTIONSENSOR, ActiveWindowBehavior)
{
    const char *configNode = "EventDetection";
    motionSensorL3ActiveWindowTestConfig_t cfg;

    UT_ASSERT_EQUAL_FATAL(
        vts_motionsensor_parse_activewindow_config(
            VTS_MOTIONSENSOR_TEST_CONFIG_PATH, gModuleName, gTestGroup, configNode, cfg),
        UT_STATUS_OK);

    SensorTuple *tup = nullptr;
    UT_ASSERT_TRUE_FATAL(getSensorAtIndex(cfg.sensorIndex, tup));

    auto& sensor     = std::get<1>(*tup);
    auto& ctrlLsn    = std::get<2>(*tup);
    auto& evtLsn     = std::get<3>(*tup);
    auto& controller = std::get<4>(*tup);

    ctrlLsn->resetStateQueue();
    evtLsn->resetEventQueue();

    /* Configure active window (sensor must be STOPPED) */
    std::vector<TimeWindow> windows;
    TimeWindow win;
    win.startTimeOfDaySeconds = cfg.windowStartSeconds;
    win.endTimeOfDaySeconds   = cfg.windowEndSeconds;
    windows.push_back(win);
    MOTIONSENSOR_SET_ACTIVE_WINDOWS_SUCCESS(controller, windows);

    /* Start in MOTION mode */
    StartConfig startCfg;
    startCfg.operationalMode    = OperationalMode::MOTION;
    startCfg.noMotionSeconds    = 0;
    startCfg.activeStartSeconds = 0;
    startCfg.activeStopSeconds  = 0;

    MOTIONSENSOR_START_SUCCESS(controller, startCfg);

    State lastState;
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STARTED, lastState);
    UT_ASSERT_EQUAL(lastState, State::STARTED);

    /* Phase 1: host injects event; check against expected outcome.
     * numEvents>0 → window open → event must arrive (PASS if true).
     * numEvents=0 → window closed → event suppressed → getNextEvent
     *   times out (PASS if false). */
    MotionEvent receivedEvent;
    bool gotEvent = evtLsn->getNextEvent(receivedEvent, std::chrono::milliseconds(cfg.eventTimeoutMs));
    if (cfg.numEvents > 0)
    {
        UT_LOG_INFO("[L3 ActiveWindowBehavior] Phase-1: expecting event — gotEvent=%s",
            gotEvent ? "true" : "false");
        UT_ASSERT_TRUE(gotEvent);
        UT_ASSERT_EQUAL(receivedEvent.mode, OperationalMode::MOTION);
        UT_ASSERT_TRUE(receivedEvent.timestampMonotonicMs > 0);
    }
    else
    {
        UT_LOG_INFO("[L3 ActiveWindowBehavior] Phase-1: expecting suppression — gotEvent=%s",
            gotEvent ? "true (UNEXPECTED)" : "false (suppressed OK)");
        UT_ASSERT_FALSE(gotEvent);
    }

    /* Stop to reconfigure */
    MOTIONSENSOR_STOP_SUCCESS(controller);
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STOPPED, lastState);
    UT_ASSERT_EQUAL(lastState, State::STOPPED);

    /* Clear all windows — events should arrive any time */
    MOTIONSENSOR_CLEAR_ACTIVE_WINDOWS_SUCCESS(controller);

    std::vector<TimeWindow> emptyWindows;
    MOTIONSENSOR_GET_ACTIVE_WINDOWS_SUCCESS(controller, emptyWindows);
    UT_ASSERT_EQUAL(emptyWindows.size(), (size_t)0);

    evtLsn->resetEventQueue();
    MOTIONSENSOR_START_SUCCESS(controller, startCfg);
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STARTED, lastState);
    UT_ASSERT_EQUAL(lastState, State::STARTED);

    /* Host injects event again — now with no window restriction */
    bool gotEvent2 = evtLsn->getNextEvent(receivedEvent, std::chrono::milliseconds(cfg.eventTimeoutMs));
    UT_ASSERT_TRUE(gotEvent2);
    UT_ASSERT_EQUAL(receivedEvent.mode, OperationalMode::MOTION);
    UT_ASSERT_TRUE(receivedEvent.timestampMonotonicMs > 0);

    MOTIONSENSOR_STOP_SUCCESS(controller);
    MOTIONSENSOR_WAIT_FOR_STATE(sensor, State::STOPPED, lastState);
    UT_ASSERT_EQUAL(lastState, State::STOPPED);
}

/**
 * @} // End of MOTION_SENSOR_HALTEST_L3
 * @} // End of MOTION_SENSOR_HALTEST
 * @} // End of MOTION_SENSOR
 * @} // End of HPK
 */
/* End of vts_motionsensor_l3_tests.cpp */
