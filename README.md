# Test Suite For Sensor HAL

## Table of Contents

- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [Description](#description)
- [Reference Documents](#reference-documents)

## Acronyms, Terms and Abbreviations

- `HAL` - Hardware Abstraction Layer
- `L1` - Functional Tests
- `L2` - Module functional Testing
- `L3` - Module testing with External Stimulus is required to validate and control device
- `API` - Application Programming Interface
- `High-Level Test Specification` : This document provides a high-level view of test cases covered as part of `L2` and `L3` testing, focusing on major use cases, system behaviour, and overall test coverage.
- `Low-Level Test Specification` : This document provides technical details of how the tests are implemented, including specific test cases with inputs, expected outputs, pass/fail criteria, and implementation specifics for individual functionalities, modules, or APIs.

## Description

This repository contains the Test Suites(`L1` , `L2` and `L3`) for Sensor `HAL`.

## Reference Documents

| SNo | Document Name | Document Description | Document Link |
|---|---|---|---|
| 1 | `HAL` Specification Document – **Thermal Sensor** | Provides specific information on the APIs for which tests are written in the Thermal Sensor HAL module | https://github.com/rdkcentral/rdk-halif-aidl/blob/0.14.0/README.md |
| 2 | High Level Test Specification – **Thermal Sensor** | High Level Test Specification documentation for the Thermal Sensor HAL | docs/pages/thermal_sensor_High-Level_TestSpecification.md |
| 3 | `L3` Low Level Test Specification – **Thermal Sensor** | `L3` Low Level Test Specification for the Thermal Sensor HAL | docs/pages/thermal_sensor_L3_Low-Level_TestSpecification.md |
| 4 | `L1`, `L2` and `L3` Test Procedure – **Thermal Sensor** | Build instructions and `L1`, `L2`, `L3` test procedures for the Thermal Sensor HAL | docs/pages/vts_thermal_sensor_L1_L2_L3_TestProcedure.md |
| 5 | `HAL` Specification Document – **Motion Sensor** | Provides specific information on the APIs for which tests are written in the Motion Sensor HAL module | https://github.com/rdkcentral/rdk-halif-aidl/blob/0.14.0/README.md |
| 6 | High Level Test Specification – **Motion Sensor** | High Level Test Specification documentation for the Motion Sensor HAL | docs/pages/motion_sensor_High-Level_TestSpecification.md |
| 7 | `L3` Low Level Test Specification – **Motion Sensor** | `L3` Low Level Test Specification for the Motion Sensor HAL | docs/pages/motion_sensor_L3_Low-Level_TestSpecification.md |
| 8 | `L1`, `L2` and `L3` Test Procedure – **Motion Sensor** | Build instructions and `L1`, `L2`, `L3` test procedures for the Motion Sensor HAL | docs/pages/vts_motion_sensor_L1_L2_L3_TestProcedure.md |

