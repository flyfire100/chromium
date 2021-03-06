// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_POLICY_CLOUD_USER_CLOUD_POLICY_MANAGER_FACTORY_H_
#define CHROME_BROWSER_POLICY_CLOUD_USER_CLOUD_POLICY_MANAGER_FACTORY_H_

#include <map>

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "base/memory/singleton.h"
#include "components/browser_context_keyed_service/browser_context_keyed_base_factory.h"

namespace base {
class SequencedTaskRunner;
}

namespace content {
class BrowserContext;
}

namespace policy {

class UserCloudPolicyManager;

// BrowserContextKeyedBaseFactory implementation for UserCloudPolicyManager
// instances that initialize per-profile cloud policy settings on the desktop
// platforms.
//
// UserCloudPolicyManager is handled different than other
// BrowserContextKeyedServices because it is a dependency of PrefService.
// Therefore, lifetime of instances is managed by Profile, Profile startup code
// invokes CreateForBrowserContext() explicitly, takes ownership, and the
// instance is only deleted after PrefService destruction.
//
// TODO(mnissler): Remove the special lifetime management in favor of
// PrefService directly depending on UserCloudPolicyManager once the former has
// been converted to a BrowserContextKeyedService.
// See also http://crbug.com/131843 and http://crbug.com/131844.
class UserCloudPolicyManagerFactory : public BrowserContextKeyedBaseFactory {
 public:
  // Returns an instance of the UserCloudPolicyManagerFactory singleton.
  static UserCloudPolicyManagerFactory* GetInstance();

  // Returns the UserCloudPolicyManager instance associated with |context|.
  static UserCloudPolicyManager* GetForBrowserContext(
      content::BrowserContext* context);

  // Creates an instance for |context|. Note that the caller is responsible for
  // managing the lifetime of the instance. Subsequent calls to
  // GetForBrowserContext() will return the created instance as long as it
  // lives. If RegisterTestingFactory() has been called, then calls to
  // this method will return null.
  //
  // If |force_immediate_load| is true, policy is loaded synchronously from
  // UserCloudPolicyStore at startup.
  //
  // |background_task_runner| is used for the cloud policy store.
  // |file_task_runner| is used for file operations. Currently this must be the
  // FILE BrowserThread.
  // |io_task_runner| is used for network IO. Currently this must be the IO
  // BrowserThread.
  static scoped_ptr<UserCloudPolicyManager> CreateForOriginalBrowserContext(
      content::BrowserContext* context,
      bool force_immediate_load,
      const scoped_refptr<base::SequencedTaskRunner>& background_task_runner,
      const scoped_refptr<base::SequencedTaskRunner>& file_task_runner,
      const scoped_refptr<base::SequencedTaskRunner>& io_task_runner);

  static UserCloudPolicyManager* RegisterForOffTheRecordBrowserContext(
      content::BrowserContext* original_context,
      content::BrowserContext* off_the_record_context);

  typedef UserCloudPolicyManager*
      (*TestingFactoryFunction)(content::BrowserContext* context);

  // Allows testing code to inject UserCloudPolicyManager objects for tests.
  // The factory function will be invoked for every Profile created. Because
  // this class does not free the UserCloudPolicyManager objects it manages,
  // it is up to the tests themselves to free the objects after the profile is
  // shut down.
  void RegisterTestingFactory(TestingFactoryFunction factory);
  void ClearTestingFactory();

 private:
  class ManagerWrapper;
  friend struct DefaultSingletonTraits<UserCloudPolicyManagerFactory>;

  UserCloudPolicyManagerFactory();
  virtual ~UserCloudPolicyManagerFactory();

  // See comments for the static versions above.
  UserCloudPolicyManager* GetManagerForBrowserContext(
      content::BrowserContext* context);

  scoped_ptr<UserCloudPolicyManager> CreateManagerForOriginalBrowserContext(
      content::BrowserContext* context,
      bool force_immediate_load,
      const scoped_refptr<base::SequencedTaskRunner>& background_task_runner,
      const scoped_refptr<base::SequencedTaskRunner>& file_task_runner,
      const scoped_refptr<base::SequencedTaskRunner>& io_task_runner);

  UserCloudPolicyManager* RegisterManagerForOffTheRecordBrowserContext(
      content::BrowserContext* original_context,
      content::BrowserContext* off_the_record_context);

  // BrowserContextKeyedBaseFactory:
  virtual void BrowserContextShutdown(
      content::BrowserContext* context) OVERRIDE;
  virtual void BrowserContextDestroyed(
      content::BrowserContext* context) OVERRIDE;
  virtual void SetEmptyTestingFactory(
      content::BrowserContext* context) OVERRIDE;
  virtual void CreateServiceNow(content::BrowserContext* context) OVERRIDE;
  virtual bool ServiceIsCreatedWithBrowserContext() const OVERRIDE;


  typedef std::map<content::BrowserContext*, ManagerWrapper*> ManagerWrapperMap;

  ManagerWrapperMap manager_wrappers_;
  TestingFactoryFunction testing_factory_;

  DISALLOW_COPY_AND_ASSIGN(UserCloudPolicyManagerFactory);
};

}  // namespace policy

#endif  // CHROME_BROWSER_POLICY_CLOUD_USER_CLOUD_POLICY_MANAGER_FACTORY_H_
