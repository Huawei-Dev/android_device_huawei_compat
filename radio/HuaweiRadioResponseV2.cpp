/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "HuaweiRadioResponseV2.h"

namespace vendor::huawei::hardware::radio::implementation {

HuaweiRadioResponseV2::HuaweiRadioResponseV2(const sp<IRadioResponse>& radioResponse) {
    mRadioResponse = radioResponse;
}

// Methods from ::vendor::huawei::hardware::radio::V2_0::IHuaweiRadioResponseV2 follow.
Return<void> HuaweiRadioResponseV2::RspMsg(const RadioResponseInfo& info, int32_t msgId, const RspMsgPayload& payload) {

    /*Log.i(LOG_TAG, "rspmsg radioresponseinfo = $radioResponseInfo,msgtype=$msgType")
    Log.i(LOG_TAG, "serial " + radioResponseInfo)
    Log.i(LOG_TAG, "type=" + RespCode.getName(msgType))
    Log.i(LOG_TAG, "slotID=" + mSlotId)
    */
        
    return Void();
}

Return<void> HuaweiRadioResponseV2::getPolListResponse(const RadioResponseInfo& info, const RILPreferredPLMNSelector& preferredplmnselector) {
    return Void();
}

Return<void> HuaweiRadioResponseV2::getSimMatchedFileFromRilCacheResponse(const RadioResponseInfo& info, const IccIoResultEx& iccIo) {
    return Void();
}
    
}  // namespace vendor::huawei::hardware::radio::implementation
