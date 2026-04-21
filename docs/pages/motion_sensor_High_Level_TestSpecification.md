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
| Verify sensor discovery | Validate sensor enumeration and retrieval flow | getMotionSensorIds(), getMotionSensor() |
| Verify lifecycle start | Validate STOPPED → STARTED transition | start(), getState() |
| Verify lifecycle stop | Validate STARTED → STOPPED transition | stop(), getState() |
| Verify invalid lifecycle transitions | start()/stop() in wrong states fail | start(), stop() |
| Verify state reporting | Validate state consistency across transitions | getState() |
| Verify configuration state restriction | Ensure config APIs only allowed in STOPPED | setSensitivity(), setActiveWindows(), setAutonomousDuringDeepSleep() |
 
---
 
## L3 Test Specification
 
| # | Test | Description | HAL APIs used |
|---|------|------------|---------------|
| 1 | Motion & No-Motion Event Validation | Start sensor in both MOTION and NO_MOTION modes and verify correct event delivery based on motion presence and inactivity timing | start(), registerEventListener(), onEvent() |
| 2 | Timing Behavior Validation | Validate activeStartSeconds delay, noMotionSeconds inactivity trigger, and activeStopSeconds auto-stop behavior | start(), getState(), onEvent() |
| 3 | Active Window Behavior | Validate event generation within configured windows, including suppression outside windows, midnight wrap-around, and union of multiple windows | setActiveWindows(), start(), onEvent() |
| 4 | Listener Lifecycle Validation | Verify events are delivered only to registered listeners and not after unregister; validate multiple listener behavior | registerEventListener(), unregisterEventListener(), onEvent() |
| 5 | Rapid Lifecycle Stability | Perform repeated start/stop cycles and verify no stale events, correct state transitions, and stable behavior | start(), stop(), getState(), onEvent() |
| 6 | Deep Sleep Autonomy | Validate motion detection behavior during simulated deep sleep when autonomy is enabled/disabled | setAutonomousDuringDeepSleep(), onEvent() |
| 7 | Multi-Sensor Independence | Validate independent operation, configuration, and event isolation across multiple sensors | getMotionSensor(), start(), onEvent() |
| 8 | Error Handling & Recovery | Simulate failure conditions and validate transition to ERROR state and subsequent behavior | getState(), onEvent() |
| 9 | Event Timing & Accuracy | Validate event delivery latency and correctness of event type with respect to configured mode and conditions | onEvent() |
 
---
