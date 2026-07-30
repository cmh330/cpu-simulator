#include "../include/reservationStation.h"

RSGroup::RSGroup() {
    for (int i = 0; i < NUM_RS; ++i) {
        rs[i] = RS();
    }
}

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