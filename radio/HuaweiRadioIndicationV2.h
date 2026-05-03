/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <android/hardware/radio/1.4/IRadioIndication.h>
#include <vendor/huawei/hardware/radio/2.0/IHuaweiRadioIndicationV2.h>
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

using ::android::hardware::radio::V1_4::IRadioIndication;
using ::android::hardware::radio::V1_0::RadioIndicationType;

using ::vendor::huawei::hardware::radio::V2_0::RILUnsolMsgPayload;
using ::vendor::huawei::hardware::radio::V2_0::IHuaweiRadioIndicationV2;

struct HuaweiRadioIndicationV2 : public IHuaweiRadioIndicationV2 {
public:
    HuaweiRadioIndicationV2(const sp<IRadioIndication>& radioIndication);

    // Methods from ::vendor::huawei::hardware::radio::V2_0::IHuaweiRadioIndicationV2 follow.
    Return<void> UnsolMsg(int32_t type, int32_t MsgId, const RILUnsolMsgPayload& payload) override;
 
private:
    sp<IRadioIndication> mRadioIndication;
};

}  // namespace vendor::huawei::hardware::radio::implementation
