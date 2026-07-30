#include "../include/registerStatus.h"

RegisterStatus::RegisterStatus() {
    for (int i = 0; i < NUM_REGISTERS; ++i) {
        producer_tag[i] = NO_PRODUCER;
    }
}

int RegisterStatus::get_producer(int r) const {
    return producer_tag[r];
}

void RegisterStatus::set_producer(int r, int tag) {
    if (r == 0) return;
    producer_tag[r] = tag;
}

void RegisterStatus::clear_if_match(int r, int tag) {
    if (producer_tag[r] == tag) {
        producer_tag[r] = NO_PRODUCER;
    }
}