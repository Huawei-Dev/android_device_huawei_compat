/*
* Copyright (C) 2014 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef RIL_H_INCLUDED
#define RIL_H_INCLUDED


/******************************************************


   HAL V1.4


********************************************************/

struct GsmSignalStrength final {
    uint32_t signalStrength __attribute__ ((aligned(4)));
    uint32_t bitErrorRate __attribute__ ((aligned(4)));
    int32_t timingAdvance __attribute__ ((aligned(4)));
};


struct CdmaSignalStrength final {
    uint32_t dbm __attribute__ ((aligned(4)));
    uint32_t ecio __attribute__ ((aligned(4)));
};

struct EvdoSignalStrength final {
    uint32_t dbm __attribute__ ((aligned(4)));
    uint32_t ecio __attribute__ ((aligned(4)));
    uint32_t signalNoiseRatio __attribute__ ((aligned(4)));
};

struct LteSignalStrength final {
    uint32_t signalStrength __attribute__ ((aligned(4)));
    uint32_t rsrp __attribute__ ((aligned(4)));
    uint32_t rsrq __attribute__ ((aligned(4)));
    int32_t rssnr __attribute__ ((aligned(4)));
    uint32_t cqi __attribute__ ((aligned(4)));
    uint32_t timingAdvance __attribute__ ((aligned(4)));
};

struct TdscdmaSignalStrength_1_2 final {
    /**
     * UTRA carrier RSSI as defined in TS 25.225 5.1.4
     * Valid values are (0-31, 99) as defined in TS 27.007 8.5
     * INT_MAX denotes that the value is invalid/unreported.
     */
    uint32_t signalStrength __attribute__ ((aligned(4)));
    /**
     * Transport Channel BER as defined in TS 25.225 5.2.5
     * Valid values are (0-7, 99) as defined in TS 27.007 8.5
     * INT_MAX denotes that the value is invalid/unreported.
     */
    uint32_t bitErrorRate __attribute__ ((aligned(4)));
    /**
     * P-CCPCH RSCP as defined in TS 25.225 5.1.1
     * Valid values are (0-96, 255) as defined in TS 27.007 8.69
     * INT_MAX denotes that the value is invalid/unreported.
     */
    uint32_t rscp __attribute__ ((aligned(4)));
};



struct WcdmaSignalStrength_1_2 final {
    int32_t signalStrength __attribute__ ((aligned(4)));
    int32_t bitErrorRate __attribute__ ((aligned(4)));
   
    /**
     * CPICH RSCP as defined in TS 25.215 5.1.1
     * Valid values are (0-96, 255) as defined in TS 27.007 8.69
     * INT_MAX denotes that the value is invalid/unreported.
     */
    uint32_t rscp __attribute__ ((aligned(4)));
    /**
     * Ec/No value as defined in TS 25.215 5.1.5
     * Valid values are (0-49, 255) as defined in TS 27.007 8.69
     * INT_MAX denotes that the value is invalid/unreported.
     */
    uint32_t ecno __attribute__ ((aligned(4)));
};


struct NrSignalStrength_1_4 final {
    /**
     * SS reference signal received power, multipled by -1.
     *
     * Reference: 3GPP TS 38.215.
     *
     * Range [44, 140], INT_MAX means invalid/unreported.
     */
    int32_t ssRsrp __attribute__ ((aligned(4)));
    /**
     * SS reference signal received quality, multipled by -1.
     *
     * Reference: 3GPP TS 38.215.
     *
     * Range [3, 20], INT_MAX means invalid/unreported.
     */
    int32_t ssRsrq __attribute__ ((aligned(4)));
    /**
     * SS signal-to-noise and interference ratio.
     *
     * Reference: 3GPP TS 38.215 section 5.1.*, 3GPP TS 38.133 section 10.1.16.1.
     *
     * Range [-23, 40], INT_MAX means invalid/unreported.
     */
    int32_t ssSinr __attribute__ ((aligned(4)));
    /**
     * CSI reference signal received power, multipled by -1.
     *
     * Reference: 3GPP TS 38.215.
     *
     * Range [44, 140], INT_MAX means invalid/unreported.
     */
    int32_t csiRsrp __attribute__ ((aligned(4)));
    /**
     * CSI reference signal received quality, multipled by -1.
     *
     * Reference: 3GPP TS 38.215.
     *
     * Range [3, 20], INT_MAX means invalid/unreported.
     */
    int32_t csiRsrq __attribute__ ((aligned(4)));
    /**
     * CSI signal-to-noise and interference ratio.
     *
     * Reference: 3GPP TS 138.215 section 5.1.*, 3GPP TS 38.133 section 10.1.16.1.
     *
     * Range [-23, 40], INT_MAX means invalid/unreported.
     */
    int32_t csiSinr __attribute__ ((aligned(4)));
};


