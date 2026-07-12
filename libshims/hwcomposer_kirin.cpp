#define LOG_TAG "HwcKirinShim"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <dlfcn.h>

#include <cutils/properties.h>
#include <hardware/hardware.h>
#include <hardware/hwcomposer2.h>
#include <log/log.h>

#ifndef HWC_HARDWARE_MODULE_ID
#define HWC_HARDWARE_MODULE_ID "hwcomposer"
#endif

#ifndef HWC_MODULE_API_VERSION_0_1
#define HWC_MODULE_API_VERSION_0_1 HARDWARE_MODULE_API_VERSION(0, 1)
#endif

static constexpr hwc2_config_t NATIVE_CONFIG_ID = 0;

static constexpr int32_t HWC2_ERR_NONE = 0;
static constexpr int32_t HWC2_ERR_UNSUPPORTED = 8;

using PFN_GET_FUNCTION =
        hwc2_function_pointer_t (*)(hwc2_device_t*, int32_t);

using PFN_GET_DISPLAY_CONFIGS =
        int32_t (*)(hwc2_device_t*, hwc2_display_t, uint32_t*, hwc2_config_t*);

using PFN_GET_DISPLAY_ATTRIBUTE =
        int32_t (*)(hwc2_device_t*, hwc2_display_t, hwc2_config_t, int32_t, int32_t*);

using PFN_GET_ACTIVE_CONFIG =
        int32_t (*)(hwc2_device_t*, hwc2_display_t, hwc2_config_t*);

using PFN_SET_ACTIVE_CONFIG =
        int32_t (*)(hwc2_device_t*, hwc2_display_t, hwc2_config_t);

struct RealHwcState {
    void* dlopen_handle = nullptr;
    hwc2_device_t* real_device = nullptr;

    PFN_GET_FUNCTION real_getFunction = nullptr;
    int (*real_close)(hw_device_t*) = nullptr;

    PFN_GET_DISPLAY_CONFIGS real_getDisplayConfigs = nullptr;
    PFN_GET_DISPLAY_ATTRIBUTE real_getDisplayAttribute = nullptr;
    PFN_GET_ACTIVE_CONFIG real_getActiveConfig = nullptr;
    PFN_SET_ACTIVE_CONFIG real_setActiveConfig = nullptr;

    bool logged_configs = false;
};

static void get_real_hwc_path(char* out, size_t out_size) {
    char hardware[PROPERTY_VALUE_MAX] = {};

    property_get("ro.hardware", hardware, "");

#if defined(__LP64__)
    const char* libdir = "lib64";
#else
    const char* libdir = "lib";
#endif

    snprintf(out, out_size,
             "/vendor/%s/hw/hwcomposer.%s.so",
             libdir, hardware);
}

static RealHwcState g;

static bool primary_display(hwc2_display_t display) {
    return display == 0;
}

static hwc2_function_pointer_t get_real_function(int32_t descriptor) {
    if (!g.real_device || !g.real_getFunction) {
        return nullptr;
    }
    return g.real_getFunction(g.real_device, descriptor);
}

static hwc2_config_t map_config(hwc2_display_t display, hwc2_config_t config) {
    if (primary_display(display)) {
        return NATIVE_CONFIG_ID;
    }
    return config;
}

static int32_t shim_getDisplayConfigs(hwc2_device_t* device,
                                      hwc2_display_t display,
                                      uint32_t* outNumConfigs,
                                      hwc2_config_t* outConfigs) {
    if (!g.real_getDisplayConfigs) {
        g.real_getDisplayConfigs =
                reinterpret_cast<PFN_GET_DISPLAY_CONFIGS>(
                        get_real_function(HWC2_FUNCTION_GET_DISPLAY_CONFIGS));
    }

    if (!g.real_getDisplayConfigs) {
        return HWC2_ERR_UNSUPPORTED;
    }

    if (primary_display(display)) {
        if (!g.logged_configs) {
            ALOGI("getDisplayConfigs: expose only native config %u",
                  NATIVE_CONFIG_ID);
            g.logged_configs = true;
        }

        if (outNumConfigs) {
            if (outConfigs && *outNumConfigs >= 1) {
                outConfigs[0] = NATIVE_CONFIG_ID;
            }
            *outNumConfigs = 1;
        }

        return HWC2_ERR_NONE;
    }

    return g.real_getDisplayConfigs(g.real_device, display, outNumConfigs, outConfigs);
}

static int32_t shim_getDisplayAttribute(hwc2_device_t* device,
                                        hwc2_display_t display,
                                        hwc2_config_t config,
                                        int32_t attribute,
                                        int32_t* outValue) {
    if (!g.real_getDisplayAttribute) {
        g.real_getDisplayAttribute =
                reinterpret_cast<PFN_GET_DISPLAY_ATTRIBUTE>(
                        get_real_function(HWC2_FUNCTION_GET_DISPLAY_ATTRIBUTE));
    }

    if (!g.real_getDisplayAttribute) {
        return HWC2_ERR_UNSUPPORTED;
    }

    const hwc2_config_t mapped = map_config(display, config);
    return g.real_getDisplayAttribute(
            g.real_device, display, mapped, attribute, outValue);
}

