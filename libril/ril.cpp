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



/*
10-01 09:30:04.500  1436  1436 D RILJ    : [0649]> SIGNAL_STRENGTH [PHONE0]
10-01 09:30:04.500   804   804 D RILC    : getSignalStrength_1_4: serial 649
10-01 09:30:04.501   804   869 I AT      : m0_7>AT^LWCLASH?
10-01 09:30:04.506   804   876 I AT      : m0_7<^LWCLASH: 0,25425,4,26625,4,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
10-01 09:30:04.506   804   876 I AT      : m0_7<OK
10-01 09:30:04.506   804   869 I RIL_GU  : m0 GetParamValueByName: param is: ro.odm.radio.signal_smooth_alg_config, param value is:0
10-01 09:30:04.506   804   869 E libril4-wrapper: RIL_onRequestComplete: Iceows
10-01 09:30:04.506   804   869 D libril4-wrapper: RIL_onRequestComplete: request signal strenght
10-01 09:30:04.506   804   869 D RILC    : RequestComplete, RIL_SOCKET_1
10-01 09:30:04.506   804   869 E RILC    : Calling responseFunction() for token 649
10-01 09:30:04.507   804   869 D RILC    : getSignalStrengthResponse: serial 649
10-01 09:30:04.507   804   869 D RILC    : getSignalStrengthResponse4: serial 649
10-01 09:30:04.507   804   869 E RILC    : getSignalStrengthResponse: Invalid response
10-01 09:30:04.507   804   869 E RIL_GU  : FreeRequestData, data is NULL, requestId: 19
10-01 09:30:04.508  1436  1612 E CellSignalStrengthLte: convertRssiAsuToDBm Iceows ASU=0
10-01 09:30:04.509  1436  1612 D RILJ    : [0649]< SIGNAL_STRENGTH error 66 [PHONE0]
10-01 09:30:04.510  1436  1612 D RilRequest: [0649]< SIGNAL_STRENGTH error: com.android.internal.telephony.CommandException: INVALID_RESPONSE ret=SignalStrength:{mCdma=CellSignalStrengthCdma: cdmaDbm=0 cdmaEcio=0 evdoDbm=0 evdoEcio=0 evdoSnr=0 level=4,mGsm=CellSignalStrengthGsm: rssi=-113 ber=0 mTa=0 mLevel=0,mWcdma=CellSignalStrengthWcdma: ss=-113 ber=0 rscp=-120 ecno=-24 level=0,mTdscdma=CellSignalStrengthTdscdma: rssi=-113 ber=0 rscp=-120 level=0,mLte=CellSignalStrengthLte: rssi=-113 rsrp=2147483647 rsrq=0 rssnr=0 cqiTableIndex=2147483647 cqi=0 ta=0 level=1 parametersUseForLevel=1,mNr=CellSignalStrengthNr:{ csiRsrp = 2147483647 csiRsrq = 2147483647 csiCqiTableIndex = 2147483647 csiCqiReport = [] ssRsrp = 2147483647 ssRsrq = 0 ssSinr = 0 level = 0 parametersUseForLevel = 1 },primary=CellSignalStrengthLte} result={ when=-4m1s972ms what=9 target=com.android.internal.telephony.SignalStrengthController }
10-01 09:30:04.510   804   869 I AT      : m0_7>AT^CERSSI?
10-01 09:30:04.514   804   876 I AT      : m0_7<^CERSSI:3,0,0,0,255,-101,-8,-9,99,32639,32639
10-01 09:30:04.514   804   876 I AT      : m0_7<OK
10-01 09:30:04.516   804   869 E RIL_GU  : m0 UnsolicitedRSSI, g_atCerssiLteNrFlag = 0, mRssi = -101
10-01 09:30:04.517   804   869 I RIL_GU  : m0 GetParamValueByName: param is: ro.odm.radio.signal_smooth_alg_config, param value is:0
10-01 09:30:04.517   804   869 I AT      : m0_7>AT^LWCLASH?
10-01 09:30:04.522   804   876 I AT      : m0_7<^LWCLASH: 0,25425,4,26625,4,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
10-01 09:30:04.522   804   876 I AT      : m0_7<OK
10-01 09:30:04.523   804   869 E libril4-wrapper: RIL_onUnsolicitedResponse: Iceows
10-01 09:30:04.523   804   869 E RILC    : currentSignalStrengthIndV1_4: invalid response
10-01 09:30:04.523   804   869 I RILC    : RIL_SOCKET_1 UNSOLICITED: UNSOL_SIGNAL_STRENGTH length:56
10-01 09:30:04.523   804   869 E libril4-wrapper: RIL_onUnsolicitedResponse: Iceows
10-01 09:30:04.523   804   869 D rild    : currentHwSignalStrength_1_1: indicationType is 1
10-01 09:30:04.523   804   869 E rild    : currentHwSignalStrengthInd:hidl version is 1.0 
10-01 09:30:04.523   804   869 E rild    : radio::currentHwSignalStrengthInd: HisiRadioService[0]->mRadioIndication == NULL
10-01 09:30:04.523   804   869 I RILC    : RIL_SOCKET_1 UNSOLICITED: <unknown request> length:64
10-01 09:30:04.542  1992  1993 E bms_file: file /sys/class/hw_power/charger/direct_charger_sc/direct_charge_succ is invalid
10-01 09:30:04.546  1992  1993 I bms_behavior: monitor interval count 35
10-01 09:30:04.546     0     0 I         : [  253.955444s][pid:1993,cpu0,bms_behavior,4][I/bq25892_charger] vbus mv is 4900mV
10-01 09:30:04.553  1436  1436 D PhoneSwitcher: EVENT_MODEM_COMMAND_RETRY: resend modem command on phone 0
*/



