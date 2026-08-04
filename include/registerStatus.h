#pragma once
#include <cstdint>
#include "ROB.h"

const int NO_PRODUCER = -1; // 没有指令在写这个寄存器
const int NUM_REGISTERS = 32;
// const int NO_TAG = -1;

class RegisterStatus{
private:
    int producer_new[NUM_REGISTERS]; // tag是ROB队列中的下标
    int producer_old[NUM_REGISTERS];
    long producer_seq_new[NUM_REGISTERS]; // 当前生产者指令的global_seq
    long producer_seq_old[NUM_REGISTERS];
    static int age_distance(int tag, int rob_head);
public:
    RegisterStatus();
    int get_producer(int r) const;
    // void set_producer(int r, int tag);
    // void clear_if_match(int r, int tag);
    void step(int broadcast_tag, int issue_reg, int issue_tag, 
              int flush_tag, long flush_seq, long issue_global_seq); // issue_reg/issue_tag: 发布的新指令写哪个寄存器、标签是什么
    void sync();
};