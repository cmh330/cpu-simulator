#pragma once
#include <cstdint>
#include <algorithm>
#include "definitions.h"

enum class OpcodeValue : uint32_t {
    OP = 0b0110011,
    OP_IMM = 0b0010011,
    LOAD = 0b0000011,
    STORE = 0b0100011,
    BRANCH = 0b1100011,
    JAL = 0b1101111,
    JALR = 0b1100111,
    AUIPC = 0b0010111,
    LUI = 0b0110111
};

// [l, r] 未做符号拓展
inline uint32_t extract_bit(uint32_t input, int r, int l) {
    if (r < l) std::swap(l, r);
    uint32_t ans = input >> l;
    int width = r - l + 1;
    uint32_t mask;
    if (width == 32) mask = 0xFFFFFFFFu;
    else mask = (1u << width) - 1u;
    return ans & mask;
}

inline int32_t sign_extend(uint32_t input, int width) {
    uint32_t sign = (input >> (width - 1)) & 1u;
    if (sign == 1) {
        uint32_t high_mask = 0xFFFFFFFFu << width;
        return static_cast<int32_t>(high_mask | input);
    } else return static_cast<int32_t>(input);
}

inline DecodedInstruction decode(uint32_t input) {
    DecodedInstruction result;
    result.raw_input = input;
    uint32_t opcode_value = extract_bit(input, 6, 0);
    uint32_t funct3 = extract_bit(input, 14, 12);
    uint32_t funct7 = extract_bit(input, 31, 25);
    uint8_t rs1 = static_cast<uint8_t>(extract_bit(input, 19, 15));
    uint8_t rs2 = static_cast<uint8_t>(extract_bit(input, 24, 20));
    uint8_t rd = static_cast<uint8_t>(extract_bit(input, 11, 7));
    result.rs1 = rs1;
    result.rs2 = rs2;
    result.rd = rd;

    switch(static_cast<OpcodeValue>(opcode_value)) {
        case OpcodeValue::AUIPC: {
            uint32_t temp = extract_bit(input, 31, 12);
            result.imm = static_cast<int32_t>(temp << 12);
            result.opcode = Opcode::AUIPC;
            break;
        }
        case OpcodeValue::LUI: {
            uint32_t temp = extract_bit(input, 31, 12);
            result.imm = static_cast<int32_t>(temp << 12);
            result.opcode = Opcode::LUI;
            break;
        }
        case OpcodeValue::JAL: {
            uint32_t imm_20 = extract_bit(input, 31, 31);
            uint32_t imm_10_1 = extract_bit(input, 30, 21);
            uint32_t imm_11 = extract_bit(input, 20, 20);
            uint32_t imm_19_12 = extract_bit(input, 19, 12);
            uint32_t imm_unsigned = (imm_20 << 20) | (imm_10_1 << 1) | (imm_11 << 11) | (imm_19_12 << 12);
            result.imm = sign_extend(imm_unsigned, 21);
            result.opcode = Opcode::JAL;
            break; 
        }
        case OpcodeValue::JALR: {
            uint32_t imm_unsigned = extract_bit(input, 31, 20);
            result.imm = sign_extend(imm_unsigned, 12);
            result.opcode = Opcode::JALR;
            break;
        }

        case OpcodeValue::BRANCH: {
            uint32_t imm_12 = extract_bit(input, 31, 31);
            uint32_t imm_10_5 = extract_bit(input, 30, 25);
            uint32_t imm_4_1 = extract_bit(input, 11, 8);
            uint32_t imm_11 = extract_bit(input, 7, 7);
            uint32_t imm_unsigned = (imm_12 << 12) | (imm_10_5 << 5) | (imm_4_1 << 1) | (imm_11 << 11);
            result.imm = sign_extend(imm_unsigned, 13);
            switch(funct3) {
                case 0b000: result.opcode = Opcode::BEQ; break;
                case 0b101: result.opcode = Opcode::BGE; break;
                case 0b111: result.opcode = Opcode::BGEU; break;
                case 0b100: result.opcode = Opcode::BLT; break;
                case 0b110: result.opcode = Opcode::BLTU; break;
                case 0b001: result.opcode = Opcode::BNE; break;
                default: result.opcode = Opcode::INVALID; break;
            }
            break;
        }

        case OpcodeValue::STORE: {
            uint32_t imm_11_5 = extract_bit(input, 31, 25);
            uint32_t imm_4_0 = extract_bit(input, 11, 7);
            uint32_t imm_unsigned = (imm_11_5 << 5) | imm_4_0;
            result.imm = sign_extend(imm_unsigned, 12);
            switch(funct3) {
                case 0b000: result.opcode = Opcode::SB; break;
                case 0b001: result.opcode = Opcode::SH; break;
                case 0b010: result.opcode = Opcode::SW; break;
                default: result.opcode = Opcode::INVALID; break;
            }
            break;
        }

        case OpcodeValue::LOAD: {
            uint32_t imm = extract_bit(input, 31, 20);
            result.imm = sign_extend(imm, 12);
            switch(funct3) {
                case 0b000: result.opcode = Opcode::LB; break;
                case 0b100: result.opcode = Opcode::LBU; break;
                case 0b001: result.opcode = Opcode::LH; break;
                case 0b101: result.opcode = Opcode::LHU; break;
                case 0b010: result.opcode = Opcode::LW; break;
                default: result.opcode = Opcode::INVALID; break;
            }
            break;
        }

        case OpcodeValue::OP_IMM: {
            uint32_t imm_unsigned = extract_bit(input, 31, 20);
            result.imm = sign_extend(imm_unsigned, 12);
            switch(funct3) {
                case 0b000: result.opcode = Opcode::ADDI; break;
                case 0b111: result.opcode = Opcode::ANDI; break;
                case 0b110: result.opcode = Opcode::ORI; break;
                case 0b100: result.opcode = Opcode::XORI; break;
                case 0b010: result.opcode = Opcode::SLTI; break;
                case 0b011: result.opcode = Opcode::SLTIU; break;
                case 0b001: {
                    result.opcode = Opcode::SLLI; 
                    result.imm = static_cast<int32_t>(extract_bit(input, 24, 20));
                    break;
                }
                case 0b101: {
                    result.imm = static_cast<int32_t>(extract_bit(input, 24, 20));
                    if (funct7 == 0b0000000) result.opcode = Opcode::SRLI;
                    else if (funct7 == 0b0100000) result.opcode = Opcode::SRAI;
                    else result.opcode = Opcode::INVALID;
                    break;
                }
                default: result.opcode = Opcode::INVALID; break;
            }
            break;
        }

        case OpcodeValue::OP: {
            switch(funct3) {
                case 0b000: {
                    if (funct7 == 0b0000000) result.opcode = Opcode::ADD;
                    else if (funct7 == 0b0100000) result.opcode = Opcode::SUB;
                    else result.opcode = Opcode::INVALID;
                    break;
                }
                case 0b111: result.opcode = Opcode::AND; break;
                case 0b110: result.opcode = Opcode::OR; break;
                case 0b100: result.opcode = Opcode::XOR; break;
                case 0b001: result.opcode = Opcode::SLL; break;
                case 0b101: {
                    if (funct7 == 0b0000000) result.opcode = Opcode::SRL;
                    else if (funct7 == 0b0100000) result.opcode = Opcode::SRA;
                    else result.opcode = Opcode::INVALID;
                    break;
                } 
                case 0b010: result.opcode = Opcode::SLT; break;
                case 0b011: result.opcode = Opcode::SLTU; break;
                default: result.opcode = Opcode::INVALID; break;
            }
            break;
        }

        default: result.opcode = Opcode::INVALID; break;
    }
    return result;
}