/*
https://github.com/dstmath/HWFramework/blob/672bb34094b8780806a10ba9b1d21036fd808b8e/MATE-20_EMUI_11.0.0/src/main/java/com/android/internal/telephony/HwSignalStrength.java#L17

    protected static final String DEFAULT_SIGNAL_CUST_CDMA = "5,false,-112,-106,-99,-92,-85";
    protected static final String DEFAULT_SIGNAL_CUST_CDMALTE = "5,false,-120,-115,-110,-105,-97";
    protected static final String DEFAULT_SIGNAL_CUST_EVDO = "5,false,-112,-106,-99,-92,-85";
    protected static final String DEFAULT_SIGNAL_CUST_GSM = "5,false,-109,-103,-97,-91,-85";
    protected static final String DEFAULT_SIGNAL_CUST_LTE = "5,false,-120,-115,-110,-105,-97";
    protected static final String DEFAULT_SIGNAL_CUST_NR = "5,false,-120,-115,-110,-105,-97";
    protected static final String DEFAULT_SIGNAL_CUST_UMTS = "5,false,-112,-105,-99,-93,-87";
    */
    
/*
void convertRilHwSignalStrengthToHal(void *response, size_t responseLen, HwSignalStrength_1_1 *signalStrength)
{
  int iVar1;
  ulong uVar2;
  
  if (responseLen >> 0x21 != 0) {
     // WARNING: Subroutine does not return
    __ubsan_handle_implicit_conversion_minimal_abort();
  }
  if (response != (void *)0x0) {
    iVar1 = (int)(responseLen >> 2);
    if (0 < iVar1) {
      uVar2 = 0;
      do {
        // WARNING: Load size is inaccurate
        if ((*(int *)((long)response + uVar2 * 4) + 1U < 2) &&
           ((((uVar2 != 0xc || (-1 < *(int *)((long)response + 0x28))) || (*response != 0x7fffffff))
            || (*(int *)((long)response + 8) != 0x7fffffff)))) {
          *(void *)((long)response + uVar2 * 4) = 0x7fffffff;
        }
        uVar2 = uVar2 + 1;
      } while ((responseLen >> 2 & 0xffffffff) != uVar2);
    }
    // Load size is inaccurate
    *(void *)signalStrength = *response;
    *(void *)(signalStrength + 0x14) = *(void *)((long)response + 8);
    *(void *)(signalStrength + 0x18) = *(void *)((long)response + 0xc);
    *(void *)(signalStrength + 0x1c) = *(void *)((long)response + 0x10);
    *(void *)(signalStrength + 0x20) = *(void *)((long)response + 0x14);
    *(void *)(signalStrength + 0x24) = *(void *)((long)response + 0x18);
    *(void *)(signalStrength + 0x28) = *(void *)((long)response + 0x20);
    *(void *)(signalStrength + 0x30) = *(void *)((long)response + 0x24);
    *(void *)(signalStrength + 0x34) = *(void *)((long)response + 0x28);
    *(void *)(signalStrength + 0x38) = *(void *)((long)response + 0x2c);
    *(void *)(signalStrength + 0x3c) = *(void *)((long)response + 0x30);
    *(void *)(signalStrength + 0x48) = *(void *)((long)response + 0x3c);
    if (iVar1 == 0x13) {
      *(void *)(signalStrength + 0x50) = *(void *)((long)response + 0x40);
      *(void *)(signalStrength + 0x54) = *(void *)((long)response + 0x44);
      *(void *)(signalStrength + 0x58) = *(void *)((long)response + 0x48);
      return;
    }
    *(undefined8 *)(signalStrength + 0x50) = 0x7fffffff7fffffff;
    *(void *)(signalStrength + 0x58) = 0x7fffffff;
    return;
  }
  __android_log_buf_print(1,6,0,"responseInts: Invalid response %d");
 
  return;
}
*/

