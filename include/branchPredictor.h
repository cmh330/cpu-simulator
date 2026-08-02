#pragma once
#include <cstdint>

const int B_SIZE = 256;

enum class BranchState : uint8_t {
    STRONG_JUMP, WEAK_JUMP, STRONG_NOT_JUMP, WEAK_NOT_JUMP
};

class BranchPredictor {
private:
    BranchState bp_old[B_SIZE];
    BranchState bp_new[B_SIZE];
    long total_old = 0, total_new = 0; // 一共预测了多少次
    long correct_old = 0, correct_new = 0;
    int get_index(uint32_t pc) const;

public:
    BranchPredictor();
    bool predict(uint32_t pc) const; // true: 跳转
    // void update(uint32_t pc, bool actual_jump);
    // void record(bool predict_jump, bool actual_jump);
    double accuracy() const;
    void step(bool commit_branch_valid, uint32_t commit_pc, bool commit_predicted, bool commit_actual);
    void sync();
};