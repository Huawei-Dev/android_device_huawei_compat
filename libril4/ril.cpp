#include <dlfcn.h>

#define RIL_SHLIB

#include <log/log.h>
#include <telephony/ril.h>
#include <telephony/ril_mnc.h>
#include <inttypes.h>

#include "ril-HW.h"
#include "ril.h"

/*
#pragma once
#include <android/hardware/radio/1.4/IRadioResponse.h>
#include <android/hardware/radio/1.4/IRadio.h>
#include <ril_service.h>
#include <ril_service_1_4.h>
*/

typedef struct {
    int requestNumber;
    void (*dispatchFunction)(void* p, void* pRI);
    int (*responseFunction)(void* p, void* response, size_t responselen);
} CommandInfo;

typedef struct RequestInfo {
    int32_t token;
    CommandInfo* pCI;
    struct RequestInfo* p_next;
    char cancelled;
    char local;
} RequestInfo;


static void dump_response(unsigned char *hash_buf)
{

   RLOGD("{0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, ",
      *(hash_buf + 0), *(hash_buf + 1), *(hash_buf + 2), *(hash_buf + 3),
      *(hash_buf + 4), *(hash_buf + 5), *(hash_buf + 6), *(hash_buf + 7));
   RLOGD("0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, ",
      *(hash_buf + 8), *(hash_buf + 9), *(hash_buf + 10), *(hash_buf + 11),
      *(hash_buf + 12), *(hash_buf + 13), *(hash_buf + 14),
      *(hash_buf + 15));
   RLOGD("0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X,  ",
      *(hash_buf + 16), *(hash_buf + 17), *(hash_buf + 18),
      *(hash_buf + 19), *(hash_buf + 20), *(hash_buf + 21),
      *(hash_buf + 22), *(hash_buf + 23));
   RLOGD("0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X} ",
      *(hash_buf + 24), *(hash_buf + 25), *(hash_buf + 26),
      *(hash_buf + 27), *(hash_buf + 28), *(hash_buf + 29),
      *(hash_buf + 30), *(hash_buf + 31));
}




/*************************** Huawei V1.0 response size = 56 *********************
  
  *(undefined4 *)(param_3 + 8) = 0x7fffffff;
  *(undefined4 *)(param_3 + 0xc) = *(undefined4 *)((long)param_1 + 0x1c);
  *(undefined4 *)(param_3 + 0x10) = *(undefined4 *)((long)param_1 + 0x20);

  *(undefined4 *)(param_3 + 0x14) = *(undefined4 *)((long)param_1 + 0x24);
  *(undefined4 *)(param_3 + 0x18) = *(undefined4 *)((long)param_1 + 0x28);
  *(undefined4 *)(param_3 + 0x1c) = *(undefined4 *)((long)param_1 + 0x2c);

  // LTE
  *(undefined4 *)(param_3 + 0x20) = *(undefined4 *)((long)param_1 + 0x30);
  *(undefined4 *)(param_3 + 0x24) = *(undefined4 *)((long)param_1 + 0x34);
  *(undefined4 *)(param_3 + 0x28) = *(undefined4 *)((long)param_1 + 0x38);
  *(undefined4 *)(param_3 + 0x2c) = *(undefined4 *)((long)param_1 + 0x3c);
  *(undefined4 *)(param_3 + 0x30) = *(undefined4 *)((long)param_1 + 0x40);
  *(undefined4 *)(param_3 + 0x34) = *(undefined4 *)((long)param_1 + 0x44);

  
  *(undefined4 *)(param_3 + 0x38) = *(undefined4 *)((long)param_1 + 0x50);
*/

typedef struct {
    RIL_GSM_SignalStrength_v12  GSM_SignalStrength;
    RIL_CDMA_SignalStrength     CDMA_SignalStrength;
    RIL_EVDO_SignalStrength     EVDO_SignalStrength;
    RIL_LTE_SignalStrength_v8   LTE_SignalStrength;
    RIL_TD_SCDMA_SignalStrength TD_SCDMA_SignalStrength;
} RIL_SignalStrength_Huawei;

