#include "../include/reservationStation.h"

RSGroup::RSGroup() {
    for (int i = 0; i < NUM_RS; ++i) {
        rs_new[i] = RS();
        rs_old[i] = RS();
    }
}

/*
int RSGroup::find_available() const {
    for(int i = 0; i < NUM_RS; ++i) {
        if (rs[i].available) return i;
    }
    return -1;
}

bool RSGroup::is_ready(int idx) const {
    return !rs[idx].available && rs[idx].qj == NO_TAG && rs[idx].qk == NO_TAG;
}

void RSGroup::issue(int idx, AluOp op, int32_t vj, int32_t vk, int qj, int qk, int dest, bool is_branch) {
    rs[idx].available = false;
    rs[idx].op = op;
    rs[idx].vj = vj;
    rs[idx].vk = vk;
    rs[idx].qj = qj;
    rs[idx].qk = qk;
    rs[idx].dest = dest;
    rs[idx].is_branch = is_branch;
}

void RSGroup::clear(int idx) {
    rs[idx] = RS();
}

void RSGroup::update_cdb_broadcast(int tag, int32_t value) {
    for (int i = 0; i < NUM_RS; ++i) {
        if (rs[i].available) continue;
        if (rs[i].qj == tag) {
            rs[i].qj = NO_TAG;
            rs[i].vj = value;
        }
        if (rs[i].qk == tag) {
            rs[i].qk = NO_TAG;
            rs[i].vk = value;
        }
    }
}

bool RSGroup::is_occupied(int idx) const {
    return !rs[idx].available;
}

int RSGroup::get_dest(int idx) const {
    return rs[idx].dest;
}
*/

void RSGroup::sync() {
    for (int i = 0; i < NUM_RS; ++i) {
        rs_old[i] = rs_new[i];
    }
}

bool RSGroup::is_full() const {
    for (int i = 0; i < NUM_RS; ++i) {
        if (rs_old[i].available) return false;
    }
     return true;
}

// 返回结构体中的tag是rob下标
CdbBroadcast RSGroup::get_broadcast() const {
    CdbBroadcast candidate = {NO_TAG_CDB, 0};
    for(int i = 0; i < NUM_RS; ++i) {
        if (!rs_old[i].available && !rs_old[i].is_branch && rs_old[i].qj == NO_TAG_RS && rs_old[i].qk == NO_TAG_RS) {
            candidate.value = compute(rs_old[i].op, rs_old[i].vj, rs_old[i].vk); 
            candidate.tag = rs_old[i].dest;
            return candidate;
        }
    }
    return candidate;
}

void RSGroup::step(CdbBroadcast cdb, AluOp issue_op, int32_t issue_vj, int issue_qj,
                   int32_t issue_vk, int issue_qk, int issue_dest, bool issue_is_branch, 
                   int flush_tag, int rob_head) {
    for (int i = 0; i < NUM_RS; ++i) rs_new[i] = rs_old[i];

    // 看cdb能不能释放rs槽位
    CdbBroadcast my_candidate = get_broadcast();
    if (my_candidate.tag != NO_TAG_CDB && my_candidate.tag == cdb.tag) {
        for (int i = 0; i < NUM_RS; ++i) {
            if (!rs_old[i].available && rs_old[i].dest == cdb.tag) {
                rs_new[i] = RS();
            }
        }
    }

    // 看branch部分能不能释放槽位
    BranchResult my_branch = get_branch_result();
    if (my_branch.tag != NO_TAG_RS) {
        for (int i = 0; i < NUM_RS; ++i) {
            if (!rs_old[i].available && rs_old[i].is_branch && rs_old[i].dest == my_branch.tag) {
                rs_new[i] = RS();
            }
        }
    }

    // 清掉等cdb标签的槽位
    if (cdb.tag != NO_TAG_CDB) {
        for (int i = 0; i < NUM_RS; ++i) {
            if (rs_old[i].available) continue;
            if (rs_old[i].qj == cdb.tag) {
                rs_new[i].qj = NO_TAG_RS;
                rs_new[i].vj = cdb.value;
            }
            if (rs_old[i].qk == cdb.tag) {
                rs_new[i].qk = NO_TAG_RS;
                rs_new[i].vk = cdb.value;
            }
        }
    }

    // 处理发布，找下标最小的空闲槽位
    if (issue_dest != NO_TAG_RS) {
        for (int i = 0; i < NUM_RS; ++i) {
            if (rs_old[i].available) {
                rs_new[i].available = false;
                rs_new[i].dest = issue_dest;
                rs_new[i].op = issue_op;
                rs_new[i].vj = issue_vj;
                rs_new[i].qj = issue_qj;
                rs_new[i].vk = issue_vk;
                rs_new[i].qk = issue_qk;
                rs_new[i].is_branch = issue_is_branch;
                break;
            }
        }
    }

    // flush
    if (flush_tag != NO_TAG) {
        int flush_dist = age_distance(flush_tag, rob_head);
        for (int i = 0; i < NUM_RS; ++i) {
            if (rs_old[i].available) continue;
            if (age_distance(rs_old[i].dest, rob_head) > flush_dist) {
                rs_new[i] = RS();
            }
        }
    }
}

int RSGroup::age_distance(int tag, int rob_head) {
    return (tag - rob_head + ROB_SIZE) % ROB_SIZE;
}

BranchResult RSGroup::get_branch_result() const {
    BranchResult result;
    for (int i = 0; i < NUM_RS; ++i) {
        if (!rs_old[i].available && rs_old[i].is_branch && rs_old[i].qj == NO_TAG_RS && rs_old[i].qk == NO_TAG_RS) {
            int32_t v = compute(rs_old[i].op, rs_old[i].vj, rs_old[i].vk);
            result.tag = rs_old[i].dest;
            result.actual_jump = (v != 0);
            return result;
        }
    }
    return result;
}