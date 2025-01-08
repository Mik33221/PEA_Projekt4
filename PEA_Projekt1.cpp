#include <chrono>
#include "Display.h"
#include "Matrix.h"
#include "SimulatedAnnealing.h"
#include "ConfigManager.h"

#include <Windows.h>


int main()
{
	srand(time(NULL));

	ConfigManager config("config.txt");
    config.executeAlgorithms();

	return 0;
}
