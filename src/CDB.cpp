#include "../include/CDB.h"

CdbBroadcast Cdb::choose(CdbBroadcast from_rs, CdbBroadcast from_lsq) {
    if (from_lsq.tag != NO_TAG_CDB) return from_lsq;
    if (from_rs.tag != NO_TAG_CDB) return from_rs;
    return CdbBroadcast{NO_TAG_CDB, 0};
}