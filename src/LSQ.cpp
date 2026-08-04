#include "../include/LSQ.h"

LSQ::LSQ() : count_new(0), count_old(0) {
    for (int i = 0; i < LSQ_SIZE; ++i) {
        lsq_new[i] = Lsq();
        lsq_old[i] = Lsq();
    }
}

bool LSQ::is_empty() const {
    return count_old == 0;
}

bool LSQ::is_full() const {
    return count_old == LSQ_SIZE;
}

/*
int LSQ::issue(bool is_store, int qj, int32_t vj, int32_t imm, int qk, int32_t vk, int size, bool is_unsigned, int tag) {
    int idx = tail;
    lsq[idx].available = false;
    lsq[idx].is_store = is_store;
    lsq[idx].qj = qj;
    lsq[idx].vj = vj;
    lsq[idx].imm = imm;
    lsq[idx].addr_ready = (qj == NO_TAG);
    if (lsq[idx].addr_ready) lsq[idx].addr = static_cast<uint32_t>(vj + imm);
    else lsq[idx].addr = 0;
    lsq[idx].qk = qk;
    lsq[idx].vk = vk;
    lsq[idx].size = size;
    lsq[idx].is_unsigned = is_unsigned;
    lsq[idx].tag = tag;
    lsq[idx].store_commited = false;
    lsq[idx].remaining_cycles = -1;
    tail = (tail + 1) % LSQ_SIZE;
    ++count;
    return idx;
}

int LSQ::span(int head, int tail, int count) const {
    int s = (tail - head + LSQ_SIZE) % LSQ_SIZE;
    if (s == 0 && count > 0) return LSQ_SIZE;
    return s;
}

void LSQ::update_cdb_broadcast(int tag, int32_t value) {
    int i = head;
    int s = step();
    for (int n = 0; n < s; ++n, i = (i + 1) % LSQ_SIZE) {
        if (lsq[i].available) continue;
        if (lsq[i].qj == tag) {
            lsq[i].vj = value;
            lsq[i].qj = NO_TAG;
            lsq[i].addr_ready = true;
            lsq[i].addr = static_cast<uint32_t>(lsq[i].imm + value);
        }
        if (lsq[i].qk == tag) {
            lsq[i].vk = value;
            lsq[i].qk = NO_TAG;
        }
    }
}

void LSQ::notify_store_commited(int tag) {
    int i = head;
    int s = step();
    for (int n = 0; n < s; ++n, i = (i + 1) % LSQ_SIZE) {
        if (!lsq[i].available && lsq[i].tag == tag && lsq[i].is_store) {
            lsq[i].store_commited = true;
            return;
        }
    }
}
*/

int LSQ::age_distance(int tag, int rob_head) {
    return (tag - rob_head + ROB_SIZE) % ROB_SIZE;
}

bool LSQ::is_blocked_by_store(const Lsq entries[LSQ_SIZE], long load_seq, uint32_t load_addr, bool &forwarded, int32_t &forward_value) {
    forwarded = false;
    long best_seq = -1;
    int best_idx = -1;
    for (int i = 0; i < LSQ_SIZE; ++i) {
        if (entries[i].available || !entries[i].is_store) continue;
        if (entries[i].global_seq >= load_seq) continue;   // 不比load老,跳过
        if (best_idx == -1 || entries[i].global_seq > best_seq) {
            best_idx = i;
            best_seq = entries[i].global_seq;
        }
    }
    if (best_idx == -1) return false; // 没有更老的store了，无冲突
 
    if (!entries[best_idx].addr_ready) return true; // 最近的这个地址未知，必须等
    if (entries[best_idx].addr == load_addr) {
        if (entries[best_idx].qk != NO_TAG) return true; // 地址冲突但值没就绪
        forwarded = true;
        forward_value = entries[best_idx].vk;
        return false;
    }
        // 地址不冲突，继续检查下一个更老的
    return false;
}

