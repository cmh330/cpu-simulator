#pragma once
#include <cstdint>

enum class Opcode : uint8_t {
    // R
    ADD, SUB, AND, OR, XOR, SLL, SRL, SRA, SLT, SLTU,  
    // I(立即数)
    ADDI, ANDI, ORI, XORI, SLLI, SRLI, SRAI, SLTI, SLTIU,
    // I(load)
    LB, LBU, LH, LHU, LW,
    // S(store)
    SB, SH, SW,
    // B
    BEQ, BGE, BGEU, BLT, BLTU, BNE,
    // J
    JAL,
    // I
    JALR, 
    // U
    AUIPC,
    // U
    LUI, 

    INVALID
};

struct DecodedInstruction {
    Opcode opcode = Opcode::INVALID;
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;
    uint8_t rd = 0;
    int32_t imm = 0;
    uint32_t raw_input = 0;
};