struct SignalStrength_1_4 final {
    // 1.0 type
    /**
     * If GSM measurements are provided, this structure must contain valid measurements; otherwise
     * all fields should be set to INT_MAX to mark them as invalid.
     */
    GsmSignalStrength gsm __attribute__ ((aligned(4)));    // 3*4 = 12 - 0,4,0x8
    /**
     * If CDMA measurements are provided, this structure must contain valid measurements; otherwise
     * all fields should be set to INT_MAX to mark them as invalid.
     */
    CdmaSignalStrength cdma __attribute__ ((aligned(4)));  // 2*4 = 10 - 0xC,0x10
    /**
     * If EvDO measurements are provided, this structure must contain valid measurements; otherwise
     * all fields should be set to INT_MAX to mark them as invalid.
     */
    EvdoSignalStrength evdo __attribute__ ((aligned(4)));  // 3*4 = 12 - 0x14,0x18,0x1c
    /**
     * If LTE measurements are provided, this structure must contain valid measurements; otherwise
     * all fields should be set to INT_MAX to mark them as invalid.
     */
    LteSignalStrength lte __attribute__ ((aligned(4)));  // 32         - 0x20
    
    // 1.2 type
    /**
     * If TD-SCDMA measurements are provided, this structure must contain valid measurements;
     * otherwise all fields should be set to INT_MAX to mark them as invalid.
     */
    TdscdmaSignalStrength_1_2 tdscdma __attribute__ ((aligned(4)));
    /**
     * If WCDMA measurements are provided, this structure must contain valid measurements; otherwise
     * all fields should be set to INT_MAX to mark them as invalid.
     */
    WcdmaSignalStrength_1_2 wcdma __attribute__ ((aligned(4)));
    
    // 1.4 type
    /**
     * If NR 5G measurements are provided, this structure must contain valid measurements; otherwise
     * all fields should be set to INT_MAX to mark them as invalid.
     */
    NrSignalStrength_1_4 nr __attribute__ ((aligned(4)));
};



/******************************************************


   HAL V1.0


********************************************************/


struct TdScdmaSignalStrength {
    uint32_t rscp;                        // The Received Signal Code Power in dBm multiplied by -1.
                                          // Range : 25 to 120
                                          // INT_MAX: 0x7FFFFFFF denotes invalid/unreported value.
                                          // Reference: 3GPP TS 25.123, section 9.1.1.1
};

struct SignalStrength {
    /**
     * If GSM measurements are provided, this structure must contain valid measurements; otherwise
     * all fields should be set to INT_MAX to mark them as invalid.
     */
    GsmSignalStrength gsm;
    /**
     * If CDMA measurements are provided, this structure must contain valid measurements; otherwise
     * all fields should be set to INT_MAX to mark them as invalid.
     */
    CdmaSignalStrength cdma;
    /**
     * If EvDO measurements are provided, this structure must contain valid measurements; otherwise
     * all fields should be set to INT_MAX to mark them as invalid.
     */
    EvdoSignalStrength evdo;
    /**
     * If LTE measurements are provided, this structure must contain valid measurements; otherwise
     * all fields should be set to INT_MAX to mark them as invalid.
     */
    LteSignalStrength lte;
    /**
     * If TD-SCDMA measurements are provided, this structure must contain valid measurements;
     * otherwise all fields should be set to INT_MAX to mark them as invalid.
     */
    TdScdmaSignalStrength tdScdma;
};

#endif
