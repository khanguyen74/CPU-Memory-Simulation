#include "Memory.h"
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#include <stdexcept>



using namespace std;
Memory::Memory(int readPipe, int writePipe, vector<string> instructions): readPipe(readPipe), writePipe(writePipe){
	
	readInstruction(instructions);
	run();
}


void Memory::readInstruction(const vector<string> instructions){
	int memoryIndex=0;
	string instruction;
	int i = 0;
	while ( i < instructions.size()){
		instruction = instructions[i];
		if (instruction[0] == '.') // load different  memory
		{
			string subStr = instruction.substr(1, instruction.size()-1);
			memoryIndex = atoi(subStr.c_str()); // converts to cString to make it work with atoi	
			i++; // read next instruction
		}
		memory[memoryIndex++] = atoi(instructions[i++].c_str());
	}
	
	//finish loading all instruction
	sendReadySignalToCpu();

}

void Memory::run(){
	char command;
	do{
		if (read(readPipe, &command, sizeof(command) ) <=0 ){
			throw runtime_error("Memory cannot read from pipe");
		}

		executeCommand(command);	
	}while(isNotTerminated(command));
	printf("Ending memory process\n");

}


void Memory::executeCommand(const char command){
	if (isWriteCommand(command)){
		int val, address;
		read(readPipe, &address, sizeof(int));
		read(readPipe, &val, sizeof(int));
		memory[address] = val;

	}else if(isReadCommand(command)){
		int val, address;
		read(readPipe, &address, sizeof(int));
		val = memory[address];
		write(writePipe, &val, sizeof(int));
	}



}


bool Memory::isNotTerminated(const char command){
	// check for end command (E)
	return (command != 'E');	


}

bool Memory::isWriteCommand(const char command){
	return (command == 'W');
}

bool Memory::isReadCommand(const char command){

	return (command == 'R');
}

void Memory::sendReadySignalToCpu(){
	char signal = 'r';
	write(writePipe, &signal, sizeof(char));
}
