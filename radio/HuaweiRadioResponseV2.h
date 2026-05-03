/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <android/hardware/radio/1.3/IRadioResponse.h>
#include <android/hardware/radio/1.4/IRadioResponse.h>
#include <vendor/huawei/hardware/radio/2.0/IHuaweiRadioResponseV2.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor::huawei::hardware::radio::implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

using ::android::hardware::radio::V1_4::IRadioResponse;
using ::android::hardware::radio::V1_0::RadioResponseInfo;
using ::android::hardware::radio::V1_0::IccIoResult;

using ::vendor::huawei::hardware::radio::V2_0::RspMsgPayload;
using ::vendor::huawei::hardware::radio::V2_0::RILPreferredPLMNSelector;
using ::vendor::huawei::hardware::radio::V2_0::IccIoResultEx;
using ::vendor::huawei::hardware::radio::V2_0::IHuaweiRadioResponseV2;


struct HuaweiRadioResponseV2 : public IHuaweiRadioResponseV2 {
    HuaweiRadioResponseV2(const sp<IRadioResponse>& radioResponse);
    
    // Methods from ::vendor::huawei::hardware::radio::V2_0::IHuaweiRadioResponseV2 follow.
    Return<void> RspMsg(const RadioResponseInfo& info, int32_t msgId, const RspMsgPayload& payload) override;
    Return<void> getPolListResponse(const RadioResponseInfo& info, const RILPreferredPLMNSelector& preferredplmnselector) override;
    Return<void> getSimMatchedFileFromRilCacheResponse(const RadioResponseInfo& info, const IccIoResultEx& iccIo) override;

private:
    sp<IRadioResponse> mRadioResponse;
};

}  // namespace vendor::huawei::hardware::radio::implementation
