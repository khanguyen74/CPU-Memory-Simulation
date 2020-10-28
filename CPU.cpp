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
	run();
	sendEndCommand();
}


int CPU::getPC(){
	return PC;
}


/**
 * CPU loop
 */
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

/**
 *This method halts CPU, waiting for first signal from Memory, which is supposed to be 'r' stands for ready
 *
 */

void CPU::waitForMemory(){
	char signal;

	if (read(readPipe, &signal, sizeof(char)) <=0 ){
		throw runtime_error("Memory cannot read from pipe");
	}
	// check if signal sent from memory is ready signal
	if (signal != 'r'){
		throw "Cannot receive ready signal from memory";
	}


}

int CPU::fetchInstruction(){
	// read instruction at location stored in PC from memory then increment PC
	return readMemory(PC++);


}
/*
 * send read command to Memory, send address and then reads the value
 *
 * */
int CPU::readMemory(const int address){
	checkMemoryViolation(address);
	char command = 'R';
	write(writePipe, &command, sizeof(char));
	write(writePipe, &address, sizeof(int));
	int value;
	read(readPipe, &value, sizeof(int));
	return value;
}

/*
 * Check if user program tries to read system memory stack
 * */

void CPU::checkMemoryViolation(const int address){

	if (!isKernelMode && address >= 1000){
		stringstream ss;
		ss << address;
		sendEndCommand();
		string message = "Memory violation: accessing system address " + ss.str() + " in user mode";
		throw message;

	}
}
/*
 * Write value to memory at address
 *
 * */
void CPU::writeMemory(const int address, const int value){
	checkMemoryViolation(address);
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
			AC = 5;
			break;
		case 9: // put port, if port = 1, write ac as int, port = 2, write ac as char
			port = fetchInstruction();
			if ( port == 1){
				printf("%d", AC);
			}else if(port == 2){
				printf("%c", AC);
			}else throw "Invalid port is given for put operation";
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
			isKernelMode = true;
			saveAllRegisters();
			PC = SYSTEM_CALL_ADDRESS;	// set PC to start of system call routine address
			break;

		case 30: // return from system call
			restoreAllRegisters();
			counter = timer; // reset counter
			isKernelMode = false;
			break;
		case 50: 
	
			sendEndCommand();
			break;
		default:
			printf("Unknown command %d\n", IR);
			sendEndCommand();
			break;
	}
}

/*
 * Send ending signal to Memory
 * */
void CPU::sendEndCommand(){
	char command = 'E';
	write(writePipe, &command, sizeof(char));

}


/*
 * If timer reaches 0, change to kernel mode and perform the service routine
 * */
void  CPU::checkTimerInterrupt(){
	if (counter==0 && !isKernelMode){
		isKernelMode = true;
		saveAllRegisters();
		PC = TIMER_INTERRUPT_ADDRESS;
	}

}

/*
 *Save PC and SP
 * */
void CPU::saveAllRegisters(){
	int temp = SP;	// temporarily holds SP
	SP = SYSTEM_STACK;
	writeMemory(--SP, temp); // save SP first
	writeMemory(--SP, PC);
}

/**
 * This method restores all system registers from system stack
 *
 *
 */
void CPU:: restoreAllRegisters(){
	PC = readMemory(SP++);
	SP = readMemory(SP++);
}
