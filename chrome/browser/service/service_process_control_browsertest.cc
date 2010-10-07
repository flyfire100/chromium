// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/test/test_timeouts.h"
#include "chrome/browser/browser.h"
#include "chrome/browser/service/service_process_control.h"
#include "chrome/browser/service/service_process_control_manager.h"
#include "chrome/common/service_process_util.h"
#include "chrome/test/in_process_browser_test.h"
#include "chrome/test/ui_test_utils.h"

class ServiceProcessControlBrowserTest
    : public InProcessBrowserTest,
      public ServiceProcessControl::MessageHandler {
 protected:
  void LaunchServiceProcessControl() {
    ServiceProcessControl* process =
        ServiceProcessControlManager::instance()->GetProcessControl(
            browser()->profile());
    process_ = process;

    // Launch the process asynchronously.
    process->Launch(
        NewRunnableMethod(
            this,
            &ServiceProcessControlBrowserTest::ProcessControlLaunched));

    // Then run the message loop to keep things running.
    ui_test_utils::RunMessageLoop();
  }

  void SayHelloAndWait() {
    // Send a hello message to the service process and wait for a reply.
    process()->SendHello();
    ui_test_utils::RunMessageLoop();
  }

  void DisconnectAndWaitForShutdown() {
    // This will delete all instances of ServiceProcessControl and close the IPC
    // connections.
    ServiceProcessControlManager::instance()->Shutdown();
    process_ = NULL;
    MessageLoop::current()->PostDelayedTask(
        FROM_HERE,
        NewRunnableMethod(this,
                          &ServiceProcessControlBrowserTest::DoDetectShutdown),
        TestTimeouts::wait_for_terminate_timeout_ms());
    ui_test_utils::RunMessageLoop();
  }


  void DoDetectShutdown() {
    EXPECT_FALSE(CheckServiceProcessRunning());
    // Quit the current message loop.
    MessageLoop::current()->PostTask(FROM_HERE,
        new MessageLoop::QuitTask());
  }

  void ProcessControlLaunched() {
    process()->SetMessageHandler(this);
    // Quit the current message.
    MessageLoop::current()->PostTask(FROM_HERE,
        new MessageLoop::QuitTask());
  }

  // ServiceProcessControl::MessageHandler implementations.
  virtual void OnGoodDay() {
    MessageLoop::current()->PostTask(FROM_HERE,
        new MessageLoop::QuitTask());
  }

  ServiceProcessControl* process() { return process_; }

 private:
  ServiceProcessControl* process_;
};

#if defined(OS_WIN)
// They way that the IPC is implemented only works on windows. This has to
// change when we implement a different scheme for IPC.
IN_PROC_BROWSER_TEST_F(ServiceProcessControlBrowserTest, LaunchAndIPC) {
  LaunchServiceProcessControl();

  // Make sure we are connected to the service process.
  EXPECT_TRUE(process()->is_connected());
  SayHelloAndWait();

  // And then shutdown the service process.
  EXPECT_TRUE(process()->Shutdown());
}

// This tests the case when a service process is launched when browser
// starts but we try to launch it again in the remoting setup dialog.
IN_PROC_BROWSER_TEST_F(ServiceProcessControlBrowserTest, LaunchTwice) {
  // Launch the service process the first time.
  LaunchServiceProcessControl();

  // Make sure we are connected to the service process.
  EXPECT_TRUE(process()->is_connected());
  SayHelloAndWait();

  // Launch the service process again.
  LaunchServiceProcessControl();
  EXPECT_TRUE(process()->is_connected());
  SayHelloAndWait();

  // And then shutdown the service process.
  EXPECT_TRUE(process()->Shutdown());
}

// Tests whether disconnecting from the service IPC causes the service process
// to die.
IN_PROC_BROWSER_TEST_F(ServiceProcessControlBrowserTest, DieOnDisconnect) {
  // Launch the service process the first time.
  LaunchServiceProcessControl();
  // Make sure we are connected to the service process.
  EXPECT_TRUE(process()->is_connected());
  DisconnectAndWaitForShutdown();
}

#endif

DISABLE_RUNNABLE_METHOD_REFCOUNT(ServiceProcessControlBrowserTest);
