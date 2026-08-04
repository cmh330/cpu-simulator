#pragma once
#include <cstdint>
#include "CDB.h"

const int NO_TAG = -1;
const int ROB_SIZE = 20;

enum class RobType {
    REGULAR, STORE, BRANCH
};

struct Rob {
    bool available = true;
    int dest_reg = 0;
    int32_t value = 0; // 算出来还没生效的结果
    bool ready = false;
    uint32_t pc;
    uint32_t target; // pc + imm
    bool predict_jump = false; // 预测是否跳转
    bool actual_jump = false; // 实际是否跳转
    RobType type = RobType::REGULAR;
    long global_seq = -1;
};

class ROB {
private:
    Rob rob_old[ROB_SIZE];
    Rob rob_new[ROB_SIZE];
    int head_old, tail_old, count_old; // tail指向队尾下一个空位
    int head_new, tail_new, count_new;
    static int age_distance(int tag, int rob_head);

public:
    ROB();
    bool is_empty() const;
    bool is_full() const;
    // int allocate(RobType type, int dest_reg, uint32_t pc, bool predict_jump); // 发布时用，返回下标(tag), 调用前用is_full()检查
    // void write_result(int tag, int32_t value); // cdb广播后，写结果，标记ready
    // void write_branch_result(int tag, bool actual_jump);
    int head_tag() const;
    int tail_tag() const;
    // const Rob& get_head() const;
    const Rob& get(int tag) const;
    // bool can_commit_head() const;
    // void commit_head();
    // void flush(int tag); // tag之后全部清空，tag本身不清

    struct CommitResult {
        RobType type = RobType::REGULAR;
        int tag = NO_TAG; // -1: 队首没有东西要提交
        int dest_reg = 0; // 写进哪
        int32_t value = 0;
        bool mispredict = false;
        uint32_t correct_pc = 0;
    };

    CommitResult get_commit() const;
    void step(CdbBroadcast cdb, int branch_tag, bool branch_jump, int store_ready_tag,
              bool issue_valid, RobType issue_type, int issue_dest_reg,
              uint32_t issue_pc, uint32_t issue_target, bool issue_predict_jump, int flush_tag, long issue_global_seq);
    void sync();
};