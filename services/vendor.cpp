/*
 *
 * Copyright 2018 Samsung Electronics S.LSI Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "samsung.hardware.media.c2-service"

#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <minijail.h>

#include <util/C2InterfaceHelper.h>
#include <C2Component.h>
#include <C2Config.h>

#include <codec2/aidl/ComponentStore.h>
#include <codec2/aidl/ParamTypes.h>

#include "C2ExynosSupport.h"

using namespace ::aidl::android::hardware::media::c2;

// This is created by module "codec2.vendor.base.policy". This can be modified.
static constexpr char kBaseSeccompPolicyPath[] =
        "/vendor/etc/seccomp_policy/codec2.vendor.base.policy";

// Additional device-specific seccomp permissions can be added in this file.
static constexpr char kExtSeccompPolicyPath[] =
        "/vendor/etc/seccomp_policy/codec2.vendor.ext.policy";

int main(int /* argc */, char** /* argv */) {
    ALOGI("samsung.hardware.media.c2-service starting...");
    signal(SIGPIPE, SIG_IGN);
    android::SetUpMinijail(kBaseSeccompPolicyPath, kExtSeccompPolicyPath);

    ABinderProcess_setThreadPoolMaxThreadCount(8);
    ABinderProcess_startThreadPool();

    // Create IComponentStore service.
    std::shared_ptr<IComponentStore> store;

    ALOGD("Instantiating Codec2's Vendor IComponentStore service...");
    store = ::ndk::SharedRefBase::make<utils::ComponentStore>(std::static_pointer_cast<C2ComponentStore>(android::GetCodec2ExynosComponentStore()));
    
    if (store == nullptr) {
        ALOGE("Cannot create Codec2's Vendor IComponentStore service.");
    } else {
        const std::string serviceName =
            std::string(IComponentStore::descriptor) + "/default";
        binder_exception_t ex = AServiceManager_addService(
                store->asBinder().get(), serviceName.c_str());
        if (ex != EX_NONE) {
            ALOGE("Cannot register Codec2's Vendor IComponentStore service"
                          " with instance name %s", serviceName.c_str());
        } else {
            ALOGE("Codec2's Vendor IComponentStore service registered. "
                          "Instance name: %s", serviceName.c_str());
        }
    }

    ABinderProcess_joinThreadPool();

    return 0;
}
