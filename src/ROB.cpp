#include "../include/ROB.h"

ROB::ROB() : head(0), tail(0), count(0) {
    for (int i = 0; i < ROB_SIZE; ++i) {
        rob[i] = Rob();
    }
}

bool ROB::is_empty() const {
    return count == 0;
}

bool ROB::is_full() const {
    return count == ROB_SIZE;
}

int ROB::allocate(RobType type, int dest_reg, uint32_t pc, bool predict_jump) {
    int tag = tail;
    rob[tag].type = type;
    rob[tag].available = false;
    rob[tag].dest_reg = dest_reg;
    rob[tag].pc = pc;
    rob[tag].value = 0;
    rob[tag].ready = false;
    rob[tag].predict_jump = predict_jump;
    rob[tag].actual_jump = false;
    tail = (tail + 1) % ROB_SIZE;
    ++count;
    return tag;
}

void ROB::write_result(int tag, int32_t value) {
    rob[tag].value = value;
    rob[tag].ready = true;
}

void ROB::write_branch_result(int tag, bool actual_jump) {
    rob[tag].ready = true;
    rob[tag].actual_jump = actual_jump;
}

int ROB::head_tag() const {
    return head;
}

const Rob& ROB::get_head() const {
    return rob[head];
}

const Rob& ROB::get(int tag) const {
    return rob[tag];
}

bool ROB::can_commit_head() const {
    return count > 0 && rob[head].ready;
}

void ROB::commit_head() {
    rob[head] = Rob();
    head = (head + 1) % ROB_SIZE;
    --count;
}

void ROB::flush(int tag) {
    int new_tail = (tag + 1) % ROB_SIZE;
    while (tail != new_tail) {
        tail = (tail - 1 + ROB_SIZE) % ROB_SIZE;
        rob[tail] = Rob();
        --count;
    }
}