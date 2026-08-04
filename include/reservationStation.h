#pragma once
#include <cstdint>
#include "alu.h"
#include "CDB.h"
#include "ROB.h"

const int NUM_RS = 10;
const int NO_TAG_RS = -1;

struct RS{
    bool available = true;
    AluOp op;
    int32_t vj = 0, vk = 0;
    int qj = NO_TAG_RS, qk = NO_TAG_RS;
    int dest = NO_TAG_RS; // 交给谁
    // qj, qk, dest都是ROB队列的下标
    bool is_branch = false;
    long global_seq = -1;
};

// 不走cdb（不需要写寄存器），直接告诉rob
struct BranchResult {
    int tag = NO_TAG_RS;
    bool actual_jump = false;
};

class RSGroup{
private:
    RS rs_new[NUM_RS];
    RS rs_old[NUM_RS];
    static int age_distance(int tag, int rob_head);
public:
    RSGroup();
    // int find_available() const; // -1: no available
    // bool is_ready(int idx) const; // 两个标签都清空
    // void issue(int idx, AluOp op, int32_t vj, int32_t vk, int qj, int qk, int dest, bool is_branch); // issue时分配
    // void clear(int idx); // 开始执行后释放
    // void update_cdb_broadcast(int tag, int32_t value);
    // bool is_occupied(int idx) const;
    // int get_dest(int idx) const;
    bool is_full() const;
    CdbBroadcast get_broadcast() const; // cpu.h中调用，返回基于old的广播候选，选已就绪中下标最小的
    BranchResult get_branch_result() const; 
    void step(CdbBroadcast cdb, AluOp issue_op, int32_t issue_vj, int issue_qj,
              int32_t issue_vk, int issue_qk, int issue_dest, bool issue_is_branch, 
              int flush_tag, long flush_seq, long issue_global_seq);
    void sync();
};