static void dump_hash_auth(unsigned char *hash_buf)
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




static size_t fixSignalStrength(const void* response, size_t datalen) {
    RIL_SignalStrength_v10* rilResponse = (RIL_SignalStrength_v10*)response;

    ALOGD("%s: fixSignalStrength - size datalen %d", __func__, sizeof(RIL_SignalStrength_v10));
    
    // LTE  /* LTE_SignalStrength Valid values are (0-31, 99) as defined in TS 27.007 8.5 */
    if (rilResponse->LTE_SignalStrength.rsrp >= -97) {
        rilResponse->LTE_SignalStrength.signalStrength = 63;
    } else if (rilResponse->LTE_SignalStrength.rsrp >= -105) {
        rilResponse->LTE_SignalStrength.signalStrength = 10;
    } else if (rilResponse->LTE_SignalStrength.rsrp >= -113) {
        rilResponse->LTE_SignalStrength.signalStrength = 5;
    } else if (rilResponse->LTE_SignalStrength.rsrp >= -125) {
        rilResponse->LTE_SignalStrength.signalStrength = 3;
    } else if (rilResponse->LTE_SignalStrength.rsrp >= -44) {
        rilResponse->LTE_SignalStrength.signalStrength = 64;
     }

    // EvDO
    if (rilResponse->EVDO_SignalStrength.dbm >= -89) {
        rilResponse->EVDO_SignalStrength.dbm = -65;
        rilResponse->EVDO_SignalStrength.signalNoiseRatio = 7;
    } else if (rilResponse->EVDO_SignalStrength.dbm >= -99) {
        rilResponse->EVDO_SignalStrength.dbm = -75;
        rilResponse->EVDO_SignalStrength.signalNoiseRatio = 5;
    } else if (rilResponse->EVDO_SignalStrength.dbm >= -106) {
        rilResponse->EVDO_SignalStrength.dbm = -90;
        rilResponse->EVDO_SignalStrength.signalNoiseRatio = 3;
    } else if (rilResponse->EVDO_SignalStrength.dbm >= -112) {
        rilResponse->EVDO_SignalStrength.dbm = -105;
        rilResponse->EVDO_SignalStrength.signalNoiseRatio = 1;
    } else {
        rilResponse->EVDO_SignalStrength.dbm = -999;
        rilResponse->EVDO_SignalStrength.signalNoiseRatio = -999;
    }

    // CDMA
    if (rilResponse->CDMA_SignalStrength.dbm >= -89) {
        rilResponse->CDMA_SignalStrength.dbm = -75;
        rilResponse->CDMA_SignalStrength.ecio = -90;
    } else if (rilResponse->CDMA_SignalStrength.dbm >= -99) {
        rilResponse->CDMA_SignalStrength.dbm = -85;
        rilResponse->CDMA_SignalStrength.ecio = -110;
    } else if (rilResponse->CDMA_SignalStrength.dbm >= -106) {
        rilResponse->CDMA_SignalStrength.dbm = -95;
        rilResponse->CDMA_SignalStrength.ecio = -130;
    } else if (rilResponse->CDMA_SignalStrength.dbm >= -112) {
        rilResponse->CDMA_SignalStrength.dbm = -100;
        rilResponse->CDMA_SignalStrength.ecio = -130;
    } else {
        rilResponse->CDMA_SignalStrength.dbm = -100;
        rilResponse->CDMA_SignalStrength.ecio = -150;
    }

    // GSM
    if (rilResponse->GW_SignalStrength.signalStrength >= -89) {
        rilResponse->GW_SignalStrength.signalStrength = 12;
    } else if (rilResponse->GW_SignalStrength.signalStrength >= -97) {
        rilResponse->GW_SignalStrength.signalStrength = 8;
    } else if (rilResponse->GW_SignalStrength.signalStrength >= -103) {
        rilResponse->GW_SignalStrength.signalStrength = 5;
    } else {
        rilResponse->GW_SignalStrength.signalStrength = 0;
    }

    if (rilResponse->GW_SignalStrength.signalStrength != -1) {
        rilResponse->GW_SignalStrength.signalStrength =
                -(rilResponse->GW_SignalStrength.signalStrength - 113) / 2;
    }

    return sizeof(RIL_SignalStrength_v10);
}