int convertRilSignalStrengthToHal_1_0(const void* response, size_t responseLen)
{
 int iVar1;
 SignalStrength signalStrength = {};
 
   RLOGD("%s: convertRilHwSignalStrengthToHal V 1.0 : responseLen %d - sizeof %d ", __func__, responseLen, sizeof(signalStrength));
   
   // GSM
   signalStrength.gsm.signalStrength = *(int32_t *)((long)response + 0x00);
   signalStrength.gsm.bitErrorRate = *(int32_t *)((long)response + 0x04);
   signalStrength.gsm.timingAdvance = 0x7fffffff;

   // CDMA
   signalStrength.cdma.dbm  = *(int32_t *)((long)response + 0x1c);
   signalStrength.cdma.ecio = *(int32_t *)((long)response + 0x20);

   // EVO
   signalStrength.evdo.dbm = *(int32_t *)((long)response + 0x24);
   signalStrength.evdo.ecio = *(int32_t *)((long)response + 0x28);
   signalStrength.evdo.signalNoiseRatio = *(int32_t *)((long)response + 0x2c);  

   // LTE
   signalStrength.lte.signalStrength= *(int32_t *)((long)response + 0x30);
   signalStrength.lte.rssnr = *(int32_t *)((long)response + 0x34);
   signalStrength.lte.rsrp = *(int32_t *)((long)response + 0x38);
   signalStrength.lte.rsrq = *(int32_t *)((long)response + 0x3c);
   signalStrength.lte.cqi = *(int32_t *)((long)response + 0x40);
   signalStrength.lte.timingAdvance = *(int32_t *)((long)response + 0x44);

   // TDSCMA
   signalStrength.tdScdma.rscp = *(int32_t *)((long)response + 0x50);
   
   return sizeof(signalStrength);
}



/*************************** Huawei V1.4 response *********************

  // GSM
  *(int32_t *)(signalStrength + 0x0) = *(int32_t *)((long)response + 0x0);
  *(int32_t *)(signalStrength + 0x4) = *(int32_t *)((long)response + 0x4);
  *(int32_t *)(signalStrength + 0x8) = *(int32_t *)((long)response + 0x8);

  // WCDMA
  *(undefined4 *)(param_3 + 0x44) = *(undefined4 *)((long)param_1 + 0xc);
  *(undefined4 *)(param_3 + 0x48) = *(undefined4 *)((long)param_1 + 0x10);
  *(undefined4 *)(param_3 + 0x4c) = *(undefined4 *)((long)param_1 + 0x14);
  *(undefined4 *)(param_3 + 0x50) = *(undefined4 *)((long)param_1 + 0x18);

  // CDMA
  *(undefined4 *)(param_3 + 0x0c) = *(undefined4 *)((long)param_1 + 0x1c);
  *(undefined4 *)(param_3 + 0x10) = *(undefined4 *)((long)param_1 + 0x20);

  // Evo
  *(undefined4 *)(param_3 + 0x14) = *(undefined4 *)((long)param_1 + 0x24);
  *(undefined4 *)(param_3 + 0x18) = *(undefined4 *)((long)param_1 + 0x28);
  *(undefined4 *)(param_3 + 0x1c) = *(undefined4 *)((long)param_1 + 0x2c);
  
  // LTE
  *(undefined4 *)(param_3 + 0x20) = *(undefined4 *)((long)param_1 + 0x30);
  *(undefined4 *)(param_3 + 0x24) = *(undefined4 *)((long)param_1 + 0x34);
  *(undefined4 *)(param_3 + 0x28) = *(undefined4 *)((long)param_1 + 0x38);
  *(undefined4 *)(param_3 + 0x2c) = *(undefined4 *)((long)param_1 + 0x3c);
  *(undefined4 *)(param_3 + 0x30) = *(undefined4 *)((long)param_1 + 0x40);
  *(undefined4 *)(param_3 + 0x34) = *(undefined4 *)((long)param_1 + 0x44);

  // TD-SCDMA
  *(undefined4 *)(param_3 + 0x38) = *(undefined4 *)((long)param_1 + 0x48);
  *(undefined4 *)(param_3 + 0x3c) = *(undefined4 *)((long)param_1 + 0x4c);    
  *(undefined4 *)(param_3 + 0x40) = *(undefined4 *)((long)param_1 + 0x50);

*/

