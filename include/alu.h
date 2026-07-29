#pragma once
#include <cstdint>

enum class AluOp{
    ADD, SUB, AND, OR, XOR, SLL, SRL, SRA, SLT, SLTU
};

inline int32_t compute(AluOp op, int32_t l, int32_t r) {
    switch(op) {
        case AluOp::ADD: return l + r;
        case AluOp::SUB: return l - r;
        case AluOp::AND: return l & r;
        case AluOp::OR: return l | r;
        case AluOp::XOR: return l ^ r;
        case AluOp::SLL: return l << (r & 0x1F);
        case AluOp::SRL: return static_cast<int32_t>(static_cast<uint32_t>(l) >> (r & 0x1F));
        case AluOp::SRA: return l >> (r & 0x1F);
        case AluOp::SLT: return (l < r) ? 1 : 0;
        case AluOp::SLTU: return (static_cast<uint32_t>(l) < static_cast<uint32_t>(r)) ? 1 : 0;
    }
    return 0;
}