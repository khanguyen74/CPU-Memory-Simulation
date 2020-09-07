#include <stdio.h>
#include "CPU.h"
#include <unistd.h>
#include <stdexcept>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <sstream>

using namespace std;
CPU::CPU(int readPipe, int writePipe, int timer): readPipe(readPipe), writePipe(writePipe), timer(timer){
	PC =IR = AC = X =Y = 0;
	SP = USER_PROGRAM_STACK;
	counter = timer;
	isKernelMode = false;
	waitForMemory();
	printf("Memory ready, start\n");
	run();
	sendEndCommand();
}


int CPU::getPC(){
	return PC;
}


void CPU::run(){
	// loop until meets terminate instruction
	do{
		
		checkTimerInterrupt();
		IR = fetchInstruction();
		executeInstruction();
		if (!isKernelMode){
			counter--;
		}
	
	}while(IR != TERMINATE_INSTRUCTION);
	
}

void CPU::waitForMemory(){
	char signal;

	if (read(readPipe, &signal, sizeof(char)) <=0 ){
		throw runtime_error("Memory cannot read from pipe");
	}
	if (signal != 'r'){
		throw runtime_error("First signal receive from memory supposed to be ready signal");
	}


}

int CPU::fetchInstruction(){
	// read instruction at location stored in PC from memory then increment PC
	return readMemory(PC++);


}

int CPU::readMemory(const int address){
	checkMemoryViolation(address);
	char command = 'R';
	write(writePipe, &command, sizeof(char));
	write(writePipe, &address, sizeof(int));
	int value;
	read(readPipe, &value, sizeof(int));
	return value;
}


void CPU::checkMemoryViolation(const int address){

	if (!isKernelMode && address >= 1000){
		stringstream ss;
		ss << address;
		sendEndCommand();
		string message = "Memory violation: accessing system address " + ss.str() + " in user mode";
		throw runtime_error(message);
	}
}
/*
 * Write value to memory at address
 *
 * */
void CPU::writeMemory(const int address, const int value){
	char command = 'W';
	write(writePipe, &command, sizeof(char));
	write(writePipe, &address, sizeof(int));
	write(writePipe, &value, sizeof(int));



}

void CPU::executeInstruction(){
	int address;
	int port;
	switch (IR){
		case 1:// load value to AC
			AC = fetchInstruction();
		 	break;
		case 2: // load value at address to AC
			address = fetchInstruction();
			AC = readMemory(address);
			break;
		case 3: //load value from the address found in the given address into AC
			address = fetchInstruction();
			address = readMemory(address);
			AC = readMemory(address);
			break;
		case 4: // Load value at (address + X) into AC
			address = fetchInstruction();
			address += X;
			AC = readMemory(address);
			break;
		case 5: // load value at (address + Y)
			address = fetchInstruction();
			address += Y;
			AC = readMemory(address);
			break;
			
		case 6: // load from (SP + X)
			address = SP + X;
			AC = readMemory(address);	
			break;
		case 7: // Store value in AC to address
			address = fetchInstruction();
			writeMemory(address, AC);
			break;
		case 8: // get a random number from 1 to 100 into AC
			srand(time(0));
			AC = rand() % 100 + 1;
			break;
		case 9: // put port, if port = 1, write ac as int, port = 2, write ac as char
			port = fetchInstruction();
			if ( port == 1){
				printf("%d", AC);
			}else if(port == 2){
				printf("%c", AC);
			}else throw runtime_error("Invalid port is given");
			break;
		case 10: // add value in X to AC
			AC += X;
			break;

		case 11: // add value in Y to AC
			AC += Y;
			break;

		case 12: // subtract value in X from AC
			AC -= X;
			break;

		case 13: // substract value in Y from AC
			AC -= Y;
			break;
		case 14: // copy AC to X
			X = AC;
			break;
		case 15: // copy X to AC
			AC = X;
			break;
		
		case 16: // copy AC to Y
			Y = AC;
			break;
		case 17: // copy X to AC
			AC = Y;
			break;
		case 18: // copy value from AC to SP
			SP = AC;
			break;

		case 19: // copy from SP
			AC = SP;
			break;
		
		case 20: // jump address
			PC = fetchInstruction();
			break;

		case 21: // jump to address if AC is 0
			address = fetchInstruction();
			PC = (AC == 0) ? address : PC;
			break;

		case 22: // jump to address if AC is not 0
			address = fetchInstruction();
			PC = (AC != 0) ? address : PC;
			break;
		case 23: // push return address to stack, jump to address
			address = fetchInstruction();
			writeMemory(--SP, PC);
			PC = address;
			break;

		case 24: // pop return address from stack, jump to address
			address = readMemory(SP++);
			PC = address;
			break;

		case 25: // Increment X
			X++;
			break;

		case 26: // Decrement X
			X--;
			break;

		case 27: // Push AC onto Stack
			writeMemory(--SP, AC);
			break;

		case 28: // Pop from stack to AC
			AC = readMemory(SP++);
			break;

		case 29: // Perform system call
			saveAllRegisters();
			PC = SYSTEM_CALL_ADDRESS;	// set PC to start of system call routine address
			isKernelMode = true;
			break;

		case 30: // return from system call
			restoreAllRegisters();
			counter = timer;
			isKernelMode = false;
			break;
		case 50: 
	
			printf("Ending CPU\n");
			sendEndCommand();
			break;
		default:
			printf("Unknown command %d\n", IR);
			break;
	}
}

void CPU::sendEndCommand(){
	char command = 'E';
	write(writePipe, &command, sizeof(char));

}

void  CPU::checkTimerInterrupt(){
	if (counter==0 && !isKernelMode){
		isKernelMode = true;
		saveAllRegisters();
		PC = TIMER_INTERRUPT_ADDRESS;
	}

}


void CPU::saveAllRegisters(){
	int temp = SP;	// temporarily holds SP
	SP = SYSTEM_STACK;
	writeMemory(--SP, temp); // save SP first
	writeMemory(--SP, PC);
	writeMemory(--SP, AC);
	writeMemory(--SP, X);
	writeMemory(--SP, Y);
	//printf("Register saved : SP:%d PC:%d AC:%d X:%d Y: %d\n", temp, PC, AC, X, Y);
}

/**
 * This method restores all system registers from system stack, and reset timer
 *
 *
 */
void CPU:: restoreAllRegisters(){
	Y = readMemory(SP++);
	X = readMemory(SP++);
	AC = readMemory(SP++);
	PC = readMemory(SP++);
	SP = readMemory(SP++);
	//printf("Register restored: SP:%d PC:%d AC:%d X:%d Y: %d\n", SP, PC, AC, X, Y);
}
