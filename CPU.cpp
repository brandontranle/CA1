#include "CPU.h"
#include <sstream>
#include <iomanip>



instruction::instruction(bitset<32> fetch)
{
	instr = fetch;
}

CPU::CPU()
{
	PC = 0; //set PC to 0
	for (int i = 0; i < 4096; i++) //copy instrMEM
	{
		dmemory[i] = (0);
	}

    for (int i = 0; i < 32; i++)
        registerFile[i] = (0);
}


unsigned long CPU::readPC()
{
	return PC;
}
void CPU::incPC()
{
	PC+=4;
}

void CPU::writePC(unsigned long value) {
    PC = value;
}


bitset<32> CPU::Fetch(const char* instMem) {
    unsigned long pc = readPC(); // Fetch the current PC value

    if (pc + 3 >= 4096) {
        cout << "Error: PC out of bounds during fetch. PC = 0x" << hex << pc << endl;
        return bitset<32>(0);  // Return 0 as a NOP (No Operation) in case of out-of-bounds fetch
    }

    // Read 4 bytes from the instruction memory (little-endian)
    unsigned char byte0 = instMem[pc];
    unsigned char byte1 = instMem[pc + 1];
    unsigned char byte2 = instMem[pc + 2];
    unsigned char byte3 = instMem[pc + 3];

    // Combine bytes into a 32-bit instruction (little-endian order)
    unsigned int instr = (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0;
    return bitset<32>(instr);
}


bool CPU::Decode(instruction* instr) {
    bitset<32> inst = instr->instr;  // Access the instruction from the instruction object

    // Decode basic fields: opcode, rd, funct3, funct7, rs1, rs2
    decodedInstr.opcode = (inst.to_ulong() & 0x7F);
    decodedInstr.rd = (inst.to_ulong() >> 7) & 0x1F;
    decodedInstr.funct3 = (inst.to_ulong() >> 12) & 0x07;
    decodedInstr.funct7 = (inst.to_ulong() >> 25) & 0x7F;

    uint32_t rs1_addr = (inst.to_ulong() >> 15) & 0x1F;
    uint32_t rs2_addr = (inst.to_ulong() >> 20) & 0x1F;

    // Fetch register values from the register file
    decodedInstr.rs1 = registerFile[rs1_addr];
    decodedInstr.rs2 = registerFile[rs2_addr];

    // Handle immediate and instruction types
    switch (decodedInstr.opcode) {
        case 0x33: // R-type (e.g., ADD, SUB, OR, AND)
        {
            if (decodedInstr.funct3 == 0x0) {
                if (decodedInstr.funct7 == 0x0)
                    decodedInstr.op_type = "ADD";
                else if (decodedInstr.funct7 == 0x20)
                    decodedInstr.op_type = "SUB";
                else
                    decodedInstr.op_type = "NULL";
            } else if (decodedInstr.funct3 == 0x6 && decodedInstr.funct7 == 0x0)
                decodedInstr.op_type = "OR";
            else if (decodedInstr.funct3 == 0x7 && decodedInstr.funct7 == 0x0)
                decodedInstr.op_type = "AND";
            else if (decodedInstr.funct3 == 0x4 && decodedInstr.funct7 == 0x0)
                decodedInstr.op_type = "XOR";
            else
                decodedInstr.op_type = "NULL";
            break;
        }

        case 0x13: // I-type (e.g., ORI, ADDI, SRAI)
        {
            decodedInstr.imm = (inst.to_ulong() >> 20) & 0xFFF;  // 12-bit immediate
            decodedInstr.imm = (decodedInstr.imm & 0x800) ? (decodedInstr.imm | 0xFFFFF000) : decodedInstr.imm;  // sign-extend

            if (decodedInstr.funct3 == 0x0) {
                decodedInstr.op_type = "ADDI";
            }
            else if (decodedInstr.funct3 == 0x6) // ORI
                decodedInstr.op_type = "ORI";
            else if (decodedInstr.funct3 == 0x5) { // SRAI
                decodedInstr.op_type = "SRAI";
                decodedInstr.imm = (inst.to_ulong() >> 20) & 0x1F;  // Extract 5-bit shift amount
            }
            break;
        }

        case 0x3: // Load (LB, LW)
        {
            decodedInstr.imm = (inst.to_ulong() >> 20) & 0xFFF;  // Extract 12-bit immediate
            decodedInstr.imm = (decodedInstr.imm & 0x800) ? (decodedInstr.imm | 0xFFFFF000) : decodedInstr.imm;  // Sign-extend

            if (decodedInstr.funct3 == 0x0) // LB
                decodedInstr.op_type = "LB";
            else if (decodedInstr.funct3 == 0x2) // LW
                decodedInstr.op_type = "LW";
            else {
                decodedInstr.op_type = "NULL";
                cerr << "Error: unknown funct3 for Load instruction" << endl;
            }
            break;
        }

        case 0x23: // Store (SW, SB)
        {
            int imm11_5 = (inst.to_ulong() >> 25) & 0x7F;
            int imm4_0 = (inst.to_ulong() >> 7) & 0x1F;
            decodedInstr.imm = (imm11_5 << 5) | imm4_0;  // concat the two halves of the immediate
            decodedInstr.imm = (decodedInstr.imm & 0x800) ? (decodedInstr.imm | 0xFFFFF000) : decodedInstr.imm;  // sign-extend

            if (decodedInstr.funct3 == 0x0) // SB
                decodedInstr.op_type = "SB";
            else if (decodedInstr.funct3 == 0x2) // SW
                decodedInstr.op_type = "SW";
            else
                decodedInstr.op_type = "NULL";
            break;
        }

        case 0x6F: // J-type (JAL)
        {
            int imm20 = (inst.to_ulong() >> 31) & 0x1;
            int imm101 = (inst.to_ulong() >> 21) & 0x3FF;
            int imm11 = (inst.to_ulong() >> 20) & 0x1;
            int imm1912 = (inst.to_ulong() >> 12) & 0xFF;
            decodedInstr.imm = (imm20 << 20) | (imm1912 << 12) | (imm11 << 11) | (imm101 << 1);
            decodedInstr.imm = (decodedInstr.imm & 0x100000) ? (decodedInstr.imm | 0xFFE00000) : decodedInstr.imm;  // sign-extend
            decodedInstr.op_type = "JAL";
            break;
        }

        case 0x63: // B-type (BEQ)
        {
            int imm12 = (inst.to_ulong() >> 31) & 0x1;
            int imm10_5 = (inst.to_ulong() >> 25) & 0x3F;
            int imm4_1 = (inst.to_ulong() >> 8) & 0xF;
            int imm11 = (inst.to_ulong() >> 7) & 0x1;
            decodedInstr.imm = (imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1);
            decodedInstr.imm = (decodedInstr.imm & 0x1000) ? (decodedInstr.imm | 0xFFFFE000) : decodedInstr.imm;  // sign-extend

            if (decodedInstr.funct3 == 0x0)
                decodedInstr.op_type = "BEQ";
            break;
        }

        case 0x37: // LUI
        {
            //decodedInstr.imm = (inst.to_ulong() & 0xFFFFF000);  // LUI uses the upper 20 bits as immediate
            decodedInstr.op_type = "LUI";
            break;
        }
        case 0x0: // ZERO
        {
            decodedInstr.op_type = "ZERO";
            break;
        }
        default: {
            cout << "Error: 0x" << hex << inst.to_ulong() << endl;
            //cout << "ERROR: Unknown opcode: " << decodedInstr.opcode << endl;
            decodedInstr.op_type = "NULL";
            cerr << "Error: Unknown opcode encountered during decode." << endl;
            return false;
            }
    }

    /*
    // Debug output for the decoded instruction
    cout << "Decoded instruction: " + decodedInstr.op_type << endl;
    cout << "Opcode: " << decodedInstr.opcode << endl;
    cout << "rs1: " << decodedInstr.rs1 << endl;
    cout << "rs2: " << decodedInstr.rs2 << endl;
    cout << "rd: " << decodedInstr.rd << endl;
    cout << "Immediate: " << decodedInstr.imm << endl;
    */

    //cout << "Successful Instruction: " << hex << inst.to_ulong() << endl;

    return true;
}



void CPU::Execute() {
    //cout << "Executing instruction: " << decodedInstr.op_type << endl;
    executeInstr.op_type = decodedInstr.op_type;
    executeInstr.rd = decodedInstr.rd;
    executeInstr.rs2 = decodedInstr.rs2;

    string curr = executeInstr.op_type;

    if (curr == "ADD") {
        // R-type: Add values from rs1 and rs2
        executeInstr.alu_result = decodedInstr.rs1 + decodedInstr.rs2;
    } else if (curr == "SUB") {
        // R-type: Subtract values from rs2 from rs1
        executeInstr.alu_result = decodedInstr.rs1 - decodedInstr.rs2;
    }
    else if (curr == "ADDI"){
        executeInstr.alu_result = decodedInstr.rs1 + decodedInstr.imm;
    }
    else if (curr == "LUI") {
        // LUI: Load upper immediate (shift left by 12 bits)
        executeInstr.alu_result = decodedInstr.imm << 12;
    } 
    else if (curr == "ORI") {
        // I-type: ORI applies the OR between rs1 and the immediate
        executeInstr.alu_result = decodedInstr.rs1 | decodedInstr.imm;
    } 
    else if (curr == "XOR") {
        // R-type: XOR between rs1 and rs2
        executeInstr.alu_result = decodedInstr.rs1 ^ decodedInstr.rs2;
    } 
    else if (curr == "SRAI") {
        // I-type: Shift right arithmetic by the lower 5 bits of the immediate
        executeInstr.alu_result = decodedInstr.rs1 >> (decodedInstr.imm & 0x1F);
    } 
    else if (curr == "LB") {
        // Load Byte: Compute the address and load the byte from memory
        int32_t address = decodedInstr.rs1 + decodedInstr.imm;
        int8_t byte = dmemory[address];
        executeInstr.alu_result = (int32_t) (int8_t) (byte);  // sign-extend the byte to 32 bits        
        //cout << "Loaded byte from address: " << address << " Value: " << hex << static_cast<int>(byte) << endl;
    } 
    else if (curr == "LW"){
        int32_t address = decodedInstr.rs1 + decodedInstr.imm;
        uint32_t word = ((((dmemory[address + 3]) << 24)) | 
                         ((dmemory[address + 2]) << 16) | 
                         ((dmemory[address + 1]) << 8) | 
                         (dmemory[address])); 
        executeInstr.alu_result = static_cast<int32_t>(word);  
    } else if (curr == "SW"){
        // Store word into memory (32-bit)
        int32_t address = decodedInstr.rs1 + decodedInstr.imm;  // The address is calculated in Execute()
        uint32_t word_to_store = executeInstr.rs2;  // The value to store is in rs2

        /*
        dmemory[address + 3] = (word_to_store >> 24) & 0xFF000000;    
        dmemory[address + 2] = (word_to_store >> 16) & 0xFF0000;
        dmemory[address + 1] = (word_to_store >> 8) & 0xFF00;
        dmemory[address] = (word_to_store >> 0) & 0xFF;  // The last byte is stored in the lower 8 bits of rs2
        */
        dmemory[address] = word_to_store;
    }
    else if (curr == "SB") {
        // Store Byte: Compute the effective address and store the byte
        int32_t address = decodedInstr.rs1 + decodedInstr.imm;

        // Store byte into memory (8-bit)
        uint8_t byte_to_store = static_cast<uint8_t>(executeInstr.rs2 & 0xFF);  // The byte to store is in the lower 8 bits of rs2

        // Store the byte into memory
        dmemory[address] = byte_to_store;
    } 
    else if (curr == "BEQ") {
        // Branch Equal: If rs1 == rs2, branch to the target address
        if (decodedInstr.rs1 == decodedInstr.rs2) {
            int32_t branch_offset = decodedInstr.imm;
            PC += branch_offset; 
            pcUpdated = true;
            //this is to counteract the automated incPC() called in cpusim.cpp
            //cout << "Branch taken: PC updated to: " << hex << readPC() + branch_offset << endl;
        }/*
         else {
            executeInstr.alu_result = readPC();
            //cout << "Branch not taken: PC incremented to: " << hex << executeInstr.alu_result << endl;
        }*/
    } 
    else if (curr == "JAL") {
        // Jump and Link: Write the return address (PC + 4) to rd, and update PC to target
        executeInstr.alu_result = PC + 4;
        int32_t jump_offset = decodedInstr.imm;
        PC += jump_offset; 
        pcUpdated = true;
    
    } else if (curr == "ZERO") {
        // Zero: Set the ALU result to 0
        executeInstr.alu_result = 0;
    }
}

void CPU::WriteBack() {
    // Use the operation type from the memory stage (since WriteBack happens after MemoryAccess)
    string curr = executeInstr.op_type;

    // Avoid writing to register x0 (always zero in RISC-V) && SW, SB do not write to register
    if (executeInstr.rd == 0 || executeInstr.op_type == "SW" || executeInstr.op_type == "SB") {
        //cout << "WriteBack: No register writeback needed for instruction " << curr << endl;
        return;
    }
    

     if (curr != "NULL"  && curr != "SB" && curr != "SW" && curr != "ZERO") {
        registerFile[executeInstr.rd] = executeInstr.alu_result;  // Write ALU result
        //cout << "WriteBack: Writing result to register x" << executeInstr.rd << ": " << registerFile[executeInstr.rd] << endl;
    }
    
    /*
    else {
        // No writeback for instructions like SW, SB (store instructions)
        //cout << "WriteBack: No register writeback needed for instruction " << curr << endl;
    }*/
}





