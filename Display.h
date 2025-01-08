#pragma once
#include <iostream>
#include <iomanip>
#include <Windows.h>
#include <psapi.h>
#include <vector>
#include <string>

class Display
{
private:
	int barWidth;
	float progress;
	int instanceSize;
	std::vector<std::pair<std::string, double>> algorithmTimes;
	SIZE_T memoryUsage;

	void clearScreen() {
		system("cls");
	}

	void printProgressBar() {
		std::cout << "[";
		int pos = barWidth * progress;
		for (int i = 0; i < barWidth; ++i) {
			if (i < pos) std::cout << '=';
			else if (i == pos) std::cout << ">";
			else std::cout << " ";
		}
		std::cout << "] " << int(progress * 100.0) << " %\n";
	}

	void printInstanceSize() {
		std::cout << "Instance size: " << instanceSize << " cities\n";
	}

	void printAlgorithmTimes() {
		std::cout << "Average algorithm times:\n";
		for (const auto& algo : algorithmTimes) {
			std::cout << "  " << std::setw(20) << std::left << algo.first 
					  << ": " << std::fixed << std::setprecision(2) << algo.second << " ms\n";
		}
	}

	void printMemoryUsage() {
		std::cout << "Memory usage: " << memoryUsage   << " B\n";
	}

	void updateMemoryUsage() {
		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		memoryUsage = pmc.WorkingSetSize;
	}

public:
	Display(int barWidth) : barWidth(barWidth), progress(0), instanceSize(0), memoryUsage(0) {}

	void updateScreen(float newProgress, int newInstanceSize, const std::vector<std::pair<std::string, double>>& newAlgorithmTimes) {
		progress = newProgress;
		instanceSize = newInstanceSize;
		algorithmTimes = newAlgorithmTimes;
		updateMemoryUsage();

		clearScreen();
		printProgressBar();
		std::cout << std::endl;
		printInstanceSize();
		std::cout << std::endl;
		printAlgorithmTimes();
		std::cout << std::endl;
		printMemoryUsage();
	}
};