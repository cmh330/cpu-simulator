#pragma once
#include <cstdint>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

const uint32_t STORAGE_SIZE = 1u << 20;  // 单位为byte

class Storage{
private:
    uint8_t bytes[STORAGE_SIZE];
public:
    Storage();
    void load(std::istream& input_stream);


    uint8_t read_byte(uint32_t addr) const;
    void write_byte(uint32_t addr, uint8_t value);

    uint16_t read_half_word(uint32_t addr) const;
    void write_half_word(uint32_t addr, uint16_t value);

    uint32_t read_word(uint32_t addr) const;
    void write_word(uint32_t addr, uint32_t value);
};