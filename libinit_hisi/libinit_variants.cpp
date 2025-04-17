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
    std::string infostr;
    std::string HWRegion;
    std::string HWVersion;

    // result of the parse
    std::string device;
    std::string model;
    std::string version;    
    std::string baseband;
    std::string board;
       
    // TODO
    std::string brand;
    std::string marketname;
};

constexpr const char* kOemInfoPath = "/dev/block/by-name/oeminfo";

ProductInfo ParseProductInfo(const std::string& product_info_str) {
    ProductInfo product_info;
    std::istringstream iss(product_info_str);

    // Extract the model (i.e. "PRA-LX1").
    std::getline(iss, product_info.model, ' ');

    // Extract the version (i.e. "9.1.0.311") including parentheses if present.
    std::getline(iss, product_info.version, '(');

    // Remove trailing whitespace.
    android::base::Trim(product_info.version);

    // If version is enclosed in parentheses, extract the region_type.
    if (!product_info.version.empty() && product_info.version.back() == ')') {
        product_info.version.pop_back();

        // Extract the region_type (i.e. "C185E3R2P1").
        std::getline(iss, product_info.region_type, ')');

        // Trim leading and trailing whitespaces from region_type.
        android::base::Trim(product_info.region_type);
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


ProductInfo ReadProductInfo() {
    std::vector<char> HW_Version(8);
    std::vector<char> HW_Region(6);
    std::vector<char> SW_Version(128);
    ProductInfo product_info = {};


    std::ifstream input(kOemInfoPath, std::ios::in | std::ios::binary);
    if (!input) {
        LOG(ERROR) << "Unable to open: " << kOemInfoPath << ", error: " << strerror(errno);
        return product_info;
    }

    std::vector<char> binary((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    size_t content_length = binary.size();
    size_t content_startbyte = 0;

    if (content_length != 67108864) {
        LOG(ERROR) << "Wrong filesize";
        return product_info;
    }

    while (content_startbyte < content_length) {
        char header[8];
        uint32_t version_number, id, type, data_len, age;
        std::memcpy(header, binary.data() + content_startbyte, 8);
        std::memcpy(&version_number, binary.data() + content_startbyte + 8, sizeof(version_number));
        std::memcpy(&id, binary.data() + content_startbyte + 12, sizeof(id));
        std::memcpy(&type, binary.data() + content_startbyte + 16, sizeof(type));
        std::memcpy(&data_len, binary.data() + content_startbyte + 20, sizeof(data_len));
        std::memcpy(&age, binary.data() + content_startbyte + 24, sizeof(age));

        if (std::memcmp(header, "OEM_INFO", 8) == 0) {
            static uint32_t version = 0;
            if (version == 0) {
                version = version_number;
            }
            if (version != version_number) {
               LOG(ERROR) <<  "version number changed during parsing! wtf";
        	return product_info;
            }
            if (version_number == 8) {
                // Handle version 8 specific logic
            }
            if (version_number == 6) {
                // Handle version 6 specific logic
            }
            if (id == 0x61) {
                std::memcpy(HW_Version.data(), binary.data() + content_startbyte + 0x200, data_len);
            }
            if (id == 0x12) {
                std::memcpy(HW_Region.data(), binary.data() + content_startbyte + 0x200, data_len);
            }
            if (id == 0x4e) {
                std::memcpy(SW_Version.data(), binary.data() + content_startbyte + 0x200, data_len);
            }

            //std::string fileout = std::to_string(id) + "-" + std::to_string(type) + "-" + std::to_string(age) + "-" + std::to_string(content_startbyte);
            //std::cout << "hdr:" << std::string(header, 8) << " age:" << std::hex << age << " id:" << std::hex << id << " " << elements[version][id] << std::endl;
        }
        content_startbyte += 0x400; // Move to the next header
    }

    std::string versionfull = std::string(SW_Version.begin(), SW_Version.end());

    product_info.infostr = versionfull.substr(0, versionfull.find('\0'));
    product_info.HWRegion = std::string(HW_Region.begin(), HW_Region.end());
    product_info.HWVersion = std::string(HW_Version.begin(), HW_Version.end());
    
    std::istringstream iss(product_info.infostr);

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
    
    // Extract the brand
    product_info.brand = "HUAWEI";
    
    // Extract the board
    product_info.board = product_info.HWVersion;
    
    // Extract the device (i.e. "HWPOT")
    std::istringstream iss1(product_info.infostr);
    std::string tempmodel;
    
    std::getline(iss1, tempmodel, '-');
    product_info.device = "HW" + tempmodel + "-H";
    
    // TODO    
    product_info.marketname =  "";


    return product_info;
}

void load_variants() {

    LOG(ERROR) << "load_variants";

    ProductInfo product_info = ReadProductInfo();

    // Load the phone model dynamically from the oeminfo partition.
    if (!product_info.model.empty()) {
        LOG(INFO) << "Found HW product info: " << product_info.HWVersion << " - " << product_info.HWRegion << " - " << product_info.infostr;
        LOG(INFO) << "Extract product info: " << product_info.device << " - " << product_info.model << " - "  << product_info.board << " - " << product_info.baseband;

	set_ro_build_prop("brand", product_info.brand, true);
	set_ro_build_prop("device", product_info.device, true);
	set_ro_build_prop("model", product_info.model, true);
	
	property_override("ro.product.board", product_info.board, true);

    } else {
        LOG(ERROR) << "Unable to parse product information!";
    }
}
