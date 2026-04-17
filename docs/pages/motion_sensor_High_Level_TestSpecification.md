# MOTION SENSOR High Level Test Specification Document
 
## Table of Contents
 
- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [Introduction](#introduction)
- [Module Description](#module-description)
- [L2 Test Specification](#l2-test-specification)
- [L3 Test Specification](#l3-test-specification)
 
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
 
---
 
## Introduction
 
The Motion Sensor HAL provides a unified interface for motion detection sensors such as PIR and radar devices.
 
It enables:
- Discovery of available motion sensors
- Lifecycle management (start/stop)
- Configuration of sensitivity and timing behavior
- Event-driven motion/no-motion reporting
- Scheduling via active time windows
- Optional deep sleep autonomous operation
 
The HAL follows a **single-owner lifecycle model**, where one client controls the sensor lifecycle while multiple clients may subscribe to events.
 
---
 
## Module Description
 
The Motion Sensor HAL consists of:
 
### Key Interfaces
 
- **IMotionSensorManager**
  - getMotionSensorIds()
  - getMotionSensor()
 
- **IMotionSensor**
  - start(), stop()
  - getState()
  - getCapabilities()
  - set/getSensitivity()
  - set/getActiveWindows(), clearActiveWindows()
  - set/isAutonomousDuringDeepSleepEnabled()
  - registerEventListener(), unregisterEventListener()
 
- **IMotionSensorEventListener**
  - onEvent()
 
---
 
### Lifecycle Behavior
 
State transitions:
STOPPED → STARTING → STARTED
STARTED → STOPPING → STOPPED
ANY → ERROR
Constraints:
- start() valid only in STOPPED
- stop() valid in STARTED
- configuration APIs require STOPPED state
 
---
 
### Operational Behavior
 
- Modes:
  - MOTION → event on motion detection
  - NO_MOTION → event after inactivity duration
 
- Timing Controls:
  - activeStartSeconds → delayed activation
  - activeStopSeconds → automatic stop
  - noMotionSeconds → inactivity threshold
 
- Active Windows:
  - Define time-based event suppression
  - Support wrap-around (midnight crossing)
  - Union of windows defines active periods
 
- Capabilities:
  - Immutable per sensor instance
  - Derived from YAML configuration
 
---
 
## L2 Test Specification
 
| Test | Description | HAL APIs |
|------|------------|----------|
| Verify sensor discovery | Validate sensor IDs and retrieval of sensor instances | getMotionSensorIds(), getMotionSensor() |
| Verify capabilities | Validate immutable capabilities match expected values | getCapabilities() |
| Verify lifecycle start | Validate start() transitions state correctly | start(), getState() |
| Verify lifecycle stop | Validate stop() transitions state correctly | stop(), getState() |
| Verify invalid start state | Call start() when not STOPPED and expect failure | start() |
| Verify getState | Validate correct state reporting across lifecycle | getState() |
| Verify operational mode query | Validate getOperationalMode() in valid state | getOperationalMode() |
| Verify operational mode invalid state | Call getOperationalMode() in STOPPED and expect exception | getOperationalMode() |
| Verify sensitivity set/get | Validate sensitivity within valid range | setSensitivity(), getSensitivity() |
| Verify invalid sensitivity | Set sensitivity outside range and expect failure | setSensitivity() |
| Verify deep sleep autonomy | Validate enable/disable behavior | setAutonomousDuringDeepSleep(), isAutonomousDuringDeepSleepEnabled() |
| Verify unsupported deep sleep autonomy | Validate behavior when capability is not supported | setAutonomousDuringDeepSleep() |
| Verify listener lifecycle | Validate listener registration, event delivery, and unregister behavior | registerEventListener(), unregisterEventListener(), IMotionSensorEventListener.onEvent() |
| Validate multiple listeners | Ensure multiple listeners receive events | registerEventListener() |
| Validate duplicate listener registration | Register same listener twice and expect failure | registerEventListener() |
| Validate active windows configuration | Validate setting and retrieving time windows | setActiveWindows(), getActiveWindows() |
| Validate invalid time windows | Provide invalid time values and expect failure | setActiveWindows() |
| Validate clear active windows | Validate clearing windows results in 24-hour monitoring | clearActiveWindows(), getActiveWindows() |
| Validate configuration state restriction | Ensure configuration APIs fail when sensor is not STOPPED | setSensitivity(), setActiveWindows(), setAutonomousDuringDeepSleep() |
 
---
 
## L3 Test Specification
 
| Test | Description | HAL APIs used |
|------|------------|---------------|
| Motion detection event | Start sensor in MOTION mode and verify motion event delivery | start(), registerEventListener(), onEvent() |
| No-motion detection event | Start sensor in NO_MOTION mode and verify inactivity-based event | start(), onEvent() |
| Active start delay behavior | Validate delayed activation using activeStartSeconds | start(), onEvent() |
| Auto stop behavior | Validate sensor stops after activeStopSeconds duration | start(), getState() |
| No-motion timing validation | Validate noMotionSeconds triggers correct event timing | start(), onEvent() |
| Active window enforcement | Verify events are suppressed outside configured windows | setActiveWindows(), onEvent() |
| Midnight window handling | Validate windows that wrap across midnight | setActiveWindows(), onEvent() |
| Multiple window union behavior | Validate events occur within union of configured windows | setActiveWindows(), onEvent() |
| Rapid start/stop cycles | Validate stability across repeated lifecycle transitions | start(), stop(), getState() |
| Deep sleep autonomy behavior | Validate motion detection during simulated deep sleep | setAutonomousDuringDeepSleep(), onEvent() |
| Multi-sensor independence | Validate independent operation of multiple sensors | getMotionSensor(), start(), onEvent() |
| Error state handling | Simulate failure and validate transition to ERROR state | getState() |
| Event mode correctness | Ensure events match configured mode (MOTION vs NO_MOTION) | start(), onEvent() |
| Reaction time validation | Validate event delivery meets minimum reaction time | onEvent() |
| Timing vs window precedence | Validate sensor inactive when timing disables it even if within window | start(), onEvent() |
 
---