#include "CPU.h"
#include <iostream>
#include <bitset>
#include <stdio.h>
#include<stdlib.h>
#include <string>
#include<fstream>
#include <sstream>
using namespace std;


int main(int argc, char* argv[])
{
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
	
		// decode
		done = myCPU.Decode(&instr); 
	    if (done == false) { //bad instruction
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

        if (myCPU.readPC() >= maxPC) {
			break;
		}
	}

	// print the results
	int a0 = myCPU.registerFile[10];  // a0 is x10
    int a1 = myCPU.registerFile[11];  // a1 is x11  
	cout << "(" << dec << a0 << "," << dec << a1 << ")" << endl;
	
	return 0;

}
