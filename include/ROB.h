#pragma once
#include <cstdint>

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
    bool predict_jump = false; // 预测是否跳转
    bool actual_jump = false; // 实际是否跳转
    RobType type = RobType::REGULAR;
};

class ROB {
private:
    Rob rob[ROB_SIZE];
    int head, tail, count; // tail指向队尾下一个空位

public:
    ROB();
    bool is_empty() const;
    bool is_full() const;
    int allocate(RobType type, int dest_reg, uint32_t pc, bool predict_jump); // 发布时用，返回下标(tag), 调用前用is_full()检查
    void write_result(int tag, int32_t value); // cdb广播后，写结果，标记ready
    void write_branch_result(int tag, bool actual_jump);
    int head_tag() const;
    const Rob& get_head() const;
    const Rob& get(int tag) const;
    bool can_commit_head() const;
    void commit_head();
    void flush(int tag); // tag之后全部清空，tag本身不清
};