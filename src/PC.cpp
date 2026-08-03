#include "../include/PC.h"

PC::PC() : pc_old(0), pc_new(0) {}

void PC::sync() {
    pc_old = pc_new;
}

void PC::step(bool mispredict, uint32_t correct_pc, bool issued, uint32_t predicted_next_pc) {
    if (mispredict) {
        pc_new = correct_pc;
    }
    else if (issued) {
        pc_new = predicted_next_pc;
    }
    else {
        pc_new = pc_old;
    }
}

uint32_t PC::get() const {
    return pc_old;
}