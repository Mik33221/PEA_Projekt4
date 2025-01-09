#pragma once

#include <random>
#include <cmath>

enum CoolingMethod {
    Exponential,
    Linear,
    Logarithmic,
    Empty,
};

class SimulatedAnnealing 
{
private:
    Matrix* matrix;
    int towns;
    int* bestPath;
    int bestCost;
    std::mt19937 rng;
    CoolingMethod coolingMethod;

    double maxExecutionTime;        // New stop criterion
    double startingTemperature;     // Starting temperature
    double temperatureChangeFactor; // Factor for temperature change
    long long timeBestFound;        // Time when the best solution was found

    // Oblicza koszt ścieżki
    int calculatePathCost(int* path) {
        int cost = 0;
        for (int i = 0; i < towns - 1; i++) {
            cost += matrix[0].get(path[i], path[i + 1]);
        }
        cost += matrix[0].get(path[towns - 1], path[0]); // Powrót do początku
        return cost;
    }

    // Generuje losowe rozwiązanie początkowe
    void generateInitialSolution(int* path) {
        for (int i = 0; i < towns; i++) {
            path[i] = i;
        }
        std::shuffle(path + 1, path + towns, rng); // Zachowuj miasto 0 jako pierwsze
    }

    // Generuje sąsiednie rozwiązanie poprzez zamianę dwóch miast
    void generateNeighbor(int* currentPath, int* newPath) {
        std::uniform_int_distribution<> dist(1, towns - 1);
        int i = dist(rng);
        int j = dist(rng);
        
        std::copy(currentPath, currentPath + towns, newPath);
        std::swap(newPath[i], newPath[j]);
		// Można zamienić na insert, lepsze wyniki, insertuje jedno miasto w losowe miejsce
    }

	// Generuje temperaturę początkową na podstawie danych w pliku
    double generateStartingTemperature() {
        int iterations = 1000;
        int* currentPath = new int[towns];
        int* newPath = new int[towns];
        generateInitialSolution(currentPath);
        int currentCost = calculatePathCost(currentPath);
        int sum = 0;
        for (int i = 0; i < iterations; i++) {
            generateNeighbor(currentPath, newPath);
            int newCost = calculatePathCost(newPath);
            sum += std::abs(newCost - currentCost);
            std::copy(newPath, newPath + towns, currentPath);
            currentCost = newCost;
        }
        delete[] currentPath;
        delete[] newPath;
        return double(sum) / iterations;
    }

public:
    SimulatedAnnealing(Matrix* matrix, CoolingMethod method,
        double tempFactor, double maxTime)
        : rng(std::random_device{}()), coolingMethod(method),
        temperatureChangeFactor(tempFactor), maxExecutionTime(maxTime) {
        this->matrix = matrix;
        this->towns = matrix[0].getTowns();
        this->bestPath = new int[towns];
		this->startingTemperature = generateStartingTemperature();
		std::cout << "Starting temperature: " << startingTemperature << std::endl;
    }

    auto solve(bool output = false) {
        auto start_time = std::chrono::high_resolution_clock::now();
        timeBestFound = 0;

        // Parametry algorytmu
		double temperature = startingTemperature;
        double linearDecrease = 10.0;
        int iterationsPerTemperature = 100; //do 500 nawet do 1000 dla 403
        int iterationCount = 0;

        int* currentPath = new int[towns];
        int* newPath = new int[towns];
        
        // Generuj rozwiązanie początkowe
		// Można zmienić na nearest neighbour
        generateInitialSolution(currentPath);
        int currentCost = calculatePathCost(currentPath);
        
        bestCost = currentCost;
        std::copy(currentPath, currentPath + towns, bestPath);

        // Główna pętla algorytmu
        while (true) {
            auto current_time = std::chrono::high_resolution_clock::now();
            auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(
                current_time - start_time)
                .count();

            if (elapsed_seconds >= maxExecutionTime) {
                std::cout << "Przerwano po czasie\n";
				std::cout << "Aktualna temperatura: " << temperature;
                break;
            }
            for (int i = 0; i < iterationsPerTemperature; i++) {
                generateNeighbor(currentPath, newPath);
                int newCost = calculatePathCost(newPath);
                
                // Oblicz różnicę kosztów
                int delta = newCost - currentCost;
                
                // Akceptuj nowe rozwiązanie zgodnie z kryterium Metropolisa
                if (delta < 0 || (std::exp(-delta / temperature) > std::uniform_real_distribution<>(0, 1)(rng))) {
                    std::copy(newPath, newPath + towns, currentPath);
                    currentCost = newCost;
                    
                    // Aktualizuj najlepsze znalezione rozwiązanie
                    if (currentCost < bestCost) {
                        bestCost = currentCost;
                        std::copy(currentPath, currentPath + towns, bestPath);
                    }
                }
				iterationCount++;
            }
            
            // Cooling method selection
            switch (coolingMethod) {
                case Exponential:
                    temperature *= temperatureChangeFactor;
                    break;
                case Linear:
                    temperature -= linearDecrease;
                    break;
                case Logarithmic:
                    temperature = startingTemperature / (1 + log(100*iterationCount + 1));
                    break;
                default:
					std::cout << "Niepoprawna metoda chłodzenia\n";
            }

            iterationCount+=100;
        }

        if (output) {
            std::cout << "Best path: ";
            for (int i = 0; i < towns; i++) {
                std::cout << bestPath[i] << " ";
            }
            std::cout << "0" << std::endl;  // Powrót do początku
            std::cout << "Path length: " << bestCost << std::endl;
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        // Write to file
        std::ofstream outFile("output.txt");
        outFile << "Time best found: " << timeBestFound << " seconds\n";
        outFile << "Best path: ";
        for (int i = 0; i < towns; i++) {
            outFile << bestPath[i] << " ";
        }
        outFile << "0\n";
        outFile << "Path length: " << bestCost << "\n";
        outFile.close();

        delete[] currentPath;
        delete[] newPath;
        return time;
    }

    ~SimulatedAnnealing() {
        delete[] bestPath;
    }
};