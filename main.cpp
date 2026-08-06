#include <iostream>
#include "include/decoder.h"
#include "include/definitions.h"
#include "include/alu.h"
#include "include/storage.h"
#include "include/cpu.h"

// g++ -std=c++17 -o code main.cpp src/*.cpp -Iinclude
// ./code < 下发/data/testcases/array_test1.data
// ./run_all_tests.sh ./code 下发/data/testcases

const uint32_t END = 0x0ff00513;

uint8_t run_naive(Storage &storage, int32_t registers[32]);

int main() {
    /*
    Storage storage;
    storage.load(std::cin);
    int32_t registers[32] = {0};
    uint8_t ans = run_naive(storage, registers);
    std::cout << "naive: " << static_cast<int>(ans) << std::endl;
    */
    
    CPU cpu;
    cpu.load_program(std::cin);
    uint8_t result = cpu.run();
    std::cout << static_cast<int>(result) << std::endl;
    // std::cout << "accuracy: " << cpu.branch_accuracy() << std::endl;
    // std::cerr << "CYCLES:" << cpu.cycle_count() << std::endl;
    return 0;
}

uint8_t run_naive(Storage &storage, int32_t registers[32]) {
    uint32_t pc = 0;

    while (true) {
        uint32_t raw_instruction = storage.read_word(pc);
        if (raw_instruction == END) {
            return static_cast<uint8_t>(registers[10] & 0xFF);
        }

        DecodedInstruction instruction = decode(raw_instruction);
        uint32_t next_pc = pc + 4;
        int32_t rs1_num = registers[instruction.rs1];
        int32_t rs2_num = registers[instruction.rs2];
        int32_t result = 0;
        bool wirte_rd = true;

        switch(instruction.opcode) {
            case Opcode::LUI: {
                result = instruction.imm;
                break;
            }
            case Opcode::AUIPC: {
                result = instruction.imm + static_cast<int32_t>(pc);
                break;
            }
            
            case Opcode::JALR: {
                // next pc最低位必须是0
                result = static_cast<int32_t>(pc + 4);
                next_pc = static_cast<uint32_t>((rs1_num + instruction.imm) & ~1u);
                break;
            }
            case Opcode::JAL: {
                result = static_cast<int32_t>(pc + 4);
                next_pc = pc + static_cast<uint32_t>(instruction.imm);
                break;
            }

            case Opcode::BEQ: {
                wirte_rd = false;
                if (rs1_num == rs2_num) next_pc = pc + static_cast<uint32_t>(instruction.imm);
                break;
            }
            case Opcode::BGE: {
                wirte_rd = false;
                if (rs1_num >= rs2_num) next_pc = pc + static_cast<uint32_t>(instruction.imm);
                break;
            }
            case Opcode::BGEU: {
                wirte_rd = false;
                if (static_cast<uint32_t>(rs1_num) >= static_cast<uint32_t>(rs2_num)) next_pc = pc + static_cast<uint32_t>(instruction.imm);
                break;
            }
            case Opcode::BLT: {
                wirte_rd = false;
                if (rs1_num < rs2_num) next_pc = pc + static_cast<uint32_t>(instruction.imm);
                break;
            }
            case Opcode::BLTU: {
                wirte_rd = false;
                if (static_cast<uint32_t>(rs1_num) < static_cast<uint32_t>(rs2_num)) next_pc = pc + static_cast<uint32_t>(instruction.imm);
                break;
            }
            case Opcode::BNE: {
                wirte_rd = false;
                if (rs1_num != rs2_num) next_pc = pc + static_cast<uint32_t>(instruction.imm);
                break;
            }

            case Opcode::SB: {
                wirte_rd = false;
                uint32_t addr = static_cast<uint32_t>(rs1_num + instruction.imm);
                storage.write_byte(addr, static_cast<uint8_t>(rs2_num & 0xFF));
                break;
            }
            case Opcode::SH: {
                wirte_rd = false;
                uint32_t addr = static_cast<uint32_t>(rs1_num + instruction.imm);
                storage.write_half_word(addr, static_cast<uint16_t>(rs2_num & 0xFFFF));
                break;
            }
            case Opcode::SW: {
                wirte_rd = false;
                uint32_t addr = static_cast<uint32_t>(rs1_num + instruction.imm);
                storage.write_word(addr, static_cast<uint32_t>(rs2_num));
                break;
            }

            case Opcode::LB: {
                uint32_t addr = static_cast<uint32_t>(rs1_num + instruction.imm);
                result = sign_extend(storage.read_byte(addr), 8);
                break;
            }
            case Opcode::LBU: {
                uint32_t addr = static_cast<uint32_t>(rs1_num + instruction.imm);
                result = static_cast<int32_t>(storage.read_byte(addr));
                break;
            }
            case Opcode::LH: {
                uint32_t addr = static_cast<uint32_t>(rs1_num + instruction.imm);
                result = sign_extend(storage.read_half_word(addr), 16);
                break;
            }
            case Opcode::LHU: {
                uint32_t addr = static_cast<uint32_t>(rs1_num + instruction.imm);
                result = static_cast<int32_t>(storage.read_half_word(addr));
                break;
            }
            case Opcode::LW: {
                uint32_t addr = static_cast<uint32_t>(rs1_num + instruction.imm);
                result = static_cast<int32_t>(storage.read_word(addr));
                break;
            }

            case Opcode::ADDI: result = compute(AluOp::ADD, rs1_num, instruction.imm); break;
            case Opcode::ANDI: result = compute(AluOp::AND, rs1_num, instruction.imm); break;
            case Opcode::ORI: result = compute(AluOp::OR, rs1_num, instruction.imm); break;
            case Opcode::XORI: result = compute(AluOp::XOR, rs1_num, instruction.imm); break;
            case Opcode::SLLI: result = compute(AluOp::SLL, rs1_num, instruction.imm); break;
            case Opcode::SRLI: result = compute(AluOp::SRL, rs1_num, instruction.imm); break;
            case Opcode::SRAI: result = compute(AluOp::SRA, rs1_num, instruction.imm); break;
            case Opcode::SLTI: result = compute(AluOp::SLT, rs1_num, instruction.imm); break;
            case Opcode::SLTIU: result = compute(AluOp::SLTU, rs1_num, instruction.imm); break;

            case Opcode::ADD: result = compute(AluOp::ADD, rs1_num, rs2_num); break;
            case Opcode::SUB: result = compute(AluOp::SUB, rs1_num, rs2_num); break;
            case Opcode::AND: result = compute(AluOp::AND, rs1_num, rs2_num); break;
            case Opcode::OR: result = compute(AluOp::OR, rs1_num, rs2_num); break;
            case Opcode::XOR: result = compute(AluOp::XOR, rs1_num, rs2_num); break;
            case Opcode::SLL: result = compute(AluOp::SLL, rs1_num, rs2_num); break;
            case Opcode::SRL: result = compute(AluOp::SRL, rs1_num, rs2_num); break;
            case Opcode::SRA: result = compute(AluOp::SRA, rs1_num, rs2_num); break;
            case Opcode::SLT: result = compute(AluOp::SLT, rs1_num, rs2_num); break;
            case Opcode::SLTU: result = compute(AluOp::SLTU, rs1_num, rs2_num); break;

            case Opcode::INVALID:
            default:
                std::cout << "INVALID\n";
                return 0x0;
        }

        if (wirte_rd && instruction.rd != 0) {
            registers[instruction.rd] = result;
        }

        registers[0] = 0;
        pc = next_pc;
    }
}