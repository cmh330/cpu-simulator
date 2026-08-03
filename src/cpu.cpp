#include "../include/cpu.h"

bool CPU::is_mem_op(Opcode op) {
    return (op == Opcode::LB || op == Opcode::LBU || op == Opcode::LH || 
            op == Opcode::LHU || op == Opcode::LW || 
            op == Opcode::SB || op == Opcode::SH || op == Opcode::SW);
}

bool CPU::is_store_op(Opcode op) {
    return (op == Opcode::SB || op == Opcode::SH || op == Opcode::SW);
}

bool CPU::is_branch_op(Opcode op) {
    return (op == Opcode::BEQ || op == Opcode::BGEU || op == Opcode::BLT ||
            op == Opcode::BLTU || op == Opcode::BNE ||op == Opcode::BGE);
}

AluOp CPU::branch_aluop(Opcode op) {
    switch (op) {
        case Opcode::BEQ: return AluOp::BEQ;
        case Opcode::BGEU: return AluOp::BGEU;
        case Opcode::BLT: return AluOp::BLT;
        case Opcode::BLTU: return AluOp::BLTU;
        case Opcode::BNE: return AluOp::BNE;
        case Opcode::BGE: return AluOp::BGE;
    }
}

bool CPU::regular_aluop(Opcode op, AluOp &out, bool &uses_rs2) {
    uses_rs2 = false;
    switch (op) {
        case Opcode::ADD: out = AluOp::ADD; uses_rs2 = true; return true;
        case Opcode::SUB: out = AluOp::SUB; uses_rs2 = true; return true;
        case Opcode::AND: out = AluOp::AND; uses_rs2 = true; return true;
        case Opcode::OR: out = AluOp::OR; uses_rs2 = true; return true;
        case Opcode::XOR: out = AluOp::XOR; uses_rs2 = true; return true;
        case Opcode::SLL: out = AluOp::SLL; uses_rs2 = true; return true;
        case Opcode::SRL: out = AluOp::SRL; uses_rs2 = true; return true;
        case Opcode::SRA: out = AluOp::SRA; uses_rs2 = true; return true;
        case Opcode::SLT: out = AluOp::SLT; uses_rs2 = true; return true;
        case Opcode::SLTU: out = AluOp::SLTU; uses_rs2 = true; return true;
        case Opcode::ADDI: out = AluOp::ADD; return true;
        case Opcode::ANDI: out = AluOp::AND; return true;
        case Opcode::ORI: out = AluOp::OR; return true;
        case Opcode::XORI: out = AluOp::XOR; return true;
        case Opcode::SLLI: out = AluOp::SLL; return true;
        case Opcode::SRLI: out = AluOp::SRL; return true;
        case Opcode::SRAI: out = AluOp::SRA; return true;
        case Opcode::SLTI: out = AluOp::SLT; return true;
        case Opcode::SLTIU: out = AluOp::SLTU; return true;
        // LUI/AUIPC/JAL/JALR的写rd部分，都用ADD(vj+0)实现
        case Opcode::LUI: case Opcode::AUIPC: case Opcode::JAL: case Opcode::JALR: out = AluOp::ADD; return true;
        default: return false;
    }
}

void CPU::mem_size(Opcode op, int &size, bool &is_unsigned) {
    switch (op) {
        case Opcode::LB: size = 1; is_unsigned = false; break;
        case Opcode::LBU: size = 1; is_unsigned = true; break;
        case Opcode::LH: size = 2; is_unsigned = false; break;
        case Opcode::LHU: size = 2; is_unsigned = true; break;
        case Opcode::SB: size = 1; is_unsigned = false; break;
        case Opcode::SH: size = 2; is_unsigned = false; break;
        default: size = 4; is_unsigned = false; break;
    }
}

