#include <iostream>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstdint>
#include <cctype>

using namespace std;


 class instruction { // optional
 public:
 	bitset<32> instr;//instruction
 	instruction(bitset<32> fetch); // constructor
 }; 

class CPU {
private:
	int dmemory[4096]; //data memory byte addressable in little endian fashion;
	unsigned long PC; //pc 
	//instantiation of instruction
	//FSM initialize everything to 0
	struct DECODE_STAGE {
		int32_t rs1 = 0;
		int32_t rs2 = 0;
		int32_t rd = 0;
		int32_t imm = 0;
		int32_t opcode = 0;
		int32_t funct3 = 0;
		int32_t funct7 = 0;
		string op_type = "";
	} decodedInstr;

	struct EXECUTE_STAGE {
		int32_t rs1 = 0;
		int32_t rs2 = 0;
		int32_t rd = 0;
		int32_t alu_result = 0;
		string op_type = "";
    } executeInstr;




public:
	CPU();
	int32_t registerFile[32];
	unsigned long readPC();
	void incPC();
	bitset<32> Fetch(const char* instMem);
	bool Decode(instruction* instr);
	void Execute();
	void WriteBack();
	bool pcUpdated = false;

};
