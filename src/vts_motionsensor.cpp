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
    lock_guard<mutex> lock(mStateMutex);
    if (mStateChanges.empty())
        return false;
    stateChange = mStateChanges.front();
    mStateChanges.pop();
    return true;
}

bool MotionSensorControllerListener::hasActiveWindowEntered()
{
    lock_guard<mutex> lock(mWindowMutex);
    return mActiveWindowEntered;
}

bool MotionSensorControllerListener::hasActiveWindowExited()
{
    lock_guard<mutex> lock(mWindowMutex);
    return mActiveWindowExited;
}

void MotionSensorControllerListener::resetStateQueue()
{
    lock_guard<mutex> lock(mStateMutex);
    queue<pair<State, State>> empty;
    swap(mStateChanges, empty);
}

void MotionSensorControllerListener::resetActiveWindowFlags()
{
    lock_guard<mutex> lock(mWindowMutex);
    mActiveWindowEntered = false;
    mActiveWindowExited  = false;
}

/* ========================================================================== */
/*  MotionSensorEventListener Implementations                                 */
/* ========================================================================== */

android::binder::Status MotionSensorEventListener::onEvent(
    ::com::rdk::hal::sensor::motion::OperationalMode mode)
{
    UT_LOG_INFO("MotionSensorEventListener::onEvent called with mode:[%s]",
                ::com::rdk::hal::sensor::motion::toString(mode).c_str());
    {
        lock_guard<mutex> lock(mEventMutex);
        mEventQueue.push(mode);
    }
    return android::binder::Status::ok();
}

bool MotionSensorEventListener::getNextEvent(OperationalMode& outMode)
{
    lock_guard<mutex> lock(mEventMutex);
    if (mEventQueue.empty())
        return false;
    outMode = mEventQueue.front();
    mEventQueue.pop();
    return true;
}

bool MotionSensorEventListener::hasEvent()
{
    lock_guard<mutex> lock(mEventMutex);
    return !mEventQueue.empty();
}

void MotionSensorEventListener::resetEventQueue()
{
    lock_guard<mutex> lock(mEventMutex);
    queue<OperationalMode> empty;
    swap(mEventQueue, empty);
}

/** @} */ // End of MOTION_SENSOR_HALTEST_CLASS
/** @} */ // End of MOTION_SENSOR_HALTEST
/** @} */ // End of MOTION_SENSOR Module
/** @} */ // End of HPK
