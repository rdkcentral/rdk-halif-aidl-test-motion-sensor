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

## L2 Test Specification

| Test | Description | HAL APIs |
|------|------------|----------|
| Register Event Listener | Register a valid listener and verify success | `registerEventListener()` |
| Duplicate Listener Registration | Register same listener twice and expect failure | `registerEventListener()` |
| Unregister Listener | Unregister existing listener and verify success | `unregisterEventListener()` |
| Unregister Invalid Listener | Attempt to unregister non-existent listener | `unregisterEventListener()` |
| Get Current Thermal State | Validate returned state is a valid enum | `getCurrentThermalState()` |
| Get Current Temperatures | Validate structure and correctness of returned data | `getCurrentTemperatures()` |
| Empty Temperature Support | Validate behavior when no sensors are reported | `getCurrentTemperatures()` |

---

## L3 Test Specification

### Prerequisites

- Thermal sensors are functional and validated at L1/L2  
- YAML policy configuration is correctly provisioned  
- Thermal injection or simulation mechanism is available  

---

### Feature List

| # | Testing Feature | Description | Expected Scenario |
|---|----------------|------------|------------------|
| 1 | Verify thermal state transition (NORMAL → EXCEEDED) | Increase temperature beyond `critical_temperature_exceeded_celsius` and observe event | Device enters `CRITICAL_TEMPERATURE_EXCEEDED` |
| 2 | Verify thermal recovery | Reduce temperature below `critical_temperature_recovered_celsius` | Device transitions to `CRITICAL_TEMPERATURE_RECOVERED` then `NORMAL` |
| 3 | Verify critical shutdown trigger | Increase temperature beyond `entering_critical_shutdown_celsius` | Device emits `CRITICAL_SHUTDOWN_IMMINENT` and initiates shutdown |
| 4 | Verify event callback delivery | Register listener and trigger thermal transitions | Listener receives asynchronous callbacks |
| 5 | Verify temperature telemetry accuracy | Call `getCurrentTemperatures()` and validate sensor data fields | Accurate and consistent sensor readings returned |
| 6 | Verify multi-sensor reporting | Simulate multiple sensors (SoC, Board) | All sensors reported with correct identifiers |
| 7 | Verify YAML policy compliance | Validate transitions follow configured thresholds | State transitions strictly follow YAML |
| 8 | Verify no event without state change | Maintain temperature within same range | No redundant callbacks triggered |
| 9 | Verify monotonic timestamp ordering | Trigger multiple state transitions | `timestampMonotonicMs` increases monotonically |
| 10 | Verify listener lifecycle | Register → unregister → trigger event | No callback after unregister |
| 11 | Verify null temperature handling | Trigger event without associated sensor | `temperatureReading` handled as null safely |
| 12 | Verify vendor metadata propagation | Validate `vendorCode` and `vendorInfo` fields | Correct vendor data returned |

---

## Validation Constraints

### Thermal Threshold Ordering (Mandatory)
recovered < exceeded < shutdown

### Timestamp Requirements
- Must be monotonic (not wall-clock time)  
- Used strictly for event ordering  

### Event Behavior
- Events must only be emitted on state changes  
- Duplicate events are not allowed  

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
