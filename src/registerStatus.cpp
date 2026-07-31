#include "../include/registerStatus.h"

RegisterStatus::RegisterStatus() {
    for (int i = 0; i < NUM_REGISTERS; ++i) {
        producer_new[i] = NO_PRODUCER;
        producer_old[i] = NO_PRODUCER;
    }
}

int RegisterStatus::get_producer(int r) const {
    return producer_old[r];
}

/*
void RegisterStatus::set_producer(int r, int tag) {
    if (r == 0) return;
    producer_tag[r] = tag;
}

void RegisterStatus::clear_if_match(int r, int tag) {
    if (producer_tag[r] == tag) {
        producer_tag[r] = NO_PRODUCER;
    }
}
*/

void RegisterStatus::sync() {
    for (int i = 0; i < NUM_REGISTERS; ++i) {
        producer_old[i] = producer_new[i];
    }
}

void RegisterStatus::step(int broadcast_tag, int issue_reg, int issue_tag) {
    for (int i = 0; i <  NUM_REGISTERS; ++i) {
        producer_new[i] = producer_old[i];
    }

    if (broadcast_tag != NO_TAG) {
        for (int i = 0; i < NUM_REGISTERS; ++i) {
            if (producer_old[i] == broadcast_tag) {
                producer_new[i] = NO_TAG;
            }
        }
    }

    if (issue_tag != NO_TAG && issue_reg != 0) {
        producer_new[issue_reg] = issue_tag;
    }
}