#ifndef CPU_H
#define CPU_H

class CPU{
	private:
		int SP;
		int PC;
		int IR;	
		int AC;
		int X; 
		int Y;
		int counter;
		bool isKernelMode;
		const int timer;
		static const int SYSTEM_STACK = 2000;
		static const int USER_PROGRAM_STACK = 1000;
		static const int TERMINATE_INSTRUCTION = 50;
		static const int TIMER_INTERRUPT_ADDRESS = 1000;
		static const int SYSTEM_CALL_ADDRESS = 1500;
		const int readPipe;
		const int writePipe;
	public:
		CPU(int readPipe, int writePipe, int time);
		int getPC();
		void run();
		void waitForMemory();
		int fetchInstruction();
		int readMemory(const int);
		void executeInstruction();
		void sendEndCommand();
		void writeMemory(const int address, const int val);
		void checkTimerInterrupt();
		void saveAllRegisters();
		void restoreAllRegisters();
		void checkMemoryViolation(const int address);
};

#endif
