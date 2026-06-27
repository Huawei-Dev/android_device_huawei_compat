#define LOG_TAG "audio@6.0-impl-hisi"

#include <android/hardware/audio/6.0/IDevicesFactory.h>
#include <log/log.h>
#include <utils/Errors.h>
#include <utils/StrongPointer.h>

using android::OK;
using android::sp;
using android::status_t;
using android::hardware::audio::V6_0::IDevicesFactory;

extern "C" IDevicesFactory* HIDL_FETCH_IDevicesFactory(const char* name);

extern "C" int registerHisiAudioFactory() {
    sp<IDevicesFactory> factory = HIDL_FETCH_IDevicesFactory("default");

    if (factory == nullptr) {
        ALOGE("HIDL_FETCH_IDevicesFactory(default) returned null");
        return 1;
    }

    status_t status = factory->registerAsService("default");
    if (status != OK) {
        ALOGE("Could not register android.hardware.audio@6.0::IDevicesFactory/default, status=%d",
              status);
        return 1;
    }

    ALOGI("Registered android.hardware.audio@6.0::IDevicesFactory/default from hisi impl");
    return 0;
}