static int32_t shim_getActiveConfig(hwc2_device_t* device,
                                    hwc2_display_t display,
                                    hwc2_config_t* outConfig) {
    if (!g.real_getActiveConfig) {
        g.real_getActiveConfig =
                reinterpret_cast<PFN_GET_ACTIVE_CONFIG>(
                        get_real_function(HWC2_FUNCTION_GET_ACTIVE_CONFIG));
    }

    if (!g.real_getActiveConfig) {
        return HWC2_ERR_UNSUPPORTED;
    }

    int32_t ret = g.real_getActiveConfig(g.real_device, display, outConfig);
    if (ret != HWC2_ERR_NONE) {
        return ret;
    }

    if (primary_display(display) && outConfig) {
        *outConfig = NATIVE_CONFIG_ID;
    }

    return HWC2_ERR_NONE;
}

static int32_t shim_setActiveConfig(hwc2_device_t* device,
                                    hwc2_display_t display,
                                    hwc2_config_t config) {
    if (!g.real_setActiveConfig) {
        g.real_setActiveConfig =
                reinterpret_cast<PFN_SET_ACTIVE_CONFIG>(
                        get_real_function(HWC2_FUNCTION_SET_ACTIVE_CONFIG));
    }

    if (!g.real_setActiveConfig) {
        return HWC2_ERR_UNSUPPORTED;
    }

    const hwc2_config_t mapped = map_config(display, config);

    if (primary_display(display) && config != mapped) {
        ALOGI("setActiveConfig: display=%" PRIu64 " requested=%u mapped=%u",
              static_cast<uint64_t>(display), config, mapped);
    }

    return g.real_setActiveConfig(g.real_device, display, mapped);
}

static hwc2_function_pointer_t shim_getFunction(hwc2_device_t* device,
                                                int32_t descriptor) {
    hwc2_function_pointer_t real = get_real_function(descriptor);

    switch (descriptor) {
        case HWC2_FUNCTION_GET_DISPLAY_CONFIGS:
            g.real_getDisplayConfigs =
                    reinterpret_cast<PFN_GET_DISPLAY_CONFIGS>(real);
            return reinterpret_cast<hwc2_function_pointer_t>(shim_getDisplayConfigs);

        case HWC2_FUNCTION_GET_DISPLAY_ATTRIBUTE:
            g.real_getDisplayAttribute =
                    reinterpret_cast<PFN_GET_DISPLAY_ATTRIBUTE>(real);
            return reinterpret_cast<hwc2_function_pointer_t>(shim_getDisplayAttribute);

        case HWC2_FUNCTION_GET_ACTIVE_CONFIG:
            g.real_getActiveConfig =
                    reinterpret_cast<PFN_GET_ACTIVE_CONFIG>(real);
            return reinterpret_cast<hwc2_function_pointer_t>(shim_getActiveConfig);

        case HWC2_FUNCTION_SET_ACTIVE_CONFIG:
            g.real_setActiveConfig =
                    reinterpret_cast<PFN_SET_ACTIVE_CONFIG>(real);
            return reinterpret_cast<hwc2_function_pointer_t>(shim_setActiveConfig);

        default:
            return real;
    }
}

static int shim_close(hw_device_t* dev) {
    int ret = 0;

    if (g.real_close) {
        ret = g.real_close(dev);
    }

    if (g.dlopen_handle) {
        dlclose(g.dlopen_handle);
    }

    memset(&g, 0, sizeof(g));
    return ret;
}

static int shim_open(const hw_module_t* module,
                     const char* id,
                     hw_device_t** device) {
    if (!device) {
        return -EINVAL;
    }

    char real_path[PATH_MAX] = {};
    get_real_hwc_path(real_path, sizeof(real_path));
    
    ALOGI("opening real HWC: %s", real_path);

    void* handle = dlopen(real_path, RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        ALOGE("dlopen real HWC failed: %s", dlerror());
        return -EINVAL;
    }

    auto* real_module = reinterpret_cast<hw_module_t*>(
            dlsym(handle, HAL_MODULE_INFO_SYM_AS_STR));

    if (!real_module || !real_module->methods || !real_module->methods->open) {
        ALOGE("real HAL_MODULE_INFO_SYM/open not found");
        dlclose(handle);
        return -EINVAL;
    }

    hw_device_t* real_common = nullptr;
    int ret = real_module->methods->open(real_module, id, &real_common);
    if (ret != 0 || !real_common) {
        ALOGE("real HWC open failed ret=%d", ret);
        dlclose(handle);
        return ret ? ret : -EINVAL;
    }

    auto* real_device = reinterpret_cast<hwc2_device_t*>(real_common);

    g.dlopen_handle = handle;
    g.real_device = real_device;
    g.real_getFunction = real_device->getFunction;
    g.real_close = real_device->common.close;

    real_device->getFunction = shim_getFunction;
    real_device->common.close = shim_close;
    real_device->common.module = const_cast<hw_module_t*>(module);

    auto real_set =
            reinterpret_cast<PFN_SET_ACTIVE_CONFIG>(
                    get_real_function(HWC2_FUNCTION_SET_ACTIVE_CONFIG));

    if (real_set) {
        int32_t r = real_set(real_device, 0, NATIVE_CONFIG_ID);
        ALOGI("initial setActiveConfig(display=0, config=%u) ret=%d",
              NATIVE_CONFIG_ID, r);
    }

    *device = real_common;

    ALOGI("shim open ok, native config forced to %u", NATIVE_CONFIG_ID);
    return 0;
}

static hw_module_methods_t shim_module_methods = {
    .open = shim_open,
};

extern "C" __attribute__((visibility("default")))
hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .module_api_version = HWC_MODULE_API_VERSION_0_1,
    .hal_api_version = HARDWARE_HAL_API_VERSION,
    .id = HWC_HARDWARE_MODULE_ID,
    .name = "Huawei Kirin HWC2 native-config shim",
    .author = "@Surdu_Petru",
    .methods = &shim_module_methods,
};
