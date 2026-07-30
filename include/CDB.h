#pragma once
#include <cstdint>
#include "reservationStation.h"

struct CdbBroadcast{
    int tag = -NO_TAG; // ROB下标
    int32_t value = 0;
};