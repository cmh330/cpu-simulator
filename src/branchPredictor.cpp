#include "../include/branchPredictor.h"

int BranchPredictor::get_index(uint32_t pc) const {
    return static_cast<int>((pc >> 2) % B_SIZE);
}

BranchPredictor::BranchPredictor() : total_new(0), correct_new(0), total_old(0), correct_old(0) {
    for (int i = 0; i < B_SIZE; ++i) {
        bp_old[i] = BranchState::WEAK_NOT_JUMP;
        bp_new[i] = BranchState::WEAK_NOT_JUMP;
    }
}

bool BranchPredictor::predict(uint32_t pc) const {
    int idx = get_index(pc);
    return (bp_old[idx] == BranchState::STRONG_JUMP || bp_old[idx] == BranchState::WEAK_JUMP);
}

/*

void BranchPredictor::update(uint32_t pc, bool actual_jump) {
    int idx = get_index(pc);
    BranchState &state = bp[idx];
    if (actual_jump) {
        if (state == BranchState::STRONG_NOT_JUMP) state = BranchState::WEAK_NOT_JUMP;
        else if (state == BranchState::WEAK_NOT_JUMP) state = BranchState::WEAK_JUMP;
        else if (state == BranchState::WEAK_JUMP) state = BranchState::STRONG_JUMP;
    } else {
        if (state == BranchState::STRONG_JUMP) state = BranchState::WEAK_JUMP;
        else if (state == BranchState::WEAK_JUMP) state = BranchState::WEAK_NOT_JUMP;
        else if (state == BranchState::WEAK_NOT_JUMP) state = BranchState::STRONG_NOT_JUMP;
    }
}


void BranchPredictor::record(bool predict_jump, bool actual_jump) {
    ++total;
    if (predict_jump == actual_jump) ++correct;
}
*/

double BranchPredictor::accuracy() const {
    if (total_old == 0) return 0.0;
    return (static_cast<double>(correct_old) / static_cast<double>(total_old));
}

void BranchPredictor::sync() {
    for (int i = 0; i < B_SIZE; ++i) bp_old[i] = bp_new[i];
    total_old = total_new;
    correct_old = correct_new;
}

void BranchPredictor::step(bool commit_branch_valid, uint32_t commit_pc, bool commit_predicted, bool commit_actual) {
    if (!commit_branch_valid) return;

    for (int i = 0; i < B_SIZE; ++i) bp_new[i] = bp_old[i];
    total_new = total_old;
    correct_new = correct_old;

    if (commit_predicted == commit_actual) ++correct_new;
    ++total_new;

    int idx = get_index(commit_pc);
    BranchState s = bp_old[idx];
    if (commit_actual) {
        if (s == BranchState::STRONG_NOT_JUMP) bp_new[idx] = BranchState::WEAK_NOT_JUMP;
        else if (s == BranchState::WEAK_NOT_JUMP) bp_new[idx] = BranchState::WEAK_JUMP;
        else if (s == BranchState::WEAK_JUMP) bp_new[idx] = BranchState::STRONG_JUMP;
    } 
    else {
        if (s == BranchState::STRONG_JUMP) bp_new[idx] = BranchState::WEAK_JUMP;
        else if (s == BranchState::WEAK_JUMP) bp_new[idx] = BranchState::WEAK_NOT_JUMP;
        else if (s == BranchState::WEAK_NOT_JUMP) bp_new[idx] = BranchState::STRONG_NOT_JUMP;
    }
}