/*
bool LSQ::tick(Storage &storage, int &out_tag, int32_t &out_value) {
    // 先尝试让地址已经好但还没开始访存的指令开始访存
    int i = head;
    int s = step();
    for (int n = 0; n < s; ++n, i = (i + 1) % LSQ_SIZE) {
        Lsq &cur = lsq[i];
        if (cur.available || cur.remaining_cycles >= 0 || !cur.addr_ready) continue;

        if (cur.is_store) {
            if (cur.store_commited && cur.qk == NO_TAG) {
                cur.remaining_cycles = CYCLES;
            }
            continue;
        }

        // load
        bool forwarded = false;
        int32_t forward_value = 0;
        bool blocked = is_blocked_by_store(i, cur.addr, forwarded, forward_value);

        if (blocked) continue;
        if (forwarded) {
            out_value = apply_load_sign(forward_value, cur.size, cur.is_unsigned);
            out_tag = cur.tag;
            clear(i);
            return true;
        }
        cur.remaining_cycles = CYCLES;
    }

    // 再推进所有正在访问的
    i = head;
    for (int n = 0; n < s; ++n, i = (i + 1) % LSQ_SIZE) {
        Lsq &cur = lsq[i];
        if (cur.available || cur.remaining_cycles < 0) continue;
        --cur.remaining_cycles;
        if (cur.remaining_cycles > 0) continue;
        if (cur.is_store) {
            write(storage, cur.addr, cur.vk, cur.size);
            clear(i);
        }
        else {
            out_value = read(storage, cur.addr, cur.size, cur.is_unsigned);
            out_tag = cur.tag;
            clear(i);
            return true;
        }
    }
    return false;
}

void LSQ::clear(int idx) {
    lsq[idx] = Lsq();
    --count;
    while (count > 0 && lsq[head].available) {
        head = (head + 1) % LSQ_SIZE;
    }
}
*/

int32_t LSQ::apply_load_sign(int32_t raw_value, int size, bool is_unsigned) {
    if (size == 4) return raw_value;

    int bits = size * 8;
    if (is_unsigned) {
        uint32_t mask = (1u << bits) - 1;
        return static_cast<int32_t>(static_cast<uint32_t>(raw_value) & mask);
    }

    // 符号扩展
    uint32_t sign = (static_cast<uint32_t>(raw_value) >> (bits - 1)) & 1u;
    if (sign == 1) {
        uint32_t high_mask = 0xFFFFFFFFu << bits;
        return static_cast<int32_t>(high_mask | static_cast<uint32_t>(raw_value));
    } else {
        return raw_value;
    }
}

void LSQ::write(Storage &storage, uint32_t addr, int32_t value, int size) {
    if (size == 1) storage.write_byte(addr, static_cast<uint8_t>(value & 0xFF));
    else if (size == 2) storage.write_half_word(addr, static_cast<uint16_t>(value & 0xFFFF));
    else storage.write_word(addr, static_cast<uint32_t>(value));
}

int32_t LSQ::read(const Storage &storage, uint32_t addr, int size, bool is_unsigned) {
    if (size == 1) {
        uint8_t value = storage.read_byte(addr);
        return apply_load_sign(value, size, is_unsigned);
    } else if (size == 2) {
        uint16_t value = storage.read_half_word(addr);
        return apply_load_sign(value, size, is_unsigned);
    } else {
        return static_cast<int32_t>(storage.read_word(addr));
    }
}

/*
bool LSQ::is_occupied(int idx) const {
    return !lsq[idx].available;
}

int LSQ::tag_of(int idx) const {
    return lsq[idx].tag;
}
*/

// 倒计时到0 / forward成功
CdbBroadcast LSQ::get_broadcast(const Storage& storage) const {
    for (int i = 0; i < LSQ_SIZE; ++i) {
        const Lsq &l = lsq_old[i];
        if (l.available || l.is_store) continue;
        if (l.remaining_cycles == 0) {
            int32_t value = read(storage, l.addr, l.size, l.is_unsigned);
            return CdbBroadcast{l.tag, value};
        }
        if (l.remaining_cycles == -1 && l.addr_ready) {
            bool forwarded = false; 
            int32_t fv = 0;
            bool blocked = is_blocked_by_store(lsq_old, l.global_seq, l.addr, forwarded, fv);
            if (!blocked && forwarded) {
                return CdbBroadcast{l.tag, apply_load_sign(fv, l.size, l.is_unsigned)};
            }
        }
    }
    return CdbBroadcast{NO_TAG, 0};
}

void LSQ::sync() {
    for (int i = 0; i < LSQ_SIZE; ++i) lsq_old[i] = lsq_new[i];
    count_old = count_new;
}

