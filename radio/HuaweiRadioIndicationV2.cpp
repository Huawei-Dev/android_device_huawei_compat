/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "HuaweiRadioIndicationV2.h"

namespace vendor::huawei::hardware::radio::implementation {

using ::android::hardware::radio::V1_0::GsmSignalStrength;
using ::android::hardware::radio::V1_4::SignalStrength;

HuaweiRadioIndicationV2::HuaweiRadioIndicationV2(const sp<IRadioIndication>& radioIndication) {
    mRadioIndication = radioIndication;
}

// Methods from ::vendor::huawei::hardware::radio::V2_0::IHuaweiRadioIndicationV2 follow.
Return<void> HuaweiRadioIndicationV2::UnsolMsg(int32_t type, int32_t MsgId, const RILUnsolMsgPayload& payload) {
    return Void();
}

}  // namespace vendor::huawei::hardware::radio::implementation
