#pragma once
#include <vector>
#include <cstdint>

// Build a SET_LEFT (0x05) or SET_RIGHT (0x06) temperature packet
inline std::vector<uint8_t> alpicool_set_temp_packet(uint8_t cmd, int temp) {
    uint8_t t = (uint8_t)(temp & 0xFF);
    std::vector<uint8_t> pkt = {0xfe, 0xfe, 0x04, cmd, t, 0x00, 0x00};
    uint16_t cs = 0;
    for (size_t i = 0; i < 5; i++) cs += pkt[i];
    pkt[5] = (cs >> 8) & 0xFF;
    pkt[6] = cs & 0xFF;
    return pkt;
}

// Build a SET (0x02) packet from current device state.
// tc_* are factory calibration offsets — pass them as read from the device.
inline std::vector<uint8_t> alpicool_set_packet(
    bool locked, bool powered_on, int run_mode, int bat_saver,
    int left_target, int temp_max, int temp_min, int left_ret_diff,
    int start_delay,
    int left_tc_hot, int left_tc_mid, int left_tc_cold, int left_tc_halt,
    bool dual_zone,
    int right_target, int right_ret_diff,
    int right_tc_hot, int right_tc_mid, int right_tc_cold, int right_tc_halt
) {
    auto u8 = [](int x) -> uint8_t { return (uint8_t)(x & 0xFF); };

    std::vector<uint8_t> data = {
        u8(locked),       // [0]  lock state
        u8(powered_on),   // [1]  power
        u8(run_mode),     // [2]  0=fridge/max, 1=eco/freezer
        u8(bat_saver),    // [3]  0=low, 1=mid, 2=high
        u8(left_target),  // [4]  left zone target temperature
        u8(temp_max),     // [5]  upper temp limit
        u8(temp_min),     // [6]  lower temp limit
        u8(left_ret_diff),// [7]  hysteresis
        u8(start_delay),  // [8]  compressor start delay (minutes)
        0x00,             // [9]  unit (0=Celsius)
        u8(left_tc_hot),  // [10] calibration offsets (factory set)
        u8(left_tc_mid),  // [11]
        u8(left_tc_cold), // [12]
        u8(left_tc_halt), // [13]
    };

    if (dual_zone) {
        std::vector<uint8_t> right_data = {
            u8(right_target),  // [14]
            0x00, 0x00,        // [15-16] unknown
            u8(right_ret_diff),// [17]
            u8(right_tc_hot),  // [18]
            u8(right_tc_mid),  // [19]
            u8(right_tc_cold), // [20]
            u8(right_tc_halt), // [21]
            0x00, 0x00, 0x00,  // [22-24] unknown
        };
        data.insert(data.end(), right_data.begin(), right_data.end());
    }

    // Packet: 0xfe 0xfe + length_byte + cmd(0x02) + data + checksum(2)
    uint8_t cmd = 0x02;
    uint8_t len_byte = (uint8_t)(1 + data.size() + 2); // cmd + data + checksum
    std::vector<uint8_t> pkt = {0xfe, 0xfe, len_byte, cmd};
    pkt.insert(pkt.end(), data.begin(), data.end());

    uint16_t cs = 0;
    for (auto b : pkt) cs += b;
    pkt.push_back((cs >> 8) & 0xFF);
    pkt.push_back(cs & 0xFF);

    return pkt;
}
