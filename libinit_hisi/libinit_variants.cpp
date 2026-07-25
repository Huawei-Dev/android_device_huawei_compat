/*
 * Copyright (C) 2023 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "libinit_variants"
#include <libinit_utils.h>
#include <libinit_variants.h>

#include <android-base/logging.h>
#include <android-base/strings.h>

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <filesystem>
#include <cstring>
#include <cstdlib>
#include <algorithm>

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct ProductInfo {
    // From oeminfo
    std::string devicehw;
    std::string infostr = "";
    std::string region;
    std::string model;
    std::string marketname;

    // result of the parse
    std::string version;
    std::string baseband;
    std::string device;
    std::string board;
    std::string camera;

    // TODO
    std::string brand;
};

constexpr const char* kOemInfoPath = "/dev/block/by-name/oeminfo";

ProductInfo ParseProductInfo(const std::string& product_info_str) {
    ProductInfo product_info;
    std::istringstream iss(product_info_str);

    // Extract the model (i.e. "PRA-LX1").
    std::getline(iss, product_info.model, ' ');

    // Extract the version (i.e. "9.1.0.311").
    std::getline(iss, product_info.version, '(');

    // Remove trailing whitespace.
    if (!product_info.version.empty() && product_info.version.back() == ')') {
        product_info.version.pop_back();
    }

    // Extract the baseband (i.e. "C185E3R2P1").
    std::getline(iss, product_info.baseband, ')');
    return product_info;
}

std::map<int, std::map<int, std::string>> elements = {
    {6, {
        {0x12, "Region"},
        {0x43, "Root Type (info)"},
        {0x44, "rescue Version"},
        {0x4a, "16 byte string 0 terminated"},
        {0x4e, "Rom Version"},
        {0x58, "Alternate ROM Version?"},
        {0x5e, "OEMINFO_VENDER_AND_COUNTRY_NAME_COTA"}, // Taken from fastboot logs
        {0x5b, "Hardware Version Customizeable"},
        {0x5c, "USB Switch?"}, // Guessed from fastboot logs
        {0x61, "Hardware Version"},
        {0x62, "PRF?"},
        {0x65, "Rom Version Customizeable"},
        {0x67, "CN or CDMA info 0x67"},
        {0x68, "CN or CDMA info 0x68"},
        {0x6a, "CN or CDMA info 0x6a"},
        {0x6b, "CN or CDMA info 0x6b"},
        {0x6f, "Software Version"},
        {0x73, "Oeminfo Gamma"}, // From fastboot, but who knows what it actually is, has to do with hisifb_write_gm_to_reserved_mem and the display panel
        {0x76, "pos_delivery constant"},
        {0x8b, "Unknown SHA256 1"},
        {0x85, "3rd_recovery constant"},
        {0x8c, "Software Version as CSV"},
        {0x8d, "Unknown SHA256 2"},
        {0x96, "Unknown SHA256 3"},
        {0xa6, "Update Token"},
        {0xa9, "Some kind of json changelog"},
        {0xb4, "cust version"},
        {0xb6, "preload version"},
        {0xba, "system version"},
        {0x15f, "Logo Boot"}, // Can be overridden in product, version, vendor or system partitions
        {0x160, "Logo Battery Empty"},
        {0x161, "Logo Battery Charge"},
    }},
    {8, {
        {0x5c, "Userlock"},
        {0x5d, "System Lock State"},
        {0x28, "Version number"},
        {0x33, "Software Version as CSV"},
        {0x35, "semicolon separated text containing device identifiers, possibly used in bootloader code generation"},
        {0x3f, "update token"},
        {0x50, "cust version"},
        {0x52, "preload version"},
        {0x56, "system version"},
        {0x5ec, "build number"},
        {0x5ee, "model number"},
        {0xc, "system security data"},
        {0x1197, "Logo Battery Charge"},
        {0x1196, "Logo Battery Empty"},
        {0x1196, "Logo additional (custom format)"},
        {0x1195, "Logo Google"},
    }}
};


namespace {

constexpr size_t kOemInfoHeaderSize = 0x200;
constexpr size_t kOemInfoScanStep = 0x400;
constexpr size_t kMaxTextRecordSize = 4096;

struct OemInfoRecord {
    uint32_t version = 0;
    uint32_t id = 0;
    uint32_t type = 0;
    uint32_t data_len = 0;
    uint32_t age = 0;
    size_t offset = 0;
    std::string data;
};

bool IsWantedRecord(uint32_t version, uint32_t id) {
    if (version == 6) {
        return id == 0x61 || id == 0x12 || id == 0x4e || id == 0x81 || id == 0x5b;
    }

    if (version == 8) {
        // 0x33  - software customization data, including C_version/region
        // 0x5ec - full build number, e.g. EVR-L29 9.1.0.353(C432E3R1P12)
        // 0x5ee - model number, e.g. EVR-L29
        return id == 0x33 || id == 0x5ec || id == 0x5ee;
    }

    return false;
}

bool IsNewerAge(uint32_t candidate, uint32_t current) {
    // OEMINFO keeps redundant copies. Treat age as a wrapping sequence counter.
    return static_cast<int32_t>(candidate - current) > 0;
}

std::string RecordToString(const OemInfoRecord& record) {
    const auto nul = record.data.find('\0');
    return record.data.substr(0, nul);
}

std::string ExtractQuotedValue(const std::string& text, const std::string& key) {
    const std::string marker = "\"" + key + "\":\"";
    const auto value_start = text.find(marker);
    if (value_start == std::string::npos) {
        return {};
    }

    const auto begin = value_start + marker.size();
    const auto end = text.find('"', begin);
    if (end == std::string::npos) {
        return {};
    }

    return text.substr(begin, end - begin);
}

void ParseVersionAndBaseband(ProductInfo* product_info) {
    if (product_info == nullptr || product_info->infostr.empty()) {
        return;
    }

    const auto first_space = product_info->infostr.find(' ');
    if (first_space == std::string::npos) {
        if (product_info->model.empty()) {
            product_info->model = product_info->infostr;
        }
        return;
    }

    if (product_info->model.empty()) {
        product_info->model = product_info->infostr.substr(0, first_space);
    }

    const auto left_paren = product_info->infostr.find('(', first_space + 1);
    if (left_paren == std::string::npos) {
        product_info->version = product_info->infostr.substr(first_space + 1);
        return;
    }

    product_info->version =
            product_info->infostr.substr(first_space + 1, left_paren - first_space - 1);

    const auto right_paren = product_info->infostr.find(')', left_paren + 1);
    if (right_paren != std::string::npos) {
        product_info->baseband =
                product_info->infostr.substr(left_paren + 1, right_paren - left_paren - 1);
    }
}

}  // namespace

ProductInfo ReadProductInfo() {
    ProductInfo product_info = {};

    std::ifstream input(kOemInfoPath, std::ios::in | std::ios::binary);
    if (!input) {
        LOG(ERROR) << "Unable to open: " << kOemInfoPath << ", error: " << strerror(errno);
        return product_info;
    }

    std::vector<char> binary((std::istreambuf_iterator<char>(input)),
                             std::istreambuf_iterator<char>());
    const size_t content_length = binary.size();

    // OEMINFO sizes differ by generation and device. Version 6 images used by
    // Kirin 710 devices are at most 64 MiB, while the EVR Kirin 980 version 8
    // image is 96 MiB and contains two redundant 48 MiB banks. Accept either
    // layout as long as the image is large enough for a header and aligned to
    // the OEMINFO scan step; do not require one fixed partition size.
    if (content_length < kOemInfoHeaderSize ||
        (content_length % kOemInfoScanStep) != 0) {
        LOG(ERROR) << "Invalid OEMINFO size: " << content_length;
        return product_info;
    }

    uint32_t detected_version = 0;
    std::map<uint32_t, OemInfoRecord> latest_records;

    for (size_t offset = 0; offset + kOemInfoHeaderSize <= content_length;
         offset += kOemInfoScanStep) {
        if (std::memcmp(binary.data() + offset, "OEM_INFO", 8) != 0) {
            continue;
        }

        uint32_t version_number = 0;
        uint32_t id = 0;
        uint32_t type = 0;
        uint32_t data_len = 0;
        uint32_t age = 0;

        std::memcpy(&version_number, binary.data() + offset + 8, sizeof(version_number));
        std::memcpy(&id, binary.data() + offset + 12, sizeof(id));
        std::memcpy(&type, binary.data() + offset + 16, sizeof(type));
        std::memcpy(&data_len, binary.data() + offset + 20, sizeof(data_len));
        std::memcpy(&age, binary.data() + offset + 24, sizeof(age));

        if (detected_version == 0) {
            detected_version = version_number;
        } else if (detected_version != version_number) {
            LOG(WARNING) << "Ignoring OEMINFO record with unexpected version "
                         << version_number << " at offset 0x" << std::hex << offset;
            continue;
        }

        if (!IsWantedRecord(version_number, id)) {
            continue;
        }

        const size_t data_offset = offset + kOemInfoHeaderSize;
        if (data_offset > content_length || data_len > content_length - data_offset) {
            LOG(WARNING) << "Invalid OEMINFO record length for id 0x" << std::hex << id
                         << " at offset 0x" << offset;
            continue;
        }

        if (data_len > kMaxTextRecordSize) {
            LOG(WARNING) << "OEMINFO text record too large for id 0x" << std::hex << id
                         << ": " << std::dec << data_len;
            continue;
        }

        OemInfoRecord record;
        record.version = version_number;
        record.id = id;
        record.type = type;
        record.data_len = data_len;
        record.age = age;
        record.offset = offset;
        record.data.assign(binary.data() + data_offset, binary.data() + data_offset + data_len);

        const auto existing = latest_records.find(id);
        if (existing == latest_records.end() || IsNewerAge(age, existing->second.age)) {
            latest_records[id] = std::move(record);
        }
    }

    if (detected_version == 0) {
        LOG(ERROR) << "No OEM_INFO records found";
        return product_info;
    }

    const auto get_record_string = [&latest_records](uint32_t id) -> std::string {
        const auto it = latest_records.find(id);
        return it == latest_records.end() ? std::string{} : RecordToString(it->second);
    };

    if (detected_version == 6) {
        product_info.board = get_record_string(0x61);
        product_info.region = get_record_string(0x12);
        product_info.infostr = get_record_string(0x4e);
        product_info.marketname = get_record_string(0x81);
        product_info.model = get_record_string(0x5b);
    } else if (detected_version == 8) {
        product_info.infostr = get_record_string(0x5ec);
        product_info.model = get_record_string(0x5ee);
        product_info.region = ExtractQuotedValue(get_record_string(0x33), "C_version");

        // V8 does not expose the old v6 hardware-version/marketing-name IDs.
        // For EVR and similar Kirin 980 layouts, the model is the useful board variant.
        product_info.board = product_info.model;
        product_info.marketname = product_info.model;
    } else {
        LOG(ERROR) << "Unsupported OEMINFO version: " << detected_version;
        return product_info;
    }

    ParseVersionAndBaseband(&product_info);

    product_info.brand = "HUAWEI";

    const auto dash = product_info.model.find('-');
    const std::string model_prefix = product_info.model.substr(0, dash);
    if (!model_prefix.empty()) {
        switch (detected_version) {
            case 6:
                // Older Huawei OEMINFO layout, used by Kirin 710 devices:
                // POT-LX1 -> HWPOT-H
                product_info.device = "HW" + model_prefix + "-H";
                break;

            case 8:
                // Newer Huawei OEMINFO layout, used by Kirin 980 devices:
                // EVR-L29 -> HWEVR
                product_info.device = "HW" + model_prefix;
                break;

            default:
                // Unsupported versions have already returned above.
                LOG(ERROR) << "Unable to construct device for OEMINFO v"
                           << detected_version;
                break;
        }

        product_info.camera = model_prefix;
        product_info.devicehw = model_prefix;
    }

    LOG(INFO) << "OEMINFO v" << detected_version << ": model=" << product_info.model
              << ", region=" << product_info.region
              << ", build=" << product_info.infostr;

    return product_info;
}

void load_variants() {

    LOG(ERROR) << "load_variants";

    ProductInfo product_info = ReadProductInfo();


    // TODO
    std::string brand;
    
    // Load the phone model dynamically from the oeminfo partition.
    if (!product_info.model.empty()) {
        LOG(INFO) << "Found HW product info (1/2): " << product_info.devicehw << " - " << product_info.region << " - " << product_info.model;
        LOG(INFO) << "Found HW product info (2/2): " << product_info.infostr  << " - " << product_info.marketname;
        LOG(INFO) << "Extract product info: " << product_info.device << " - " << product_info.version << " - "  << product_info.board << " - " << product_info.baseband;

	set_ro_build_prop("brand", product_info.brand, true);
	set_ro_build_prop("device", product_info.device, true);
	set_ro_build_prop("model", product_info.model, true);
	set_ro_build_prop("name", product_info.model, true);
	
	property_override("ro.product.camera_product", product_info.camera, true);

	//env CUST_POLICY_DIRS not set
	if (getenv("CUST_POLICY_DIRS") == nullptr) {
	    std::string s = "/vendor/etc:/odm/etc:/product/etc:/data/cota:/odm/hw_odm/";
            std::string policy = s + product_info.board;
            setenv("CUST_POLICY_DIRS", policy.c_str(), 0 /*override*/);
            LOG(INFO) << "New CUST_POLICY_DIRS=" << getenv("CUST_POLICY_DIRS");
        }
    } else {
        LOG(ERROR) << "Unable to parse product information!";
    }
}

