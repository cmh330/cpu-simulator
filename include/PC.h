#pragma once
#include <cstdint>

class PC {
private:
    uint32_t pc_old;
    uint32_t pc_new;
public:
    PC();
    uint32_t get() const;
    void step(bool mispredict, uint32_t correct_pc, bool issued, uint32_t predicted_next_pc);
    void sync();
};