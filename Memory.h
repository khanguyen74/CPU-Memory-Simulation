#ifndef MEMORY_H
#define MEMORY_H
#include <vector>
#include <string>

using namespace std;
class Memory{
	private:
		int memory[2000];
		const int writePipe;
		const int readPipe;
	public:
		Memory(int, int, vector<string>);
		void readInstruction(const vector<string> instructions);
		void run();
		void executeCommand(const char );
		bool isNotTerminated(const char );
		bool isWriteCommand(const char );
		bool isReadCommand(const char);
		void sendReadySignalToCpu();
};





#endif
