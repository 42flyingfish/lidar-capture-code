#include <chrono> // for seconds
#include <iostream>
#include <thread> // for sleep_for
#include "decoder.h"

void delay(int amount);

int main(int argc, char **argv) {

	//delay(30); // Artificial delay set by seconds. Adjust as needed.

	if (getData() != 0) {
		std::cout << "An error has occurred.\n";
	}
	return 0;
}

// An artificial delay requested for start up.
// Adjust as needed
void delay(int amount) {
	std::cout << "\nWaiting for " << amount << " seconds\n";
	std::this_thread::sleep_for(std::chrono::seconds(amount));
	std::cout << "Resuming\n";
}
