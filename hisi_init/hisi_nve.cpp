/*
 * Copyright (C) 2023 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "hisi_nve"

#include "include/hisi_nve.h"

#include <android-base/logging.h>
#include <android-base/properties.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace {

constexpr const char* kWifiMacPath = "/data/vendor/wifi/macwlan";
constexpr const char* kBtMacPath = "/data/vendor/bluedroid/macbt";

/*
 * Fallbacks are used only when neither NVE nor existing data files contain
 * valid addresses. They are locally administered, unicast addresses.
 */
constexpr const char* kDefaultWifiMac = "02:00:00:00:00:51";
constexpr const char* kDefaultBtMac = "02:00:00:00:00:52";

bool path_exists_readable(const std::string& path) {
    return access(path.c_str(), R_OK) == 0;
}

bool ensure_parent_dir(const std::string& path) {
    const auto pos = path.find_last_of('/');
    if (pos == std::string::npos || pos == 0)
        return true;

    const std::string dir = path.substr(0, pos);
    std::string partial;

    for (size_t i = 1; i < dir.size(); ++i) {
        if (dir[i] != '/')
            continue;

        partial = dir.substr(0, i);
        if (!partial.empty() && mkdir(partial.c_str(), 0771) < 0 && errno != EEXIST) {
            LOG(WARNING) << "Unable to create directory " << partial << ": " << strerror(errno);
            return false;
        }
    }

    if (mkdir(dir.c_str(), 0771) < 0 && errno != EEXIST) {
        LOG(WARNING) << "Unable to create directory " << dir << ": " << strerror(errno);
        return false;
    }

    return true;
}

bool write_text_file(const std::string& path, const std::string& value) {
    if (!ensure_parent_dir(path))
        return false;

    std::ofstream file(path, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        LOG(ERROR) << "Unable to open " << path << " for writing";
        return false;
    }

    file << value << '\n';

    if (!file.good()) {
        LOG(ERROR) << "Unable to write " << path;
        return false;
    }

    LOG(INFO) << "Wrote " << path << " = " << value;
    return true;
}

std::string trim_ascii(std::string s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front())))
        s.erase(s.begin());

    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))
        s.pop_back();

    return s;
}

std::string read_text_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        return "";

    std::string value;
    std::getline(file, value);
    return trim_ascii(value);
}

bool is_hex_char(char c) {
    return std::isxdigit(static_cast<unsigned char>(c)) != 0;
}

std::string uppercase(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return s;
}

std::string normalize_raw_mac(std::string raw) {
    /*
     * Huawei NVE may store MACs either as 12 ASCII hex chars:
     *   E4FDA172EA51
     * or, on broken/empty entries, as 12 zero bytes.
     */
    if (raw.size() < 12)
        return "";

    raw = raw.substr(0, 12);

    for (char c : raw) {
        if (!is_hex_char(c))
            return "";
    }

    return uppercase(raw);
}

std::string normalize_formatted_mac(std::string mac) {
    mac = trim_ascii(mac);

    if (mac.size() != 17)
        return "";

    for (size_t i = 0; i < mac.size(); ++i) {
        if ((i + 1) % 3 == 0) {
            if (mac[i] != ':')
                return "";
        } else if (!is_hex_char(mac[i])) {
            return "";
        }
    }

    return uppercase(mac);
}

bool is_bad_mac_no_colons(const std::string& raw) {
    return raw == "000000000000" || raw == "FFFFFFFFFFFF";
}

bool is_bad_formatted_mac(const std::string& mac) {
    return mac == "00:00:00:00:00:00" || mac == "FF:FF:FF:FF:FF:FF";
}

bool is_multicast_mac(const std::string& mac) {
    if (mac.size() != 17)
        return true;

    unsigned int first_byte = 0;
    std::stringstream ss;
    ss << std::hex << mac.substr(0, 2);
    ss >> first_byte;

    return (first_byte & 0x01) != 0;
}

bool is_valid_formatted_mac(const std::string& mac) {
    const std::string normalized = normalize_formatted_mac(mac);
    if (normalized.empty())
        return false;

    if (is_bad_formatted_mac(normalized))
        return false;

    if (is_multicast_mac(normalized))
        return false;

    return true;
}

std::string format_raw_mac(const std::string& raw) {
    if (raw.size() != 12)
        return "";

    return raw.substr(0, 2) + ":" +
           raw.substr(2, 2) + ":" +
           raw.substr(4, 2) + ":" +
           raw.substr(6, 2) + ":" +
           raw.substr(8, 2) + ":" +
           raw.substr(10, 2);
}