void CPU::step_once() {
    // 1: 读old
    int rob_head = rob.head_tag();
    ROB::CommitResult commit = rob.get_commit();

    int flush_tag = NO_TAG;
    uint32_t flush_correct_pc = 0;
    bool is_mispredict = false;
    if (commit.tag != NO_TAG && commit.type == RobType::BRANCH && commit.mispredict) {
        flush_tag = commit.tag;
        flush_correct_pc = commit.correct_pc;
        is_mispredict = true;
    }
    int store_commit_tag = NO_TAG;
    if (commit.tag != NO_TAG && commit.type == RobType::STORE) {
        store_commit_tag = commit.tag;
    }

    bool commit_regular = (commit.tag != NO_TAG && commit.type == RobType::REGULAR);
    bool commit_branch = (commit.tag != NO_TAG && commit.type == RobType::BRANCH);
 
    BranchResult branch_result = rs.get_branch_result();

    CdbBroadcast from_rs = rs.get_broadcast();
    CdbBroadcast from_lsq = lsq.get_broadcast(storage, rob_head);
    CdbBroadcast cdb = Cdb::choose(from_rs, from_lsq);

    // 2: issue, mispredict的话不发布
    bool issue_valid = false;
    DecodedInstruction instruction;
    uint32_t fetch_pc = pc.get();
    bool is_termination = false;
    bool is_mem = false, is_branch = false;

    if (!termination_seen && !is_mispredict) {
        uint32_t raw = storage.read_word(fetch_pc);
        if (raw == TERMINATION_INSTRUCTION) {
            is_termination = true;
        }
        else {
            instruction = decode(raw);
            is_mem = is_mem_op(instruction.opcode);
            is_branch = is_branch_op(instruction.opcode);
            bool rob_ok = !rob.is_full();
            bool slot_ok = is_mem ? !lsq.is_full() : !rs.is_full();
            if (rob_ok && slot_ok) issue_valid = true;
        }
    }

    int predicted_tag = rob.tail_tag(); // 如果发布，这条指令的标签

    int qj = NO_TAG, qk = NO_TAG;
    int32_t vj = 0, vk = 0;
    if (issue_valid) {
        qj = reg_status.get_producer(instruction.rs1);
        if (qj == NO_PRODUCER) { 
            vj = reg_file.get(instruction.rs1); 
            qj = NO_TAG; 
        }
        else if (qj == cdb.tag) { 
            // 同拍广播
            vj = cdb.value; 
            qj = NO_TAG; 
        }
        else if (rob.get(qj).ready) {
            // 之前已算完,还没提交
            vj = rob.get(qj).value; 
            qj = NO_TAG; 
        }
        
        qk = reg_status.get_producer(instruction.rs2);
        if (qk == NO_PRODUCER) {
            vk = reg_file.get(instruction.rs2); 
            qk = NO_TAG;
        }
        else if (qk == cdb.tag) {
            vk = cdb.value;
            qk = NO_TAG;
        }
        else if (rob.get(qk).ready) {
            vk = rob.get(qk).value;
            qk = NO_TAG;
        }
    }

    // 按指令类型算具体参数
    bool issue_to_rs = false;
    AluOp rs_op = AluOp::ADD;
    int32_t rs_vj = 0, rs_vk = 0;
    int rs_qj = NO_TAG, rs_qk = NO_TAG;
    bool rs_is_branch = false;

    bool issue_to_lsq = false;
    bool lsq_is_store = false;
    int32_t lsq_vj = 0, lsq_vk = 0, lsq_imm = 0;
    int lsq_qk = NO_TAG, lsq_qj = NO_TAG;
    int lsq_size = 4;
    bool lsq_unsigned = false;

    RobType rob_issue_type = RobType::REGULAR;
    int rob_issue_dest_reg = 0; // 结果写进哪个寄存器
    int rob_issue_target = 0; // 如果分支要跳转跳到哪(pc + imm)
    bool rob_issue_predict = false; // BranchPredictor预测它会不会跳转

    int next_pc_if_issued = fetch_pc + 4;

    if(issue_valid) {
        if (is_mem) {
            issue_to_lsq = true;
            bool store = is_store_op(instruction.opcode);
            lsq_is_store = store;
            int size;
            bool unsigned_;
            mem_size(instruction.opcode, size, unsigned_);
            lsq_size = size;
            lsq_unsigned = unsigned_;
            lsq_qj = qj;
            lsq_vj = vj;
            lsq_imm = instruction.imm;
            if (store) {
                lsq_qk = qk;
                lsq_vk = vk;
            }
            rob_issue_type = store ? RobType::STORE : RobType::REGULAR;
            rob_issue_dest_reg = store ? 0 : instruction.rd;
        }
        else if (is_branch) {
            issue_to_rs = true;
            rs_is_branch = true;
            rs_op = branch_aluop(instruction.opcode);
            rs_vj = vj;
            rs_vk = vk;
            rs_qj = qj;
            rs_qk = qk;
            bool predict = bp.predict(fetch_pc);
            uint32_t target = fetch_pc + static_cast<uint32_t>(instruction.imm);
            rob_issue_type = RobType::BRANCH;
            rob_issue_target = target;
            rob_issue_predict = predict;
            next_pc_if_issued = predict ? target : (fetch_pc + 4);
        }
        else if (instruction.opcode == Opcode::JAL) {
            issue_to_rs = true;
            rs_op = AluOp::ADD;
            rs_vj = static_cast<int32_t>(fetch_pc + 4);
            rs_vk = 0;
            rs_qj = NO_TAG;
            rs_qk = NO_TAG;
            next_pc_if_issued = fetch_pc + static_cast<uint32_t>(instruction.imm);
            rob_issue_type = RobType::REGULAR;
            rob_issue_dest_reg = instruction.rd;
        }
        else if (instruction.opcode == Opcode::JALR) {
            // rs1就绪才能发布
            if (qj != NO_TAG) {
                issue_valid = false;
            }
            else {
                issue_to_rs = true;
                rs_op = AluOp::ADD;
                rs_vj = static_cast<int32_t>(fetch_pc + 4);
                rs_vk = 0;
                rs_qj = NO_TAG;
                rs_qk = NO_TAG;
                next_pc_if_issued = static_cast<uint32_t>(vj + instruction.imm) & ~1u;
                rob_issue_type = RobType::REGULAR;
                rob_issue_dest_reg = instruction.rd;
            }
        }
        else if (instruction.opcode == Opcode::AUIPC) {
            issue_to_rs = true;
            rs_op = AluOp::ADD;
            rs_vj = static_cast<int32_t>(fetch_pc) + instruction.imm;
            rs_vk = 0;
            rs_qj = NO_TAG;
            rs_qk = NO_TAG;
            rob_issue_type = RobType::REGULAR;
            rob_issue_dest_reg = instruction.rd;
        }
        else if (instruction.opcode == Opcode::LUI) {
            issue_to_rs = true;
            rs_op = AluOp::ADD;
            rs_vj = instruction.imm;
            rs_vk = 0;
            rs_qj = NO_TAG;
            rs_qk = NO_TAG;
            rob_issue_type = RobType::REGULAR;
            rob_issue_dest_reg = instruction.rd;
        }
        else {
            AluOp op;
            bool uses_rs2 = false;
            if (regular_aluop(instruction.opcode, op, uses_rs2)) {
                issue_to_rs = true;
                rs_op = op;
                rs_vj = vj;
                rs_qj = qj;
                if (uses_rs2) {
                    rs_vk = vk;
                    rs_qk = qk;
                } else {
                    rs_vk = instruction.imm;
                    rs_qk = NO_TAG;
                }
                rob_issue_type = RobType::REGULAR;
                rob_issue_dest_reg = instruction.rd;
            } else {
                std::cout << "invalid opcode\n";
                issue_valid = false;
            }
        }
    }

    int final_tag = issue_valid ? predicted_tag : NO_TAG;

    // 3: step
    int store_ready_tag = lsq.get_store_ready_tag(rob_head);
    int regs_status_clear_tag = commit_regular ? commit.tag : NO_TAG;
    rs.step(cdb, rs_op, rs_vj, rs_qj, rs_vk, rs_qk,
            issue_to_rs ? final_tag : NO_TAG, rs_is_branch,
            flush_tag, rob_head);
    if (store_commit_tag != NO_TAG) {
        ++global_commit_counter;
    }
    lsq.step(storage, cdb, store_commit_tag, rob_head, lsq_is_store, 
             lsq_qj, lsq_vj, lsq_imm, lsq_qk, lsq_vk, lsq_size, lsq_unsigned, 
             issue_to_lsq ? final_tag : NO_TAG, flush_tag, global_commit_counter);
    rob.step(cdb, branch_result.tag, branch_result.actual_jump, store_ready_tag, 
             issue_valid, rob_issue_type, rob_issue_dest_reg, fetch_pc, rob_issue_target, rob_issue_predict, flush_tag);
    reg_status.step(regs_status_clear_tag, issue_valid ? rob_issue_dest_reg : 0,
                    issue_valid ? final_tag : NO_TAG, flush_tag ,rob_head);
    reg_file.step(commit_regular, commit.dest_reg, commit.value);
    bp.step(commit_branch, commit.type==RobType::BRANCH ? rob.get(commit.tag).pc : 0,
            commit.type==RobType::BRANCH ? rob.get(commit.tag).predict_jump : false,
            commit.type==RobType::BRANCH ? rob.get(commit.tag).actual_jump : false);
    pc.step(is_mispredict, flush_correct_pc, issue_valid, next_pc_if_issued);

    // 4: sync
    rs.sync();
    lsq.sync();
    rob.sync();
    reg_status.sync();
    reg_file.sync();
    bp.sync();
    pc.sync();

    if (is_termination) termination_seen = true;
}


void CPU::load_program(std::istream &input) {
    storage.load(input);
}

uint8_t CPU::run() {
    while (!halted) {
        step_once();
        ++cycles;
        if (termination_seen && rob.is_empty()) halted = true;
    }
    return static_cast<uint8_t>(reg_file.get(10) & 0xFF);
}

long CPU::cycle_count() const {
    return cycles;
}
    
double CPU::branch_accuracy() const {
    return bp.accuracy();
}