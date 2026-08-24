#!/usr/bin/env python3
#** *****************************************************************************
# *
# * If not stated otherwise in this file or this component's LICENSE file the
# * following copyright and licenses apply:
# *
# * Copyright 2026 RDK Management
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *
# http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *
#* ******************************************************************************

import os
import sys
import tarfile
import time
import threading
import yaml

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))
sys.path.append(dir_path)
sys.path.append(os.path.join(dir_path, "..", "raft"))

from raft.framework.plugins.ut_raft.configRead import ConfigRead
from raft.framework.plugins.ut_raft import utHelperClass
from raft.framework.core.logModule import logModule
from raft.framework.plugins.ut_raft.utSuiteNavigator import UTSuiteNavigatorClass
from raft.framework.plugins.ut_raft.utBaseUtils import utBaseUtils
from motionController.MotionSensorController import MotionSensorController

MOTIONSENSOR_TEST_MENU_CONFIGURATION_FILE = os.path.join(
    dir_path, "vts_motionsensor_testMenuConfiguration.yaml").replace("\\", "/")
MOTIONSENSOR_L3_TEST_CONFIG_FILE = os.path.join(
    os.path.join(dir_path, "../../test_configuration"), "vts_l3_MotionSensor.yaml").replace("\\", "/")

MOTIONSENSOR_L3_TEST_SUITE_NAME = "VTS_L3_MOTIONSENSOR"
DEFAULT_CONTROLLER_PORT = 8080


