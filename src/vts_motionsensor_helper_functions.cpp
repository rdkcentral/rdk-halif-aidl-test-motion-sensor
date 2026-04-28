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
 * @file vts_motionsensor_helper_functions.cpp
 */

#include "vts_motionsensor_helper_functions.h"
#include <binder/Status.h>

using android::binder::Status;

/* Exception code map table */
ut_control_keyStringPair_t statusMotionSensorException_mapTable[] = {
    { Status::EX_NONE,              "EX_NONE"              },
    { Status::EX_SECURITY,         "EX_SECURITY"          },
    { Status::EX_BAD_PARCELABLE,   "EX_BAD_PARCELABLE"    },
    { Status::EX_ILLEGAL_ARGUMENT, "EX_ILLEGAL_ARGUMENT"  },
    { Status::EX_NULL_POINTER,     "EX_NULL_POINTER"      },
    { Status::EX_ILLEGAL_STATE,    "EX_ILLEGAL_STATE"      },
    { Status::EX_NETWORK_MAIN_THREAD, "EX_NETWORK_MAIN_THREAD" },
    { Status::EX_UNSUPPORTED_OPERATION, "EX_UNSUPPORTED_OPERATION" },
    { Status::EX_SERVICE_SPECIFIC, "EX_SERVICE_SPECIFIC"  },
    { Status::EX_PARCELABLE,       "EX_PARCELABLE"        },
    { -1, nullptr }
};

/* Boolean map table */
ut_control_keyStringPair_t bool_mapTable_motionsensor[] = {
    { 0, "false" },
    { 1, "true"  },
    { -1, nullptr }
};
