#pragma once
#include <cstdint>
#include "CDB.h"
#include "storage.h"
#include "ROB.h"

const int LSQ_SIZE = 10;
const int CYCLES = 3;
// const int NO_TAG = -1;

struct Lsq {
    bool available = true;
    bool is_store = false;
    int qj = NO_TAG, qk = NO_TAG;
    int32_t vj = 0, vk = 0, imm = 0;
    uint32_t addr = 0;
    bool addr_ready = false;
    int size = 4; // 字节
    bool is_unsigned = false; // load时才用
    int tag = NO_TAG; // 发布时ROB分配的标签，load：用tag广播结果，store：看是否轮到提交
    bool store_commited = false; // store时才用，记录ROB是否已经允许他真正写内存
    int remaining_cycles = -1; // >= 0才开始访问内存
    // int commit_order = -1;
    long global_seq = -1;
};

class LSQ {
private:
    Lsq lsq_new[LSQ_SIZE];
    Lsq lsq_old[LSQ_SIZE];
    int count_old;
    int count_new;
    // int span(int head, int tail, int count) const;

    static int age_distance(int tag, int rob_head);
    // 返回true：被挡住，load要等；返回false & forwarded = true，没被挡，forward_value里是值；返回false & forwarded = false，没被挡，无冲突
    static bool is_blocked_by_store(const Lsq entries[LSQ_SIZE], long load_seq, uint32_t load_addr, bool &forwarded, int32_t &forward_value);

    static int32_t apply_load_sign(int32_t raw_value, int size, bool is_unsigned);
    static void write(Storage &storage, uint32_t addr, int32_t value, int size);
    static int32_t read(const Storage &storage, uint32_t addr, int size, bool is_unsigned);

public:
    LSQ();
    bool is_empty() const;
    bool is_full() const;
    // int issue(bool is_store, int qj, int32_t vj, int32_t imm, int qk, int32_t vk, int size, bool is_unsigned, int tag);
    // void update_cdb_broadcast(int tag, int32_t value);
    // void notify_store_commited(int tag); // ROB提交到某个store时，它可以写内存
    // bool tick(Storage &storage, int &out_tag, int32_t &out_value); // 返回true：有一个load完成，要广播
    // void clear(int idx);
    // bool is_occupied(int idx) const;
    // int tag_of(int idx) const;
    CdbBroadcast get_broadcast(const Storage& storage) const;
    void sync();
    void step(Storage& storage, CdbBroadcast cdb, int store_commit_tag, 
              bool issue_is_store, int issue_qj, int32_t issue_vj, int32_t issue_imm,
              int issue_qk, int32_t issue_vk, int issue_size, bool issue_unsigned, int issue_tag, 
              int flush_tag, long flush_seq, long issue_global_seq, long global_commit_counter);
    int get_store_ready_tag() const;
};