int convertRilSignalStrengthToHal_1_4(const void* response, size_t responseLen) {
 int iVar1;
 SignalStrength_1_4 signalStrength = {};

   // convertRilSignalStrengthToHal_1_4: convertRilSignalStrengthToHal : responseLen 104 - sizeof 108     
   RLOGD("%s: convertRilSignalStrengthToHal v1.4 :. responseLen %d - sizeof %d ", __func__, responseLen, sizeof(signalStrength));

   //dump_response((unsigned char *)response);
   //dump_response((unsigned char *)response+0x20);
   //dump_response((unsigned char *)response+0x40);

   if (*(int *)((long)response + 0x30) + 1U < 2) *(int32_t *)((long)response + 0x30) = 0x7fffffff;
   iVar1 = *(int *)((long)response + 0x34);
   if (iVar1 + 1U < 2) {
      iVar1 = 0x7fffffff;
   }

   *(int *)((long)response + 0x34) = iVar1;

   if (*(int *)((long)response + 0x38) + 1U < 2) *(int32_t *)((long)response + 0x38) = 0x7fffffff;
   if (*(int *)((long)response + 0x40) + 1U < 2) *(int32_t *)((long)response + 0x40) = 0x7fffffff;

   if ((iVar1 == 0x7fffffff) && (*(int *)((long)response + 0x3c) + 1U < 2)) {
      *(int32_t *)((long)response + 0x3c) = 0x7fffffff;
   }
   
   if (*(int *)((long)response + 0x44) + 1U < 2) *(int32_t *)((long)response + 0x44) = 0x7fffffff;

   // GSM
   if (*(int *)((long)response + 0x00) + 1U < 2) *(int32_t *)response = 0x7fffffff;
   if (*(int *)((long)response + 0x04) + 1U < 2) *(int32_t *)((long)response + 4) = 0x7fffffff;
   if (*(int *)((long)response + 0x08) + 1U < 2) *(int32_t *)((long)response + 8) = 0x7fffffff;
   if (*(int *)((long)response + 0x0c) + 1U < 2) *(int32_t *)((long)response + 0xc) = 0x7fffffff;
   if (*(int *)((long)response + 0x10) + 1U < 2) *(int32_t *)((long)response + 0x10) = 0x7fffffff;
   if (*(int *)((long)response + 0x14) + 1U < 2) *(int32_t *)((long)response + 0x14) = 0x7fffffff;
   if (*(int *)((long)response + 0x18) + 1U < 2) *(int32_t *)((long)response + 0x18) = 0x7fffffff;
   if (*(int *)((long)response + 0x48) + 1U < 2) *(int32_t *)((long)response + 0x48) = 0x7fffffff;
   if (*(int *)((long)response + 0x4c) + 1U < 2) *(int32_t *)((long)response + 0x4c) = 0x7fffffff;
   if (*(int *)((long)response + 0x50) + 1U < 2) *(int32_t *)((long)response + 0x50) = 0x7fffffff;
   if (*(int *)((long)response + 0x1c) + 1U < 2) *(int32_t *)((long)response + 0x1c) = 0x7fffffff;
   if (*(int *)((long)response + 0x20) + 1U < 2) *(int32_t *)((long)response + 0x20) = 0x7fffffff;
   if (*(int *)((long)response + 0x24) + 1U < 2) *(int32_t *)((long)response + 0x24) = 0x7fffffff;
   if (*(int *)((long)response + 0x28) + 1U < 2) *(int32_t *)((long)response + 0x28) = 0x7fffffff;
   if (*(int *)((long)response + 0x2c) + 1U < 2) *(int32_t *)((long)response + 0x2c) = 0x7fffffff;

   ALOGD("%s: after signalStrength", __func__);

   /* -------------------------------- GSM = 3*4 = 12 = 0x00 -> 0x08 ------------------------------------------*/
   signalStrength.gsm.signalStrength = *(int32_t *)((long)response + 0x0); // huawei send rssi
   signalStrength.gsm.bitErrorRate = *(int32_t *)((long)response + 0x4);
   signalStrength.gsm.timingAdvance = *(int32_t *)((long)response + 0x8);

    // Fix GSM
//    *(int32_t *)((long)response + 0x00) = signalStrength.gsm.signalStrength;
   if (signalStrength.gsm.signalStrength >= -70) {
        signalStrength.gsm.signalStrength = 30;
   } else if (signalStrength.gsm.signalStrength>= -80) {
        signalStrength.gsm.signalStrength = 20;
   } else if (signalStrength.gsm.signalStrength >= -90) {
        signalStrength.gsm.signalStrength = 10;
   } else if (signalStrength.gsm.signalStrength >= -110) {
        signalStrength.gsm.signalStrength = 5;
   }
   *(int32_t *)((long)response + 0x00) = signalStrength.gsm.signalStrength;

    /* -------------------------------- WCDMA -------------------------------------------------------------------------------*/
   // WCDMA
   signalStrength.wcdma.signalStrength = *(int32_t *)((long)response + 0xc);  // signalStrength=rssi (not use)
   signalStrength.wcdma.bitErrorRate = *(int32_t *)((long)response + 0x10);
   signalStrength.wcdma.rscp = *(int32_t *)((long)response + 0x14);
   signalStrength.wcdma.ecno = *(int32_t *)((long)response + 0x18);

   // Valid values are (0-31, 99) as defined in TS 27.007 8.5
   // Ec/No=RSCP−RSSI
   // RSSI=RSCP-Ec/No
   /*
	0        -113 dBm or less  (low)
	1        -111 dBm  
	2...30   -109... -53 dBm  
	31       -51 dBm or greater
	
	*(int32_t *)((long)response + 0xc) = 5; // 20=-73db (tres fort) --- 10=-93db   	
   */
   
   signalStrength.wcdma.signalStrength = 0x7FFFFFFF;
   if (signalStrength.wcdma.rscp >= -70) {
        signalStrength.wcdma.signalStrength = 30;
   } else if (signalStrength.wcdma.rscp >= -80) {
        signalStrength.wcdma.signalStrength = 20;
   } else if (signalStrength.wcdma.rscp >= -90) {
        signalStrength.wcdma.signalStrength = 10;
   } else if (signalStrength.wcdma.rscp >= -110) {
        signalStrength.wcdma.signalStrength = 5;
   }
   *(int32_t *)((long)response + 0xc) = signalStrength.wcdma.signalStrength;
   

   /* -------------------------------- CDMA -------------------------------------------------------------------------------*/
   signalStrength.cdma.dbm = *(int32_t *)((long)response + 0x1c);
   signalStrength.cdma.ecio = *(int32_t *)((long)response + 0x20);

   /* -------------------------------- EVO -------------------------------------------------------------------------------*/
   // EVO = 3*4  = 12 = 0x24 -> 0x2c
   signalStrength.evdo.dbm = *(int32_t *)((long)response + 0x24);
   signalStrength.evdo.ecio = *(int32_t *)((long)response + 0x28);
   signalStrength.evdo.signalNoiseRatio = *(int32_t *)((long)response + 0x2c);

   /* -------------------------------- LTE -------------------------------------------------------------------------------*/
   signalStrength.lte.signalStrength = *(int32_t *)((long)response + 0x30);
   signalStrength.lte.rsrp = *(int32_t *)((long)response + 0x34);
   signalStrength.lte.rsrq = *(int32_t *)((long)response + 0x38);
   signalStrength.lte.rssnr = *(int32_t *)((long)response + 0x3c);
   signalStrength.lte.cqi = *(int32_t *)((long)response + 0x40);
   signalStrength.lte.timingAdvance = *(int32_t *)((long)response + 0x44);
    
   signalStrength.lte.signalStrength = 0x7FFFFFFF;
   if (signalStrength.lte.rsrp >= -80) {
        signalStrength.lte.signalStrength = 30; // Excellent - Valid values are (0-31, 99) as defined in TS 27.007 8.5 
   } else if (signalStrength.lte.rsrp > -90) {
        signalStrength.lte.signalStrength = 20;
   } else if (signalStrength.lte.rsrp > -100) {
        signalStrength.lte.signalStrength = 10;
   } else if (signalStrength.lte.rsrp <= -100) {
        signalStrength.lte.signalStrength = 5;
   }
   *(int32_t *)((long)response + 0x48) = signalStrength.lte.signalStrength;
    
   /* -------------------------------- TD-SCDMA --------------------------------------------------------------------*/   
   signalStrength.tdscdma.signalStrength = *(int32_t *)((long)response + 0x48); // signalStrength=rssi (not use)
   signalStrength.tdscdma.bitErrorRate = *(int32_t *)((long)response + 0x4c);
   signalStrength.tdscdma.rscp = *(int32_t *)((long)response + 0x50);

   signalStrength.tdscdma.signalStrength = 0x7FFFFFFF;
   if (signalStrength.tdscdma.rscp >= -70) {
        signalStrength.tdscdma.signalStrength = 30;
   } else if (signalStrength.tdscdma.rscp >= -80) {
        signalStrength.tdscdma.signalStrength = 20;
   } else if (signalStrength.tdscdma.rscp >= -90) {
        signalStrength.tdscdma.signalStrength = 10;
   } else if (signalStrength.tdscdma.rscp >= -110) {
        signalStrength.tdscdma.signalStrength = 5;
   }
   *(int32_t *)((long)response + 0x48) = signalStrength.tdscdma.signalStrength;

   RLOGD("RIL SignalStrength GSM signalStrength %d, bitErrorRate %d, timingAdvance %d", signalStrength.gsm.signalStrength,
   signalStrength.gsm.bitErrorRate,
   signalStrength.gsm.timingAdvance);

   RLOGD("RIL SignalStrength CDMA dbm %d, ecio %d",
   signalStrength.cdma.dbm,
   signalStrength.cdma.ecio);

   RLOGD("RIL SignalStrength EVO dbm %d, ecio %d, signalNoiseRatio %d",
   signalStrength.evdo.dbm,
   signalStrength.evdo.ecio,
   signalStrength.evdo.signalNoiseRatio);        

   RLOGD("RIL SignalStrength LTE signalStrength %d, rssnr %d, rsrp %d, rsrq %d, cqi %d, timingAdvance %d",
   signalStrength.lte.signalStrength,
   signalStrength.lte.rssnr,
   signalStrength.lte.rsrp,
   signalStrength.lte.rsrq,
   signalStrength.lte.cqi,
   signalStrength.lte.timingAdvance);

   RLOGD("RIL SignalStrength TDSCDMA signalStrength %d, biterror-rate %d, rscp %d",
   signalStrength.tdscdma.signalStrength,
   signalStrength.tdscdma.bitErrorRate,
   signalStrength.tdscdma.rscp);

   RLOGD("RIL SignalStrength WCDMA signalStrength %d, biterror-rate %d, rscp %d, ecno %d",
   signalStrength.wcdma.signalStrength,
   signalStrength.wcdma.bitErrorRate,
   signalStrength.wcdma.rscp,
   signalStrength.wcdma.ecno);
   
   return responseLen;
}


