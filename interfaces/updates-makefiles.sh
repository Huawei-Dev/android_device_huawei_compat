#!/bin/bash

source $ANDROID_BUILD_TOP/system/tools/hidl/update-makefiles-helper.sh

do_makefiles_update \
  "vendor.huawei:device/huawei/compat/interfaces/huawei" \
  "android.hardware:device/huawei/compat/interfaces/android/hardware"