// store_commit_tag: 此刻ROB通知这个标签的store可以写内存了，NO_TAG表示没有
void LSQ::step(Storage& storage, CdbBroadcast cdb, int store_commit_tag, 
               bool issue_is_store, int issue_qj, int32_t issue_vj, int32_t issue_imm,
               int issue_qk, int32_t issue_vk, int issue_size, bool issue_unsigned, int issue_tag, 
               int flush_tag, long flush_seq, long issue_global_seq, long global_commit_counter) {
    for (int i = 0; i < LSQ_SIZE; ++i) lsq_new[i] = lsq_old[i];
    count_new = count_old;

    // cdb广播的是不是某条load
    CdbBroadcast my_candidate = get_broadcast(storage);
    
    if (my_candidate.tag != NO_TAG_CDB && my_candidate.tag == cdb.tag) {
        for (int i = 0; i < LSQ_SIZE; ++i) {
            if (lsq_old[i].tag == my_candidate.tag && !lsq_old[i].available && !lsq_old[i].is_store) {
                lsq_new[i] = Lsq();
                --count_new;
                break;
            }
        }
    }

    // 接收cdb
    if (cdb.tag != NO_TAG_CDB) {
        for (int i = 0; i < LSQ_SIZE; ++i) {
            if (lsq_old[i].available) continue;
            if (lsq_old[i].qj == cdb.tag) {
                lsq_new[i].qj = NO_TAG;
                lsq_new[i].vj = cdb.value;
                lsq_new[i].addr_ready = true;
                lsq_new[i].addr = static_cast<uint32_t>(cdb.value + lsq_old[i].imm);
            }
            if (lsq_old[i].qk == cdb.tag) {
                lsq_new[i].vk = cdb.value;
                lsq_new[i].qk = NO_TAG;
            }
        }
    }

    // ROB通知某个store可以提交
    if (store_commit_tag != NO_TAG) {
        for (int i = 0; i < LSQ_SIZE; ++i) {
            if (lsq_old[i].tag == store_commit_tag && !lsq_old[i].available && lsq_old[i].is_store) {
                lsq_new[i].store_commited = true;
                // lsq_new[i].commit_order = global_commit_counter;
            }
        }
    }

    // 尝试让地址已经好但还没开始访存的指令开始访存
    for (int i = 0; i < LSQ_SIZE; ++i) {
        const Lsq &cur = lsq_old[i];
        if (cur.available || cur.remaining_cycles >= 0 || !cur.addr_ready) continue;

        if (cur.is_store) {
            if (cur.store_commited && cur.qk == NO_TAG) {
                lsq_new[i].remaining_cycles = CYCLES - 1;
            }
            continue;
        }

        // load
        bool forwarded = false;
        int32_t forward_value = 0;
        bool blocked = is_blocked_by_store(lsq_old, cur.global_seq, cur.addr, forwarded, forward_value);
        if (blocked) continue;
        if (forwarded) continue; // 已经在第一步处理过
        lsq_new[i].remaining_cycles = CYCLES - 1;
    }

    // 推进
    for (int i = 0; i < LSQ_SIZE; ++i) {
        const Lsq &cur = lsq_old[i];
        if (cur.available || cur.remaining_cycles < 0) continue;
        if (cur.remaining_cycles == 0 && cur.is_store) {
            write(storage, cur.addr, cur.vk, cur.size);
            lsq_new[i] = Lsq();
            count_new -= 1;
            continue;
        }
        // load不管，在第一步cdb那里已经处理过
        lsq_new[i].remaining_cycles = lsq_old[i].remaining_cycles - 1;
    }

    // issue
    if (issue_tag != NO_TAG && count_old < LSQ_SIZE) {
        for (int i = 0; i < LSQ_SIZE; ++i) {
            if (lsq_old[i].available) {
                lsq_new[i].available = false;
                lsq_new[i].is_store = issue_is_store;
                lsq_new[i].qj = issue_qj;
                lsq_new[i].vj = issue_vj;
                lsq_new[i].imm = issue_imm;
                lsq_new[i].addr_ready = (issue_qj == NO_TAG);
                lsq_new[i].addr = lsq_new[i].addr_ready ? static_cast<uint32_t>(issue_vj + issue_imm) : 0;
                lsq_new[i].qk = issue_qk;
                lsq_new[i].vk = issue_vk;
                lsq_new[i].size = issue_size;
                lsq_new[i].is_unsigned = issue_unsigned;
                lsq_new[i].tag = issue_tag;
                lsq_new[i].store_commited = false;
                lsq_new[i].remaining_cycles = -1;
                lsq_new[i].global_seq = issue_global_seq;
                ++count_new;
                break;
            }
        }
    }

    // flush: 所有比flush_tag年轻的都要flush
    if (flush_tag != NO_TAG) {
        for (int i = 0; i < LSQ_SIZE; ++i) {
            if (lsq_old[i].available) continue;
            if (lsq_old[i].global_seq > flush_seq) {
                if (!lsq_new[i].available) {
                    lsq_new[i] = Lsq();
                    --count_new;
                }
            }
        }
    }
}

// 选离rob_head最近的(最老的那个)
int LSQ::get_store_ready_tag() const {
    int best_tag = NO_TAG;
    long best_seq = -1;
    for (int i = 0; i < LSQ_SIZE; ++i) {
        const Lsq &l = lsq_old[i];
        if (l.available || !l.is_store) continue;
        if (!(l.addr_ready && l.qk == NO_TAG)) continue;
        if (best_tag == NO_TAG || l.global_seq < best_seq) {
            best_tag = l.tag;
            best_seq = l.global_seq;
        }
    }
    return best_tag;
}