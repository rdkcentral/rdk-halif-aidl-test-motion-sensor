# Motion Sensor L2 Low Level Test Specification

## Table of Contents

- [Overview](#overview)
- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [References](#references)
- [Level 2 Test Specification](#level-2-test-specification)

---

## Overview

This document describes the Level 2 (L2) Low-Level Test Specification for the Motion Sensor HAL module.

---

## Acronyms, Terms and Abbreviations

- `HAL`    - Hardware Abstraction Layer
- `VTS`    - Vendor Test Suite
- `L1`     - Level 1 Testing
- `L2`     - Level 2 Testing
- `L3`     - Level 3 Testing
- `DUT`    - Device Under Test
- `RAFT`   - Rapid Automation Framework for Testing
- `YAML`   - YAML Ain't Markup Language
- `AIDL`   - Android Interface Definition Language
- `PIR`    - Passive Infrared (sensor type)
- `HFP`    - Hardware Feature Profile

---

## References

- High Level Test Specification — [motion_sensor_High_Level_TestSpecification.md.md](motion_sensor_High_Level_TestSpecification.md.md)
- Motion Sensor AIDL Interfaces:
  - [IMotionSensorManager](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.15.0/sensor/current/com/rdk/hal/sensor/motion/IMotionSensorManager.aidl)
  - [IMotionSensor](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.15.0/sensor/current/com/rdk/hal/sensor/motion/IMotionSensor.aidl)
  - [IMotionSensorController](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.15.0/sensor/current/com/rdk/hal/sensor/motion/IMotionSensorController.aidl)
  - [IMotionSensorControllerListener](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.15.0/sensor/current/com/rdk/hal/sensor/motion/IMotionSensorControllerListener.aidl)
  - [IMotionSensorEventListener](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.15.0/sensor/current/com/rdk/hal/sensor/motion/IMotionSensorEventListener.aidl)

---

## Level 2 Test Specification

The following test cases validate the Motion Sensor HAL module at L2. All tests use the vComponent (hardware simulation) and do not require physical sensor hardware or external motion stimulus.

---

### Test 1

| Title | Details |
| -- | -- |
| Function Name | `StateTransitions` |
| Description | This test validates the full lifecycle state machine of the Motion Sensor HAL. It opens the sensor, starts it, verifies the STOPPED → STARTING → STARTED progression, stops it, and verifies the STARTED → STOPPING → STOPPED progression. State-change callbacks (`onStateChanged`) are verified at each transition for correct `oldState`/`newState` values. |
| Test Group | 02 |
| Test Case ID | 001 |
| Priority | High |

**Pre-Conditions:**
- Motion Sensor HAL service must be running with at least one sensor available.
- The vComponent must be started and configured via the HFP profile.
- The sensor must be in `State::STOPPED` before the test begins.

**Dependencies:**
- HFP profile: [profiles/hfp-motionsensor-vcomponent.yaml](../../profiles/hfp-motionsensor-vcomponent.yaml)

**User Interaction:**
None

**Test Procedure:**

| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|----------------|-------|
| 1 | Get a sensor ID from `getMotionSensorIds()` | — | Non-empty sensor ID list returned | |
| 2 | Get sensor handle via `getMotionSensor(id)` | Sensor ID from step 1 | Valid, non-null `IMotionSensor` handle returned | |
| 3 | Register a controller listener via `open(listener)` | Valid `IMotionSensorControllerListener` | Non-null `IMotionSensorController` returned | |
| 4 | Verify initial state via `getState()` | — | State is `STOPPED` | |
| 5 | Call `start(config)` with `mode=MOTION`, `noMotionSeconds=30` | Valid `StartConfig` | Start call succeeds without exception | |
| 6 | Check `onStateChanged` callback | — | `oldState=STOPPED`, `newState=STARTING` | |
| 7 | Wait for `getState()` to return `STARTING` | — | State is `STARTING` | |
| 8 | Check `onStateChanged` callback | — | `oldState=STARTING`, `newState=STARTED` | |
| 9 | Wait for `getState()` to return `STARTED` | — | State is `STARTED` | |
| 10 | Call `stop()` | — | Stop call succeeds without exception | |
| 11 | Check `onStateChanged` callback | — | `oldState=STARTED`, `newState=STOPPING` | |
| 12 | Wait for `getState()` to return `STOPPING` | — | State is `STOPPING` | |
| 13 | Check `onStateChanged` callback | — | `oldState=STOPPING`, `newState=STOPPED` | |
| 14 | Wait for `getState()` to return `STOPPED` | — | State is `STOPPED` | |
| 15 | Close the sensor via `close(controller)` | Controller from step 3 | Returns `true`; sensor released | |

---

### Test 2

| Title | Details |
| -- | -- |
| Function Name | `ConfigurationAPIsStateGuard` |
| Description | This test verifies that configuration APIs (`setSensitivity`, `setActiveWindows`, `setAutonomousDuringDeepSleep`) are accepted only when the sensor is in the `STOPPED` state, and rejected with `EX_ILLEGAL_STATE` when called in the `STARTED` state. |
| Test Group | 02 |
| Test Case ID | 002 |
| Priority | High |

**Pre-Conditions:**
- Motion Sensor HAL service must be running with at least one sensor available.
- The vComponent must be started and configured via the HFP profile.
- The sensor must be in `State::STOPPED` before the test begins.

**Dependencies:**
- HFP profile: [profiles/hfp-motionsensor-vcomponent.yaml](../../profiles/hfp-motionsensor-vcomponent.yaml)

**User Interaction:**
None

**Test Procedure:**

| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|----------------|-------|
| 1 | Open sensor and obtain controller handle | Valid listener | Controller returned | |
| 2 | Verify sensor is in `STOPPED` state | — | `getState()` returns `STOPPED` | |
| 3 | Call `setSensitivity(minSensitivity)` in STOPPED state | `caps.minSensitivity` | Returns `true` | Configuration allowed in STOPPED |
| 4 | Call `setActiveWindows([window])` in STOPPED state | `TimeWindow{start=32400, end=61200}` | Returns `true` | Configuration allowed in STOPPED |
| 5 | Call `setAutonomousDuringDeepSleep(true)` in STOPPED state | `enabled=true` | Returns `true` or `false` (based on capability) | |
| 6 | Call `clearActiveWindows()` in STOPPED state | — | Returns `true` | |
| 7 | Start the sensor with valid config | `mode=MOTION, noMotionSeconds=30` | Start succeeds; state becomes `STARTED` | |
| 8 | Call `setSensitivity(minSensitivity)` in STARTED state | `caps.minSensitivity` | Throws `EX_ILLEGAL_STATE` | Configuration rejected while running |
| 9 | Call `setActiveWindows([window])` in STARTED state | `TimeWindow{start=32400, end=61200}` | Throws `EX_ILLEGAL_STATE` | Configuration rejected while running |
| 10 | Call `setAutonomousDuringDeepSleep(true)` in STARTED state | `enabled=true` | Throws `EX_ILLEGAL_STATE` | Configuration rejected while running |
| 11 | Stop the sensor | — | Stop succeeds; state returns to `STOPPED` | |
| 12 | Close the sensor | Controller handle | Returns `true` | |

---

### Test 3

| Title | Details |
| -- | -- |
| Function Name | `ActiveWindowCallbacks` |
| Description | This test validates that `onActiveWindowEntered` and `onActiveWindowExited` callbacks are delivered correctly on the controller listener when the current time crosses into and out of a configured active window. The vComponent simulates time progression to trigger window transitions. |
| Test Group | 02 |
| Test Case ID | 003 |
| Priority | High |

**Pre-Conditions:**
- Motion Sensor HAL service must be running.
- The vComponent must support time-based active window simulation.
- The sensor must be in `State::STOPPED` before the test begins.

**Dependencies:**
- HFP profile: [profiles/hfp-motionsensor-vcomponent.yaml](../../profiles/hfp-motionsensor-vcomponent.yaml)

**User Interaction:**
None

**Test Procedure:**

| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|----------------|-------|
| 1 | Open sensor and obtain controller handle | Valid listener | Controller returned | |
| 2 | Configure a narrow active window covering a future time via `setActiveWindows()` | `TimeWindow` starting 5 s from now, ending 60 s from now | Returns `true` | |
| 3 | Verify `getActiveWindows()` returns the configured window | — | Returned array matches configured window | |
| 4 | Start the sensor with `mode=MOTION` | Valid `StartConfig` | Start succeeds | |
| 5 | Wait for `onActiveWindowEntered` callback | — | Callback fires within the expected time | |
| 6 | Host/vComponent advances time past the window end | — | `onActiveWindowExited` callback fires | |
| 7 | Stop the sensor | — | Stop succeeds | |
| 8 | Close the sensor | Controller handle | Returns `true` | |
