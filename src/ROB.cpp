#include "../include/ROB.h"

ROB::ROB() : head_old(0), tail_old(0), count_old(0), head_new(0), tail_new(0), count_new(0) {
    for (int i = 0; i < ROB_SIZE; ++i) {
        rob_new[i] = Rob();
        rob_old[i] = Rob();
    }
}

bool ROB::is_empty() const {
    return count_old == 0;
}

bool ROB::is_full() const {
    return count_old == ROB_SIZE;
}

/*
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

const Rob& ROB::get_head() const {
    return rob[head];
}
*/

int ROB::head_tag() const {
    return head_old;
}

int ROB::tail_tag() const {
    return tail_old;
}

const Rob& ROB::get(int tag) const {
    return rob_old[tag];
}

/*
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
*/

ROB::CommitResult ROB::get_commit() const {
    CommitResult r;
    if (count_old == 0 || !rob_old[head_old].ready) return r;
    const Rob &temp = rob_old[head_old];
    r.type = temp.type;
    r.value = temp.value;
    r.tag = head_old;
    r.dest_reg = temp.dest_reg;
    if (temp.type == RobType::BRANCH) {
        r.mispredict = (temp.actual_jump != temp.predict_jump);
        r.correct_pc = temp.actual_jump ? temp.target : (temp.pc + 4);
    }
    return r;
}

void ROB::sync() {
    for (int i = 0; i < ROB_SIZE; ++i) rob_old[i] = rob_new[i];
    head_old = head_new;
    tail_old = tail_new;
    count_old = count_new;
}

void ROB::step(CdbBroadcast cdb, int branch_tag, bool branch_jump, int store_ready_tag,
               bool issue_valid, RobType issue_type, int issue_dest_reg,
               uint32_t issue_pc, uint32_t issue_target, bool issue_predict_jump) {
    for (int i = 0; i < ROB_SIZE; ++i) rob_new[i] = rob_old[i];
    head_new = head_old; 
    tail_new = tail_old;
    count_new = count_old;

    // cdb
    if (cdb.tag != NO_TAG_CDB) {
        rob_new[cdb.tag].value = cdb.value;
        rob_new[cdb.tag].ready = true;
    }

    // branch
    if (branch_tag != NO_TAG) {
        rob_new[branch_tag].actual_jump = branch_jump;
        rob_new[branch_tag].ready = true;
    }
    if (store_ready_tag != NO_TAG) {
        rob_new[store_ready_tag].ready = true;
    }

    // commit
    bool commit = (count_old > 0 && rob_old[head_old].ready);
    if (commit) {
        rob_new[head_old] = Rob();
        head_new = (head_old + 1) % ROB_SIZE;
        --count_new;
    }

    // issue
    if (issue_valid && count_old < ROB_SIZE) {
        int tag = tail_old;
        rob_new[tag].available = false;
        rob_new[tag].type = issue_type;
        rob_new[tag].dest_reg = issue_dest_reg;
        rob_new[tag].value = 0;
        rob_new[tag].ready = false;
        rob_new[tag].pc = issue_pc;
        rob_new[tag].target = issue_target;
        rob_new[tag].predict_jump = issue_predict_jump;
        rob_new[tag].actual_jump = false;
        tail_new = (tail_old + 1) % ROB_SIZE;
        ++count_new;
    }
}