// This is required by libreference-ril
extern "C" char* requestToString(int request) {
    auto orig_RIL_requestToString = reinterpret_cast<char* (*)(int)>(dlsym(RTLD_NEXT, __func__));
    return orig_RIL_requestToString(request);
}

//RIL_onRequestComplete(t, RIL_E_SUCCESS, response, sizeof(response));
extern "C" void RIL_onRequestComplete(RIL_Token t, RIL_Errno e, void* response,
                                      size_t responselen) {
    auto orig_RIL_onRequestComplete =
            reinterpret_cast<void (*)(RIL_Token, RIL_Errno, void*, size_t)>(
                    dlsym(RTLD_NEXT, __func__));
    
    RequestInfo* pRI = (RequestInfo*)t;
    int request = (pRI && pRI->pCI) ? pRI->pCI->requestNumber : -1;

    //socket_id = pRI->socket_id;
    //RLOGD("RequestComplete, %s", rilSocketIdToString(socket_id));

    if (pRI->local > 0) {
        // Locally issued command...void only!
        // response does not go back up the command socket
        RLOGD("C[locl]< %s", requestToString(pRI->pCI->requestNumber));
        goto do_not_handle;
    }

    if (!pRI) {
        RLOGW("%s: request info is NULL", __func__);
        goto do_not_handle;
    }
    
     if (response == NULL) {
        RLOGW("%s: response is NULL", __func__);
        goto do_not_handle;
    }

    ALOGI("%s: wrapper receive request %d code, len %d with errornum %d ", __func__, request, responselen, e);

    if (request != -1) {
        switch (request) {
            case RIL_REQUEST_HW_SIGNAL_STRENGTH: // 690
                ALOGD("%s: RIL request RIL_REQUEST_HW_SIGNAL_STRENGTH", __func__); // 
                //responselen = convertRilHwSignalStrengthToHal(response, responselen);
                break;
            case RIL_REQUEST_SIGNAL_STRENGTH:    // 19
                ALOGD("%s: RIL request RIL_REQUEST_SIGNAL_STRENGTH", __func__); // convertRilSignalStrengthToHal_1_4 : responseLen 104
                responselen = convertRilSignalStrengthToHal_1_4(response, responselen);
                break;
        }
    }

do_not_handle:
    orig_RIL_onRequestComplete(t, e, response, responselen);
}

