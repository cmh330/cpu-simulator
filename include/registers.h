#pragma once
#include <cstdint>

const int NUM_REGS = 32;

class Register {
private:
    int32_t regs_new[NUM_REGS];
    int32_t regs_old[NUM_REGS];
public:
    Register();
    int32_t get(int r) const;
    void step(bool commit_regular_valid, int dest_reg, int32_t value); // commit_regular_valid: 是否commit了一条REGULAR指令
    void sync();
};