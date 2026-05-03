#!/usr/bin/env python3
#** *****************************************************************************
# *
# * Copyright 2026 RDK Management
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# * http://www.apache.org/licenses/LICENSE-2.0
# *
#* ******************************************************************************

import os
import sys
import tarfile
import time
import threading

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))
sys.path.append(dir_path)
sys.path.append(os.path.join(dir_path, "raft"))

from raft.framework.plugins.ut_raft.configRead import ConfigRead
from raft.framework.plugins.ut_raft import utHelperClass
from raft.framework.core.logModule import logModule
from raft.framework.plugins.ut_raft.utSuiteNavigator import UTSuiteNavigatorClass
from raft.framework.plugins.ut_raft.utBaseUtils import utBaseUtils
from motionController.MotionSensorController import MotionSensorController

MOTIONSENSOR_TEST_MENU_CONFIGURATION_FILE = os.path.join(
    dir_path, "vts_motionsensor_testMenuConfiguration.yaml")
MOTIONSENSOR_L3_TEST_CONFIG_FILE = os.path.join(
    os.path.join(dir_path, "../../test_configuration"), "vts_l3_MotionSensor.yaml")

MOTIONSENSOR_L3_TEST_SUITE_NAME = "VTS_L3_MOTIONSENSOR"


class motionSensorHelperClass(utHelperClass):
    """
    Base helper for MotionSensor L3 host-side tests.
    Handles artifact deployment, session setup, and control-plane trigger helpers.
    """

    def __init__(self, testName: str, qcId: str, log: logModule = None):
        self.moduleName = "motionsensor"
        self.interfaceName = "motionsensor"
        super().__init__(testName, qcId, log)

        self.log = log
        if log is None:
            self.log = logModule(self.__class__.__name__)
            self.log.setLevel(self.log.INFO)

        # Parse device configuration
        self.deviceTestSetup = self.cpe.get("test")
        self.targetWorkspace = os.path.join(
            self.cpe.get("target_directory"), self.moduleName)
        self.moduleHFPFile = os.path.join(
            dir_path, self.deviceTestSetup.get("profile"))
        profileOnTarget = os.path.join(
            self.targetWorkspace, "profiles",
            os.path.basename(self.moduleHFPFile)).replace("\\", "/")
        self.waitTime = self.cpe.get("wait_time")
        self.platform = self.cpe.get("platform")

        # Load test menu configuration
        controllerPort = self.deviceTestSetup.get("controller_port", 8080)
        self.l3TestConfigFile = MOTIONSENSOR_L3_TEST_CONFIG_FILE
        self.testMenuConfiguration = ConfigRead(
            MOTIONSENSOR_TEST_MENU_CONFIGURATION_FILE, self.moduleName)
        self.testMenuConfiguration.test.execute = (
            self.testMenuConfiguration.test.execute + f" -p {profileOnTarget}")
        self.l3TestConfigFolder = os.path.join(
            self.targetWorkspace, "test_configuration").replace("\\", "/")

        # Establish HAL session (test binary) and a second session for vcomponent
        self.hal_session = self.dut.getConsoleSession("ssh_hal_test")
        self.hal_session_1 = self.dut.getConsoleSession("ssh_hal_test_1")

        # Initialize UT menu navigator and utilities
        self.utMenu = UTSuiteNavigatorClass(
            self.testMenuConfiguration, None, self.hal_session)
        self.utilities = utBaseUtils()

        # Copy required artifacts to target device
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)
        for artifact in self.testMenuConfiguration.test.artifacts:
            filesPath = os.path.join(dir_path, artifact)
            self.utilities.sftpCopy(
                self.hal_session, filesPath, self.targetWorkspace)
            if tarfile.is_tarfile(filesPath):
                target_tarfile = os.path.join(
                    self.targetWorkspace,
                    os.path.basename(artifact)).replace("\\", "/")
                self.utilities.untar(
                    self.hal_session, target_tarfile, self.targetWorkspace)

        #TODO: remove once the vendor layer start handled by vDevice service
        self.utilities.change_directory(
            self.hal_session_1, self.targetWorkspace)
        self.hal_session_1.write(f"pkill -f {self.cpe.get('motionsensor_process_name')} 2>/dev/null; sleep 1")
        time.sleep(1)
        self.hal_session_1.write(f"./{self.cpe.get('motionsensor_process_name')} --port {controllerPort} &")
        time.sleep(2)

        self.motionSensorDevice = MotionSensorController(
            self.platform,
            self.hal_session_1,
            self.cpe.get("prompt"),
            controllerPort)

    def triggerEventAfterDelay(self, triggerFunc, delay: float = 2.0, **kwargs):
        """
        Runs triggerFunc(**kwargs) in a background thread after `delay` seconds.
        Returns the Thread object so the caller can .join() it.

        Args:
            triggerFunc: Callable to invoke (e.g. self.motionSensorDevice.triggerMotionEvent).
            delay (float): Seconds to wait before invoking triggerFunc.
            **kwargs: Arguments forwarded to triggerFunc.

        Returns:
            threading.Thread: The started thread.
        """
        def _run():
            time.sleep(delay)
            triggerFunc(**kwargs)

        t = threading.Thread(target=_run, daemon=True)
        t.start()
        return t

    def runL3Test(self, testcase: str, triggerThread=None) -> bool:
        """
        Uploads the L3 config YAML to the target, selects the test case in the
        UT menu, collects results, and returns the pass/fail bool.

        Args:
            testcase (str): Test case name within VTS_L3_MOTIONSENSOR.
            triggerThread (threading.Thread, optional): Thread to join after test.

        Returns:
            bool: True if the test passed, False otherwise.
        """
        self.utilities.sftpCopy(
            self.hal_session, self.l3TestConfigFile, self.l3TestConfigFolder)
        self.utMenu.start()
        self.log.stepStart(
            f"Test Suite: {MOTIONSENSOR_L3_TEST_SUITE_NAME}  Test Case: {testcase}")
        output = self.utMenu.select(
            MOTIONSENSOR_L3_TEST_SUITE_NAME, testcase, None, self.waitTime, False)
        if triggerThread is not None:
            triggerThread.join()
        results = self.utMenu.collect_results(output, True)
        if results is None:
            results = False
        self.log.stepResult(
            results,
            f"Test Suite: {MOTIONSENSOR_L3_TEST_SUITE_NAME}  Test Case: {testcase}")
        return results
