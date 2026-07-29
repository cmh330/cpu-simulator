#include "../include/storage.h"

Storage::Storage() {
    for (uint32_t i = 0; i < STORAGE_SIZE; ++i) {
        bytes[i] = 0;
    }
}

void Storage::load(std::istream& input_stream) {
    uint32_t current_addr = 0;
    std::string line;
    while(getline(input_stream, line)) {
        if (line.empty()) continue;
        if (line[0] == '@') {
            current_addr = static_cast<uint32_t>(std::stoul(line.substr(1), nullptr, 16));
            continue;
        }
        std::istringstream stream(line);
        std::string one_byte;
        while (stream >> one_byte) {
            uint8_t value = static_cast<uint8_t>(std::stoul(one_byte, nullptr, 16));
            bytes[current_addr++] = value;
        }
    }
}

uint8_t Storage::read_byte(uint32_t addr) const {
    return bytes[addr];
}
void Storage::write_byte(uint32_t addr, uint8_t value) {
    bytes[addr] = value;
}

uint16_t Storage::read_half_word(uint32_t addr) const {
    uint16_t byte0 = bytes[addr];
    uint16_t byte1 = bytes[addr + 1];
    return (byte0 | (byte1 << 8));
}
void Storage::write_half_word(uint32_t addr, uint16_t value) {
    bytes[addr] = static_cast<uint8_t>(value & 0xFF);
    bytes[addr + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
}

uint32_t Storage::read_word(uint32_t addr) const {
    uint32_t byte0 = bytes[addr];
    uint32_t byte1 = bytes[addr + 1];
    uint32_t byte2 = bytes[addr + 2];
    uint32_t byte3 = bytes[addr + 3];
    return (byte0 | (byte1 << 8) | (byte2 << 16) | (byte3 << 24));
}
void Storage::write_word(uint32_t addr, uint32_t value) {
    bytes[addr] = static_cast<uint8_t>(value & 0xFF);
    bytes[addr + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    bytes[addr + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
    bytes[addr + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
}