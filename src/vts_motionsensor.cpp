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
 * @defgroup MOTION_SENSOR_HALTEST_CLASS Motion Sensor HAL Tests Listener Implementations
 * @{
 * @parblock
 *
 * ### Listener class implementations for motion sensor tests :
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 */

/**
 * @file vts_motionsensor.cpp
 */

#include "vts_motionsensor.h"
#include "vts_motionsensor_helper_functions.h"

using std::lock_guard;
using std::unique_lock;

/* ========================================================================== */
/*  MotionSensorControllerListener Implementations                            */
/* ========================================================================== */

android::binder::Status MotionSensorControllerListener::onStateChanged(
    ::com::rdk::hal::sensor::motion::State oldState,
    ::com::rdk::hal::sensor::motion::State newState)
{
    UT_LOG_INFO("MotionSensorControllerListener::onStateChanged called with oldState:[%s] newState:[%s]",
                ::com::rdk::hal::sensor::motion::toString(oldState).c_str(),
                ::com::rdk::hal::sensor::motion::toString(newState).c_str());
    {
        lock_guard<mutex> lock(mStateMutex);
        mStateChanges.push({oldState, newState});
    }
    mStateCv.notify_all();
    return android::binder::Status::ok();
}

android::binder::Status MotionSensorControllerListener::onActiveWindowEntered()
{
    UT_LOG_INFO("MotionSensorControllerListener::onActiveWindowEntered called");
    {
        lock_guard<mutex> lock(mWindowMutex);
        mActiveWindowEntered = true;
    }
    return android::binder::Status::ok();
}

android::binder::Status MotionSensorControllerListener::onActiveWindowExited()
{
    UT_LOG_INFO("MotionSensorControllerListener::onActiveWindowExited called");
    {
        lock_guard<mutex> lock(mWindowMutex);
        mActiveWindowExited = true;
    }
    return android::binder::Status::ok();
}

bool MotionSensorControllerListener::getNextStateChange(pair<State, State>& stateChange)
{
    UT_LOG_INFO("MotionSensorControllerListener::getNextStateChange (no-wait) called");
    lock_guard<mutex> lock(mStateMutex);
    if (mStateChanges.empty())
    {
        UT_LOG_INFO("MotionSensorControllerListener::getNextStateChange - queue empty, returning false");
        return false;
    }
    stateChange = mStateChanges.front();
    mStateChanges.pop();
    UT_LOG_INFO("MotionSensorControllerListener::getNextStateChange - returned oldState:[%s] newState:[%s]",
                ::com::rdk::hal::sensor::motion::toString(stateChange.first).c_str(),
                ::com::rdk::hal::sensor::motion::toString(stateChange.second).c_str());
    return true;
}

bool MotionSensorControllerListener::getNextStateChange(pair<State, State>& stateChange, std::chrono::milliseconds timeout)
{
    UT_LOG_INFO("MotionSensorControllerListener::getNextStateChange (timeout=%lldms) called",
                static_cast<long long>(timeout.count()));
    unique_lock<mutex> lock(mStateMutex);
    bool got = mStateCv.wait_for(lock, timeout, [this]{ return !mStateChanges.empty(); });
    if (!got)
    {
        UT_LOG_INFO("MotionSensorControllerListener::getNextStateChange - timed out after %lldms",
                    static_cast<long long>(timeout.count()));
        return false;
    }
    stateChange = mStateChanges.front();
    mStateChanges.pop();
    UT_LOG_INFO("MotionSensorControllerListener::getNextStateChange - returned oldState:[%s] newState:[%s]",
                ::com::rdk::hal::sensor::motion::toString(stateChange.first).c_str(),
                ::com::rdk::hal::sensor::motion::toString(stateChange.second).c_str());
    return true;
}

bool MotionSensorControllerListener::hasActiveWindowEntered()
{
    lock_guard<mutex> lock(mWindowMutex);
    UT_LOG_INFO("MotionSensorControllerListener::hasActiveWindowEntered returning [%s]",
                mActiveWindowEntered ? "true" : "false");
    return mActiveWindowEntered;
}

bool MotionSensorControllerListener::hasActiveWindowExited()
{
    lock_guard<mutex> lock(mWindowMutex);
    UT_LOG_INFO("MotionSensorControllerListener::hasActiveWindowExited returning [%s]",
                mActiveWindowExited ? "true" : "false");
    return mActiveWindowExited;
}

void MotionSensorControllerListener::resetStateQueue()
{
    UT_LOG_INFO("MotionSensorControllerListener::resetStateQueue called");
    lock_guard<mutex> lock(mStateMutex);
    queue<pair<State, State>> empty;
    swap(mStateChanges, empty);
}

void MotionSensorControllerListener::resetActiveWindowFlags()
{
    UT_LOG_INFO("MotionSensorControllerListener::resetActiveWindowFlags called");
    lock_guard<mutex> lock(mWindowMutex);
    mActiveWindowEntered = false;
    mActiveWindowExited  = false;
}

/* ========================================================================== */
/*  MotionSensorEventListener Implementations                                 */
/* ========================================================================== */

android::binder::Status MotionSensorEventListener::onEvent(
    const ::com::rdk::hal::sensor::motion::MotionEvent& event)
{
    UT_LOG_INFO("MotionSensorEventListener::onEvent called with mode:[%s] ts:[%lld]",
                ::com::rdk::hal::sensor::motion::toString(event.mode).c_str(),
                static_cast<long long>(event.timestampMonotonicMs));
    {
        lock_guard<mutex> lock(mEventMutex);
        mEventQueue.push(event);
    }
    mEventCv.notify_all();
    return android::binder::Status::ok();
}

bool MotionSensorEventListener::getNextEvent(MotionEvent& outEvent)
{
    UT_LOG_INFO("MotionSensorEventListener::getNextEvent (no-wait) called");
    lock_guard<mutex> lock(mEventMutex);
    if (mEventQueue.empty())
    {
        UT_LOG_INFO("MotionSensorEventListener::getNextEvent - queue empty, returning false");
        return false;
    }
    outEvent = mEventQueue.front();
    mEventQueue.pop();
    UT_LOG_INFO("MotionSensorEventListener::getNextEvent - returned mode:[%s] ts:[%lld]",
                ::com::rdk::hal::sensor::motion::toString(outEvent.mode).c_str(),
                static_cast<long long>(outEvent.timestampMonotonicMs));
    return true;
}

bool MotionSensorEventListener::getNextEvent(MotionEvent& outEvent, std::chrono::milliseconds timeout)
{
    UT_LOG_INFO("MotionSensorEventListener::getNextEvent (timeout=%lldms) called",
                static_cast<long long>(timeout.count()));
    unique_lock<mutex> lock(mEventMutex);
    bool got = mEventCv.wait_for(lock, timeout, [this]{ return !mEventQueue.empty(); });
    if (!got)
    {
        UT_LOG_INFO("MotionSensorEventListener::getNextEvent - timed out after %lldms",
                    static_cast<long long>(timeout.count()));
        return false;
    }
    outEvent = mEventQueue.front();
    mEventQueue.pop();
    UT_LOG_INFO("MotionSensorEventListener::getNextEvent - returned mode:[%s] ts:[%lld]",
                ::com::rdk::hal::sensor::motion::toString(outEvent.mode).c_str(),
                static_cast<long long>(outEvent.timestampMonotonicMs));
    return true;
}

bool MotionSensorEventListener::getNextEvent(OperationalMode& outMode)
{
    MotionEvent event;
    bool got = getNextEvent(event);
    if (got)
    {
        outMode = event.mode;
    }
    return got;
}

bool MotionSensorEventListener::getNextEvent(OperationalMode& outMode, std::chrono::milliseconds timeout)
{
    MotionEvent event;
    bool got = getNextEvent(event, timeout);
    if (got)
    {
        outMode = event.mode;
    }
    return got;
}

bool MotionSensorEventListener::hasEvent()
{
    lock_guard<mutex> lock(mEventMutex);
    UT_LOG_INFO("MotionSensorEventListener::hasEvent returning [%s]",
                mEventQueue.empty() ? "false" : "true");
    return !mEventQueue.empty();
}

void MotionSensorEventListener::resetEventQueue()
{
    UT_LOG_INFO("MotionSensorEventListener::resetEventQueue called");
    lock_guard<mutex> lock(mEventMutex);
    queue<MotionEvent> empty;
    swap(mEventQueue, empty);
}

/** @} */ // End of MOTION_SENSOR_HALTEST_CLASS
/** @} */ // End of MOTION_SENSOR_HALTEST
/** @} */ // End of MOTION_SENSOR Module
/** @} */ // End of HPK
