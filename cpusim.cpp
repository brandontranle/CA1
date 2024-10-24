#include "CPU.h"

#include <iostream>
#include <bitset>
#include <stdio.h>
#include<stdlib.h>
#include <string>
#include<fstream>
#include <sstream>
using namespace std;

/*
Add all the required standard and developed libraries here
*/

/*
Put/Define any helper function/definitions you need here
*/
int main(int argc, char* argv[])
{
	/* This is the front end of your project.
	You need to first read the instructions that are stored in a file and load them into an instruction memory.
	*/

	/* Each cell should store 1 byte. You can define the memory either dynamically, or define it as a fixed size with size 4KB (i.e., 4096 lines). Each instruction is 32 bits (i.e., 4 lines, saved in little-endian mode).
	Each line in the input file is stored as an hex and is 1 byte (each four lines are one instruction). You need to read the file line by line and store it into the memory. You may need a mechanism to convert these values to bits so that you can read opcodes, operands, etc.
	*/



	char instMem[4096];


	if (argc < 2) {
		cout << "No file name entered. Exiting...";
		return -1;
	}

	ifstream infile(argv[1]); //open the file
	if (!(infile.is_open() && infile.good())) {
		cout<<"error opening file\n";
		return 0; 
	}
	string line; 
	int i = 0;
    while (getline(infile, line)) {
			if (line.length() != 2) {
			//cout << "line is" + line << endl;
            cout << "Invalid input line length. Expected 2 characters per line.\n";
            return -1;
        }

        // Convert the hex string (2 characters) to a byte and store in memory
        char byte = (char)strtol(line.c_str(), nullptr, 16);
        instMem[i] = byte;
        i++;
		}
	
	

	int maxPC= i; 
	
	CPU myCPU;  

	bitset<32> myInst; //
	instruction instr = instruction(myInst);

	bool done = true;
	while (done == true) // processor's main loop. Each iteration is equal to one clock cycle.  
	{
		//fetch instruction
		myInst = myCPU.Fetch(instMem);
		instr = instruction(myInst);

		//cout << "Fetched instruction (in hex): " << hex << myInst.to_ulong() << endl;
	
		// decode
		done = myCPU.Decode(&instr);
	    if (done == false) {
		    break;
		}
		
		// execute
		myCPU.Execute();
		
		//writeback
		myCPU.WriteBack();

		if (!myCPU.pcUpdated)
			myCPU.incPC();
		else 
			myCPU.pcUpdated = false;

		// ... 
        if (myCPU.readPC() >= maxPC) {
			//cout << "PC is reached. " << endl;
			break;
		}
	}

	// print the results
	 int a0 = myCPU.registerFile[10];  // a0 is x10
     int a1 = myCPU.registerFile[11];  // a1 is x11  
	// print the results (you should replace a0 and a1 with your own variables that point to a0 and a1)
	  cout << "(" << dec << a0 << "," << dec << a1 << ")" << endl;
	
	return 0;

}