std::string parse_nve_mac(const std::string& raw_from_nve) {
    const std::string raw = normalize_raw_mac(raw_from_nve);
    if (raw.empty()) {
        LOG(WARNING) << "Invalid NVE MAC: not 12 ASCII hex chars";
        return "";
    }

    if (is_bad_mac_no_colons(raw)) {
        LOG(WARNING) << "Invalid NVE MAC: zero/ff address";
        return "";
    }

    const std::string mac = format_raw_mac(raw);
    if (!is_valid_formatted_mac(mac)) {
        LOG(WARNING) << "Invalid NVE MAC after formatting: " << mac;
        return "";
    }

    return mac;
}

std::string adjust_last_byte(std::string mac, int delta) {
    mac = normalize_formatted_mac(mac);
    if (!is_valid_formatted_mac(mac))
        return "";

    unsigned int last_byte = 0;
    std::stringstream ss;
    ss << std::hex << mac.substr(15, 2);
    ss >> last_byte;

    last_byte = (last_byte + delta) & 0xff;

    /*
     * Avoid deriving the same trailing 00 edge case from FF/01 in a way that
     * could collide with reserved/default patterns.
     */
    if (last_byte == 0x00)
        last_byte = (delta >= 0) ? 0x01 : 0xff;

    std::ostringstream out;
    out << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << last_byte;

    mac.replace(15, 2, out.str());

    if (!is_valid_formatted_mac(mac))
        return "";

    return mac;
}

std::string derive_bt_mac_from_wifi(const std::string& wifi_mac) {
    return adjust_last_byte(wifi_mac, +1);
}

std::string derive_wifi_mac_from_bt(const std::string& bt_mac) {
    return adjust_last_byte(bt_mac, -1);
}

std::string load_nve_path() {
    for (const auto& path : kNvePaths) {
        if (path_exists_readable(path)) {
            LOG(INFO) << "Using NVE path " << path;
            return path;
        }
    }

    LOG(WARNING) << "No readable NVE path found";
    return "";
}

std::streampos find_start_offset(std::ifstream& file) {
    if (!file.is_open())
        return std::streampos(-1);

    file.seekg(0, std::ios::end);
    const std::streampos file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (file_size <= 0)
        return std::streampos(-1);

    constexpr size_t kBufferSize = 4096;
    constexpr size_t kNeedleSize = 7;
    std::array<char, kBufferSize> buffer{};

    for (std::streampos i = 0; i < file_size; i += static_cast<std::streamoff>(kBufferSize - kNeedleSize)) {
        file.seekg(i);
        file.read(buffer.data(), buffer.size());

        const std::streamsize bytes_read = file.gcount();
        if (bytes_read < static_cast<std::streamsize>(kNeedleSize))
            break;

        for (std::streamsize j = 0; j <= bytes_read - static_cast<std::streamsize>(kNeedleSize); ++j) {
            if (std::memcmp(buffer.data() + j, "SWVERSI", kNeedleSize) == 0)
                return i + static_cast<std::streamoff>(j);
        }
    }

    return std::streampos(-1);
}

std::string nve_read(const std::string& name, const std::string& path) {
    if (name.empty() || name.size() > sizeof(nv_item::nv_name)) {
        LOG(ERROR) << "Invalid NVE item name " << name;
        return "";
    }

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        LOG(ERROR) << "Unable to open " << path;
        return "";
    }

    const std::streampos swversi_offset = find_start_offset(file);
    if (swversi_offset == std::streampos(-1)) {
        LOG(ERROR) << "Unable to find SWVERSI marker in NVE partition";
        return "";
    }

    if (swversi_offset < static_cast<std::streamoff>(4)) {
        LOG(ERROR) << "Invalid SWVERSI offset in NVE partition";
        return "";
    }

    file.clear();
    file.seekg(swversi_offset - static_cast<std::streamoff>(4));

    nv_item entry{};
    while (file.read(reinterpret_cast<char*>(&entry), sizeof(entry))) {
        if (entry.valid_size > sizeof(entry.nv_data)) {
            LOG(WARNING) << "Skipping NVE item with invalid size " << entry.valid_size;
            continue;
        }

        if (std::memcmp(entry.nv_name, name.c_str(), name.size()) != 0)
            continue;

        LOG(INFO) << "Found NVE item " << name << " size " << entry.valid_size;
        return std::string(entry.nv_data, entry.valid_size);
    }

    LOG(WARNING) << "NVE item " << name << " not found";
    return "";
}

std::string read_mac_from_nve(const std::string& nve_name, const std::string& nve_path) {
    if (nve_path.empty())
        return "";

    const std::string raw = nve_read(nve_name, nve_path);
    if (raw.empty())
        return "";

    return parse_nve_mac(raw);
}

std::string read_existing_mac_file(const std::string& path) {
    const std::string mac = normalize_formatted_mac(read_text_file(path));
    if (!is_valid_formatted_mac(mac))
        return "";

    LOG(INFO) << "Using existing valid MAC from " << path << ": " << mac;
    return mac;
}

}  // namespace

