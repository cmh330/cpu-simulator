#pragma once
#include <cstdint>

const int NO_TAG_CDB = -1;

struct CdbBroadcast{
    int tag = NO_TAG_CDB; // ROB下标
    int32_t value = 0;
};

class Cdb {
public:
    static CdbBroadcast choose(CdbBroadcast from_rs, CdbBroadcast from_lsq);
};