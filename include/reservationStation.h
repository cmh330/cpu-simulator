#pragma once
#include <cstdint>
#include "alu.h"

const int NUM_RS = 10;
const int NO_TAG = -1;

struct RS{
    bool available = true;
    AluOp op;
    int32_t vj = 0, vk = 0;
    int qj = NO_TAG, qk = NO_TAG;
    int dest = NO_TAG; // 交给谁
    // qj, qk, dest都是ROB队列的下标
    bool is_branch = false;
};

class RSGroup{
private:
    RS rs[NUM_RS];
public:
    RSGroup();
    int find_available() const; // -1: no available
    bool is_ready(int idx) const; // 两个标签都清空
    void issue(int idx, AluOp op, int32_t vj, int32_t vk, int qj, int qk, int dest, bool is_branch); // issue时分配
    void clear(int idx); // 开始执行后释放
    void update_cdb_broadcast(int tag, int32_t value);
    bool is_occupied(int idx) const;
    int get_dest(int idx) const;
};