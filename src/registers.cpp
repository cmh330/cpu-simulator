#include "../include/registers.h"

Register::Register() {
    for (int i = 0; i < NUM_REGS; ++i) { 
        regs_old[i] = 0; 
        regs_new[i] = 0; 
    }
}

int32_t Register::get(int r) const {
    return regs_old[r];
}

void Register::step(bool commit_regular_valid, int dest_reg, int32_t value) {
    if (!commit_regular_valid || dest_reg == 0) return;
    for (int i = 0; i < NUM_REGS; ++i) regs_new[i] = regs_old[i];
    regs_new[dest_reg] = value;
}

void Register::sync() {
    for (int i = 0; i < NUM_REGS; ++i) { 
        regs_old[i] = regs_new[i]; 
    }
}