class motionSensorHelperClass(utHelperClass):
    """
    Base helper for MotionSensor L3 host-side tests.
    Handles artifact deployment, session setup, and control-plane trigger helpers.
    """

    def __init__(
        self,
        testName: str,
        qcId: str,
        log: logModule = None,
        moduleName: str = None,
    ):
        self.moduleName = "motionsensor" if moduleName is None else moduleName
        isUnifiedTest = False if moduleName is None else True
        self.interfaceName = "motionsensor"
        super().__init__(testName, qcId, log)

        self.log = log
        if log is None:
            self.log = logModule(self.__class__.__name__)
            self.log.setLevel(self.log.INFO)

        # Parse device configuration first (used by path selection below)
        self.deviceTestSetup = self.cpe.get("test")
        self.waitTime = self.cpe.get("wait_time")
        self.platform = self.cpe.get("platform")
        self.targetWorkspace = os.path.join(
            self.cpe.get("target_directory"), self.moduleName).replace("\\", "/")

        component_root = os.path.normpath(os.path.join(dir_path, "../..")).replace("\\", "/")
        unified_root = os.path.normpath(os.path.join(dir_path, "../../../..")).replace("\\", "/")

        if not isUnifiedTest:
            self.paths = {
                "testMenuConfigFile": MOTIONSENSOR_TEST_MENU_CONFIGURATION_FILE,
                "l3TestConfigFile": MOTIONSENSOR_L3_TEST_CONFIG_FILE,
                "moduleHFPFile": os.path.join(dir_path, self.deviceTestSetup.get("profile")).replace("\\", "/"),
            }
            artifactBaseDir = dir_path
            targetCfgBase = self.targetWorkspace
        else:
            self.paths = {
                "testMenuConfigFile": os.path.join(
                    unified_root, "host/tests/TestCases/vts_l3_test_setup.yaml"
                ).replace("\\", "/"),
                "l3TestConfigFile": os.path.join(
                    component_root, "test_configuration/vts_l3_MotionSensor.yaml"
                ).replace("\\", "/"),
                "moduleHFPFile": os.path.join(
                    component_root, "profiles/hfp-motionsensor.yaml"
                ).replace("\\", "/"),
            }
            artifactBaseDir = os.path.join(component_root, "host").replace("\\", "/")
            targetCfgBase = os.path.join(self.targetWorkspace, "bin").replace("\\", "/")

        for attr, path in self.paths.items():
            normalized_path = os.path.normpath(path).replace("\\", "/")
            setattr(self, attr, normalized_path)
            self.paths[attr] = normalized_path

        profileOnTarget = os.path.join(
            targetCfgBase, "profiles", os.path.basename(self.paths["moduleHFPFile"])
        ).replace("\\", "/")

        # Load test menu configuration
        controllerPort = self.deviceTestSetup.get("motionsensor_controller_port", DEFAULT_CONTROLLER_PORT)
        self.testMenuConfiguration = ConfigRead(
            self.paths["testMenuConfigFile"], self.moduleName)
        self.testMenuConfiguration.test.execute = (
            self.testMenuConfiguration.test.execute + f" -p {profileOnTarget}")
        self.l3TestConfigFolder = os.path.join(
            targetCfgBase, "test_configuration").replace("\\", "/")

        try:
            self.hal_session = self.dut.getConsoleSession("ssh_hal_test_0")
        except Exception:
            self.hal_session = self.dut.getConsoleSession("ssh_hal_test")
        self.hal_session_1 = self.dut.getConsoleSession("ssh_hal_test_1")

        # Initialize UT menu navigator and utilities
        self.utMenu = UTSuiteNavigatorClass(
            self.testMenuConfiguration, None, self.hal_session)
        self.utilities = utBaseUtils()

        # Copy required artifacts to target device
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)
        for artifact in self.testMenuConfiguration.test.artifacts:
            filesPath = os.path.normpath(os.path.join(artifactBaseDir, artifact)).replace("\\", "/")
            self.utilities.sftpCopy(
                self.hal_session, filesPath, self.targetWorkspace)
            if tarfile.is_tarfile(filesPath):
                target_tarfile = os.path.join(
                    self.targetWorkspace,
                    os.path.basename(artifact)).replace("\\", "/")
                self.utilities.untar(
                    self.hal_session, target_tarfile, self.targetWorkspace)
        self.targetWorkspace = targetCfgBase
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)
        self.utilities.change_directory(self.hal_session_1, self.targetWorkspace)
        time.sleep(2)

        self.motionSensorDevice = MotionSensorController(
            self.platform,
            self.hal_session_1,
            self.cpe.get("prompt"),
            controllerPort)

    def _readSensorIdsFromProfile(self) -> list:
        """
        Reads sensor IDs from the local HFP profile YAML (``moduleHFPFile``).
        Returns a list of integer IDs in the same order as the profile
        (index 0 = sensor_index 0 passed to ``getSensorAtIndex`` in C++).
        Falls back to ``[0]`` if the profile cannot be read.
        """
        try:
            with open(self.paths["moduleHFPFile"], 'r') as fh:
                profile = yaml.safe_load(fh)
            sensors = profile.get('sensor', {}).get('motion', [])
            ids = [int(s['id']) for s in sensors if 'id' in s]
            if not ids:
                self.log.warning("[_readSensorIdsFromProfile] No sensors found in profile, defaulting to id=0")
                return [0]
            self.log.info(f"[_readSensorIdsFromProfile] Found {len(ids)} sensor(s): {ids}")
            return ids
        except Exception as exc:
            self.log.warning(f"[_readSensorIdsFromProfile] Could not read '{self.paths['moduleHFPFile']}': {exc}")
            return [0]

    def runL3Test(self, testcase: str, triggerCallable=None, triggerDelay: float = 25.0) -> bool:
        """
        Uploads the L3 config YAML to the target, starts the test binary, fires
        the optional trigger callback in a background thread (after the binary is
        running), selects the test case in the UT menu, collects results, and
        returns the pass/fail bool.

        The trigger thread is started *after* utMenu.start() so the test binary
        is already running before any stimulus is sent matching the CEC
        reference pattern.

        Args:
            testcase (str): Test case name within VTS_L3_MOTIONSENSOR.
            triggerCallable (callable, optional): No-arg callable to run in a
                background thread ``triggerDelay`` seconds after utMenu.start().
            triggerDelay (float): Seconds to wait after utMenu.start() before
                invoking triggerCallable. Defaults to 25.0.

        Returns:
            bool: True if the test passed, False otherwise.
        """
        self.utilities.sftpCopy(
            self.hal_session, self.paths["l3TestConfigFile"], self.l3TestConfigFolder)
        self.utMenu.start()

        # Start trigger AFTER the test binary is running (CEC pattern).
        triggerThread = None
        if triggerCallable is not None:
            def _run():
                time.sleep(triggerDelay)
                triggerCallable()
            triggerThread = threading.Thread(target=_run, daemon=True)
            triggerThread.start()

        self.log.stepStart(
            f"Test Suit: {MOTIONSENSOR_L3_TEST_SUITE_NAME}  Test Case: {testcase}")
        output = self.utMenu.select(
            MOTIONSENSOR_L3_TEST_SUITE_NAME, testcase, None, self.waitTime, False)
        if triggerThread is not None:
            triggerThread.join()
        results = self.utMenu.collect_results(output, True)
        if results is None:
            results = False

        self.log.stepResult(
            results,
            f"Test Suit: {MOTIONSENSOR_L3_TEST_SUITE_NAME}  Test Case: {testcase}")
        return results
