# Thermal Sensor High Level Test Specification Document

## Table of Contents

- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [Introduction](#introduction)
- [Module Description](#module-description)
- [L2 Test Specification](#l2-test-specification)
- [L3 Test Specification](#l3-test-specification)

---

## Acronyms, Terms and Abbreviations

- `HAL`    - Hardware Abstraction Layer
- `API`    - Application Programming Interface
- `AIDL`   - Android Interface Definition Language
- `L2`     - Level 2 Testing
- `L3`     - Level 3 Testing
- `NA`     - Not Applicable
- `Caller` - Any user of the interface via the APIs
- `CB`     - Callback function
- `RTC`    - Real-Time Clock
- `SoC`    - System on Chip

---

## Introduction

This document provides an overview of the testing requirements for the Thermal Sensor module. It outlines the scope of testing, objectives, testing levels and approaches, specific test requirements, and expected deliverables.

The Thermal Sensor HAL enables monitoring of device thermal conditions and supports event-driven notifications based on vendor-defined thermal policies.

---

## Module Description

The Thermal Sensor HAL provides:

- Event-driven thermal state change notifications via registered listeners
- APIs to query current thermal state
- APIs to retrieve temperature telemetry from one or more sensors
- Policy-driven thermal state transitions based on YAML configuration

Thermal states include:
- `NORMAL`
- `CRITICAL_TEMPERATURE_EXCEEDED`
- `CRITICAL_TEMPERATURE_RECOVERED`
- `CRITICAL_SHUTDOWN_IMMINENT`

---

Note:
This feature does not define deterministic L2 system-level behaviors, as thermal events are generated asynchronously based on hardware conditions and vendor policy. Parameter and API validation are covered under L1. Callback behavior cannot be reliably triggered at L2, hence all functional validation is handled under L3 using controlled thermal simulation setups.


### L3 Test Specification
| # | Testing Feature | Description | Expected Scenario |
|---|----------------|------------|------------------|
| 1 | Verify thermal exceed and recovery lifecycle (NORMAL to EXCEEDED to RECOVERED) | Increase temperature beyond critical_temperature_exceeded_celsius, then reduce below critical_temperature_recovered_celsius, while verifying event timestamps | Device transitions NORMAL to CRITICAL_TEMPERATURE_EXCEEDED to CRITICAL_TEMPERATURE_RECOVERED to NORMAL, and timestampMonotonicMs increases monotonically across emitted events |
| 2 | Verify critical shutdown trigger | Increase temperature beyond entering_critical_shutdown_celsius from normal operating range while observing event timestamps | Device transitions NORMAL to CRITICAL_TEMPERATURE_EXCEEDED to CRITICAL_SHUTDOWN_IMMINENT, initiates controlled shutdown, and timestampMonotonicMs increases monotonically across state transition events |
| 3 | Verify multi-sensor thermal state aggregation, reporting, and telemetry | Simulate multiple sensors (SoC, Board) reporting different thermal states and temperatures, call getCurrentThermalState and getCurrentTemperatures, and validate sensor identifiers, vendor metadata, and telemetry fields | All sensors are reported with correct identifiers, temperature readings are accurate and consistent, and the API reports the highest severity thermal state among all sensors |
| 4 | Verify event callback delivery | Register listener and trigger thermal transitions | Listener receives asynchronous callbacks |
| 5 | Verify listener lifecycle and no redundant callbacks | Register listener, trigger valid thermal transitions, unregister listener, and maintain temperature within the same range | Callbacks are received only for valid state changes while registered, no redundant callbacks are triggered without state change, and no callbacks are received after unregister |

---

## Validation Constraints

### Thermal Threshold Ordering (Mandatory)
recovered < exceeded < shutdown

### Recovery Policy Requirements
- `CRITICAL_TEMPERATURE_RECOVERED` is emitted only after temperature falls below the recovered threshold
- Return to `NORMAL` must honor the configured recovery cooldown policy
- Recovery timing must follow the YAML policy strategy for the triggering sensor

### Timestamp Requirements
- Must be monotonic (not wall-clock time)
- Used strictly for event ordering and telemetry correlation

### Event Behavior
- Events must only be emitted on state changes
- Duplicate events are not allowed
- Event payload must contain a valid `state` and `timestampMonotonicMs`
- `temperatureReading` may be null when no specific sensor context is available

### Telemetry Requirements
- `getCurrentTemperatures()` may return an empty array when telemetry is unsupported
- Each `TemperatureReading` must preserve sensor identity and configured vendor metadata
- `vendorInfo` may be null or empty without constituting a failure

### Shutdown Behavior
- Shutdown is triggered autonomously by HAL
- Clients must not attempt to override or delay shutdown
- Only minimal cleanup is allowed

---

## Conclusion

This document ensures comprehensive validation of:

- Thermal state transitions
- Event-driven architecture
- Multi-sensor telemetry
- Vendor-defined thermal policy enforcement

The test coverage aligns with AIDL interface definitions and platform thermal safety requirements.