extern "C" void RIL_onUnsolicitedResponse(int unsolResponse, const void* data, size_t datalen,
                                          RIL_SOCKET_ID modemid) {
    auto orig_RIL_onUnsolicitedResponse =
            reinterpret_cast<void (*)(int, const void*, size_t, RIL_SOCKET_ID)>(
                    dlsym(RTLD_NEXT, __func__));

    if (!data) {
        ALOGW("%s: data is NULL", __func__);
        goto do_not_handle;
    }

    switch (unsolResponse) {
  // Huawei - code
        case RIL_UNSOL_HW_EXIST_NETWORK_INFO: //2054
            ALOGD("%s: RIL_UNSOL_HW_EXIST_NETWORK_INFO enter", __func__);
            break;
        case RIL_UNSOL_HW_RESIDENT_NETWORK_CHANGED:
            ALOGD("%s: RIL_UNSOL_HW_RESIDENT_NETWORK_CHANGED enter", __func__);
            break;
        case RIL_UNSOL_HW_PLMN_SEARCH_INFO_IND:
            ALOGD("%s: RIL_UNSOL_HW_PLMN_SEARCH_INFO_IND enter", __func__);
            break;
        case RIL_UNSOL_HW_RIL_CHR_IND:
            ALOGD("%s: RIL_UNSOL_HW_RIL_CHR_IND enter", __func__);
            break;
        case RIL_UNSOL_HW_NETWORK_REJECT_CASE:
            ALOGD("%s: RIL_UNSOL_HW_NETWORK_REJECT_CASE enter", __func__);
            break;
        case RIL_UNSOL_HW_IMS_SRV_STATUS_UPDATE:
            ALOGD("%s: RIL_UNSOL_HW_IMS_SRV_STATUS_UPDATE enter", __func__);
            break;
        case RIL_UNSOL_HW_SIGNAL_STRENGTH: //2077 - RIL_UNSOL_HW_SIGNAL_STRENGTH - datalen 64
            ALOGD("%s: RIL_UNSOL_HW_SIGNAL_STRENGTH - datalen %lu", __func__, (unsigned long)datalen);
            //datalen = convertRilHwSignalStrengthToHal(data, datalen);
            break;
            
  // AOSP code          
        case RIL_UNSOL_SIGNAL_STRENGTH:             //1009 - RIL_UNSOL_SIGNAL_STRENGTH - datalen 104
            ALOGD("%s: RIL_UNSOL_SIGNAL_STRENGTH - datalen %lu", __func__, (unsigned long)datalen);
            datalen = convertRilSignalStrengthToHal_1_4(data, datalen);
            break;
        case RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED: //1019 - RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED
            ALOGD("%s: RIL_UNSOL_SIGNAL_STRENGTH - datalen %lu", __func__, (unsigned long)datalen);
            break;
        case RIL_UNSOL_RESTRICTED_STATE_CHANGED:    //1023 - RIL_UNSOL_RESTRICTED_STATE_CHANGED
            ALOGD("%s: RIL_UNSOL_RESTRICTED_STATE_CHANGED - datalen %lu", __func__, (unsigned long)datalen);
            break;
        case RIL_UNSOL_VOICE_RADIO_TECH_CHANGED:    //1035 - RIL_UNSOL_VOICE_RADIO_TECH_CHANGED
            ALOGD("%s: RIL_UNSOL_VOICE_RADIO_TECH_CHANGED - datalen %lu", __func__, (unsigned long)datalen);
            break;
        default:
           ALOGI("%s: Rilv4 receive unsolResponse %d code for modem %d ", __func__, unsolResponse, modemid);
    }
    


do_not_handle:
    orig_RIL_onUnsolicitedResponse(unsolResponse, data, datalen, modemid);
}