int LoadNveProperties() {
    const std::string nve_path = load_nve_path();

    /*
     * Read both NVE entries first. Do not assume either one exists.
     */
    std::string nve_wifi_mac = read_mac_from_nve("MACWLAN", nve_path);
    std::string nve_bt_mac = read_mac_from_nve("MACBT", nve_path);

    /*
     * Existing files are second priority, useful when NVE is missing on Board
     * Software or when a previous boot already generated stable fallback MACs.
     */
    std::string file_wifi_mac = read_existing_mac_file(kWifiMacPath);
    std::string file_bt_mac = read_existing_mac_file(kBtMacPath);

    std::string wifi_mac;
    std::string bt_mac;

    if (!nve_wifi_mac.empty()) {
        wifi_mac = nve_wifi_mac;
        LOG(INFO) << "Selected Wi-Fi MAC from NVE: " << wifi_mac;
    }

    if (!nve_bt_mac.empty()) {
        bt_mac = nve_bt_mac;
        LOG(INFO) << "Selected Bluetooth MAC from NVE: " << bt_mac;
    }

    /*
     * Cross-derive missing side from the valid side.
     *
     * Case A: MACWLAN exists, MACBT missing:
     *   BT = Wi-Fi + 1
     *
     * Case B: MACBT exists, MACWLAN missing:
     *   Wi-Fi = BT - 1
     */
    if (wifi_mac.empty() && !bt_mac.empty()) {
        wifi_mac = derive_wifi_mac_from_bt(bt_mac);
        if (!wifi_mac.empty())
            LOG(WARNING) << "Derived Wi-Fi MAC from Bluetooth MAC: " << wifi_mac;
    }

    if (bt_mac.empty() && !wifi_mac.empty()) {
        bt_mac = derive_bt_mac_from_wifi(wifi_mac);
        if (!bt_mac.empty())
            LOG(WARNING) << "Derived Bluetooth MAC from Wi-Fi MAC: " << bt_mac;
    }

    /*
     * If NVE could not provide one side and cross-derive was impossible, keep
     * already valid files before falling back to defaults.
     */
    if (wifi_mac.empty() && !file_wifi_mac.empty()) {
        wifi_mac = file_wifi_mac;
        LOG(INFO) << "Selected existing Wi-Fi MAC file: " << wifi_mac;
    }

    if (bt_mac.empty() && !file_bt_mac.empty()) {
        bt_mac = file_bt_mac;
        LOG(INFO) << "Selected existing Bluetooth MAC file: " << bt_mac;
    }

    /*
     * Try cross-derive again after considering existing files.
     */
    if (wifi_mac.empty() && !bt_mac.empty()) {
        wifi_mac = derive_wifi_mac_from_bt(bt_mac);
        if (!wifi_mac.empty())
            LOG(WARNING) << "Derived Wi-Fi MAC from existing Bluetooth MAC: " << wifi_mac;
    }

    if (bt_mac.empty() && !wifi_mac.empty()) {
        bt_mac = derive_bt_mac_from_wifi(wifi_mac);
        if (!bt_mac.empty())
            LOG(WARNING) << "Derived Bluetooth MAC from existing Wi-Fi MAC: " << bt_mac;
    }

    /*
     * Absolute final fallback: deterministic local-administered addresses.
     */
    if (wifi_mac.empty()) {
        wifi_mac = kDefaultWifiMac;
        LOG(WARNING) << "Falling back to default Wi-Fi MAC: " << wifi_mac;
    }

    if (bt_mac.empty()) {
        bt_mac = kDefaultBtMac;
        LOG(WARNING) << "Falling back to default Bluetooth MAC: " << bt_mac;
    }

    /*
     * Avoid collision if a bad source somehow produced the same address.
     */
    if (normalize_formatted_mac(wifi_mac) == normalize_formatted_mac(bt_mac)) {
        const std::string derived_bt = derive_bt_mac_from_wifi(wifi_mac);
        if (!derived_bt.empty()) {
            bt_mac = derived_bt;
            LOG(WARNING) << "Bluetooth MAC matched Wi-Fi MAC, adjusted to: " << bt_mac;
        } else {
            bt_mac = kDefaultBtMac;
            LOG(WARNING) << "Bluetooth MAC matched Wi-Fi MAC, using default: " << bt_mac;
        }
    }

    if (!write_text_file(kWifiMacPath, wifi_mac))
        LOG(ERROR) << "Failed to write Wi-Fi MAC";

    if (!write_text_file(kBtMacPath, bt_mac))
        LOG(ERROR) << "Failed to write Bluetooth MAC";

    android::base::SetProperty(kPropMacsAreReady, "1");
    return 0;
}

void load_hisi_nve() {
    if (LoadNveProperties() < 0)
        LOG(WARNING) << "Unable to load NVE properties";
}
