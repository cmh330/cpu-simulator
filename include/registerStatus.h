#pragma once
#include <cstdint>

const int NO_PRODUCER = -1; // 没有指令在写这个寄存器
const int NUM_REGISTERS = 32;

class RegisterStatus{
private:
    int producer_tag[NUM_REGISTERS]; // tag是ROB队列中的下标
public:
    RegisterStatus();
    int get_producer(int r) const;
    void set_producer(int r, int tag);
    void clear_if_match(int r, int tag);
};