#pragma once
#include <cstdint>
#include <iostream>
#include "storage.h"
#include "definitions.h"
#include "decoder.h"
#include "alu.h"
#include "CDB.h"
#include "registerStatus.h"
#include "reservationStation.h"
#include "ROB.h"
#include "LSQ.h"
#include "branchPredictor.h"
#include "PC.h"
#include "registers.h"

const uint32_t TERMINATION_INSTRUCTION = 0x0ff00513;

class CPU {
private:
    Storage storage;
    RegisterStatus reg_status;
    RSGroup rs;
    ROB rob;
    LSQ lsq;
    BranchPredictor bp;
    PC pc;
    Register reg_file;
    bool termination_seen = false;
    bool halted = false;
    long cycles = 0;
    long global_commit_counter = 0;

    static bool is_mem_op(Opcode op);
    static bool is_store_op(Opcode op);
    static bool is_branch_op(Opcode op);
    static AluOp branch_aluop(Opcode op); // 把opcode转换成aluop
    static bool regular_aluop(Opcode op, AluOp &out, bool &uses_rs2); // 把opcode转换成aluop
    static void mem_size(Opcode op, int &size, bool &is_unsigned);

    void step_once();

public:
    void load_program(std::istream &input);
    uint8_t run();
    long cycle_count() const;
    double branch_accuracy() const;
};