/***************************** to HAL - size = 56 - RIL_SignalStrength_v10* rilResponse
  
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




/**************************

  // GSM
  *(int32_t *)(signalStrength + 0x0) = *(int32_t *)((long)response + 0x0);
  *(int32_t *)(signalStrength + 0x4) = *(int32_t *)((long)response + 0x4);
  *(int32_t *)(signalStrength + 0x8) = *(int32_t *)((long)response + 0x8);

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

  // WCDMA
  *(undefined4 *)(param_3 + 0x44) = *(undefined4 *)((long)param_1 + 0xc);
  *(undefined4 *)(param_3 + 0x48) = *(undefined4 *)((long)param_1 + 0x10);
  *(undefined4 *)(param_3 + 0x4c) = *(undefined4 *)((long)param_1 + 0x14);
  *(undefined4 *)(param_3 + 0x50) = *(undefined4 *)((long)param_1 + 0x18);
    
*/

int convertRilSignalStrengthToHal_1_4(const void* response, size_t responseLen) {
 int iVar1;
 SignalStrength_1_4 signalStrength = {};

   // convertRilSignalStrengthToHal_1_4: convertRilSignalStrengthToHal : responseLen 104 - sizeof 108     
   RLOGD("%s: convertRilSignalStrengthToHal v1.4 :. responseLen %d - sizeof %d ", __func__, responseLen, sizeof(signalStrength));

   dump_hash_auth((unsigned char *)response);
   dump_hash_auth((unsigned char *)response+0x20);
   dump_hash_auth((unsigned char *)response+0x40);

   if (*(int *)((long)response + 0x30) + 1U < 2) *(int32_t *)((long)response + 0x30) = 0x7fffffff;
   iVar1 = *(int *)((long)response + 0x34);
   if (iVar1 + 1U < 2) {
      iVar1 = 0x7fffffff;
   }
   //else {
   //if (iVar1 != -2 && iVar1 + 2 < 0 == SCARRY4(iVar1,2)) goto LAB_00172cc8;
   //iVar1 = -iVar1;
   //}*/
   *(int *)((long)response + 0x34) = iVar1;

LAB_00172cc8:
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
    

    ALOGI("%s: Iceows receive request %d code, len %d with errornum %d ", __func__, request, responselen, e);
    
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
           ALOGI("%s: Iceows receive unsolResponse %d code for modem %d ", __func__, unsolResponse, modemid);
    }
    


do_not_handle:
    orig_RIL_onUnsolicitedResponse(unsolResponse, data, datalen, modemid);
}
