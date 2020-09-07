#include <sys/types.h>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <string>
#include <ctype.h>
#include "CPU.h"
#include <cmath>
#include <vector>
#include "Memory.h"
#include <exception>

using namespace std;

vector<string> getInstructionsFromFile(const string&);
int main(int argc, char*argv[]);

int main(int argc, char* argv[]){
	int i=0;
	string input;


	if (argc < 3){
		printf("Must include input file path as first argument and timer as second argument\n");
		exit(-1);
	
}
	int timer = atoi(argv[2]);
	//char buf[20];
	// file descriptor for Mem to send signal to CPU
	int MemToCpu[2];

	// file descriptor for Cpu to send signal to memory
	int CpuToMem[2];
	
	
	if ((pipe(MemToCpu) == -1) || (pipe(CpuToMem) == -1))
	{
		// pipe failed
		exit(-1);
	}
	try {
	
		pid_t pid = fork();
		if (pid == -1){
			printf("fork failed!\n");
			exit(-1);
		}
		else if (pid == 0) // Child, Memory process
		{
			
			int inst; 
			close(MemToCpu[0]); // Memory doesn't need to read this pipe
			close(CpuToMem[1]); // Memory doesn't write to this pipe

			vector<string> instructions = getInstructionsFromFile(argv[1]);	

			Memory memory(CpuToMem[0], MemToCpu[1], instructions);
			exit(0);
		}else { // parent, CPU process
		
		
			close(CpuToMem[0]); // CPU doesn't read this pipe
			close(MemToCpu[1]); // CPU doesn't write to this pipe
			int inst, val;
			CPU cpu(MemToCpu[0], CpuToMem[1], timer);

			waitpid(-1, NULL, 0);
		}
	
	}catch(exception& e){
	
		cout << e.what() <<"\n";
	}
	
  	return 0;

}



vector<string> getInstructionsFromFile(const string& path){

	ifstream inFile(path.c_str());
	string line;
	int instruction;
	vector <string> instructions;
	while (getline(inFile, line )){
		if (line[0] == '.'){ // if load operation
			string str = line.substr(1, line.size());
			istringstream iss(str);
			if (iss >> instruction){ // if a number appears first in string, get that number
				ostringstream oss;
				oss << '.' << instruction;
				instructions.push_back(oss.str());
			}
		}else{	// regular instruction
			
			istringstream iss(line);
			if (iss >> instruction){ // if a number appears first in string, get that number
				ostringstream oss;
				oss << instruction;
				instructions.push_back(oss.str());
			}
		}
	}
	inFile.close();
	return instructions;
}
