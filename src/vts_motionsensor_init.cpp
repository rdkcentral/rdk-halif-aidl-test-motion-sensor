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
 * @defgroup MOTION_SENSOR_HALTEST_INIT Motion Sensor HAL Tests Init Implementation
 * @{
 * @parblock
 *
 * ### Init/destroy implementation for motion sensor tests :
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 */

/**
 * @file vts_motionsensor_init.cpp
 */

#include <ut.h>
#include "vts_motionsensor_init.h"
#include "vts_motionsensor.h"
#include "vts_motionsensor_parse_hfp.h"

#include <ut_log.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

using android::sp;
using android::String16;
using android::defaultServiceManager;
using android::binder::Status;
using android::ProcessState;
using android::IPCThreadState;

using namespace com::rdk::hal::sensor::motion;

/* Global handle to IMotionSensorManager */
sp<IMotionSensorManager> gMOTIONSENSORMGRHandle = nullptr;

static UT_status_t vts_motionsensor_getService()
{
    gMOTIONSENSORMGRHandle = android::interface_cast<IMotionSensorManager>(
            defaultServiceManager()->getService(String16(IMotionSensorManager::serviceName().c_str())));

    if (gMOTIONSENSORMGRHandle == nullptr)
    {
        UT_LOG_ERROR("MOTIONSENSOR Service not available");
        return UT_STATUS_FAILURE;
    }
    return UT_STATUS_OK;
}

UT_status_t VTS_MOTIONSENSOR_init(void)
{
    UT_LOG_INFO("VTS_MOTIONSENSOR_init: Connecting to IMotionSensorManager service");

    if(vts_motionsensor_getService() != UT_STATUS_OK)
    {
        UT_LOG_ERROR("Failed to get MOTIONSENSOR Service");
        return UT_STATUS_FAILURE;
    }

    UT_LOG_INFO("VTS_MOTIONSENSOR_init: Successfully connected to IMotionSensorManager");

    /* Parse the HFP profile for device capabilities */
    vts_motionsensor_parse_hfp();

    UT_LOG_INFO("VTS_MOTIONSENSOR_init: Initialization complete");
    return UT_STATUS_OK;
}

void VTS_MOTIONSENSOR_destroy(void)
{
    UT_LOG_INFO("VTS_MOTIONSENSOR_destroy: Releasing IMotionSensorManager handle");

    gMOTIONSENSORMGRHandle = nullptr;

    UT_LOG_INFO("VTS_MOTIONSENSOR_destroy: Cleanup complete");
}

/** @} */ // End of MOTION_SENSOR_HALTEST_INIT
/** @} */ // End of MOTION_SENSOR_HALTEST
/** @} */ // End of MOTION_SENSOR Module
/** @} */ // End of HPK
