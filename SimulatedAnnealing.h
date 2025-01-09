#pragma once

#include <random>
#include <cmath>
#include <tuple>
#include "enums.h"


class SimulatedAnnealing 
{
private:
    Matrix* matrix;
    int towns;
    int* bestPath;
    int bestCost;
    std::mt19937 rng;
    CoolingMethod coolingMethod;

    double maxExecutionTime;        
    double startingTemperature;     
    double temperatureChangeFactor; 
    long long timeBestFound;        

    NeighborGeneration neighborMethod;
    InitialPathGeneration initialPathMethod;

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
        if (initialPathMethod == Random) {
            for (int i = 0; i < towns; i++) {
                path[i] = i;
            }
            std::shuffle(path + 1, path + towns, rng);
        }
        else if (initialPathMethod == NearestNeighbour) {
            std::vector<bool> visited(towns, false);
            path[0] = 0;  // Start od miasta 0
            visited[0] = true;

            for (int i = 1; i < towns; i++) {
                int current = path[i - 1];
                int nearest = -1;
                int minDist = INT_MAX;

                for (int j = 0; j < towns; j++) {
                    if (!visited[j]) {
                        int dist = matrix[0].get(current, j);
                        if (dist < minDist) {
                            minDist = dist;
                            nearest = j;
                        }
                    }
                }
                path[i] = nearest;
                visited[nearest] = true;
            }
        }
        else {
            std::cout << "Niepoprawna metoda wyboru początkowej trasy\n";
        }
    }

    // Generuje sąsiednie rozwiązanie poprzez zamianę dwóch miast
    void generateNeighbor(int* currentPath, int* newPath) {
        std::copy(currentPath, currentPath + towns, newPath);
        
        if (neighborMethod == Swap) {
            std::uniform_int_distribution<> dist(1, towns - 1);
            int i = dist(rng);
            int j = dist(rng);
            std::swap(newPath[i], newPath[j]);
        }
        else if (neighborMethod == Insert) {
            std::uniform_int_distribution<> dist(1, towns - 1);
            int from = dist(rng);
            int to = dist(rng);

            if (from < to) {
                int temp = newPath[from];
                for (int i = from; i < to; i++) {
                    newPath[i] = newPath[i + 1];
                }
                newPath[to] = temp;
            }
            else {
                int temp = newPath[from];
                for (int i = from; i > to; i--) {
                    newPath[i] = newPath[i - 1];
                }
                newPath[to] = temp;
            }
        }
        else {
            std::cout << "Niepoprawna metoda wyboru sąsiada\n";
        }
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
        double tempFactor, double maxTime,
        NeighborGeneration neighborGen,
        InitialPathGeneration pathGen)
        : rng(std::random_device{}()), 
          coolingMethod(method),
          temperatureChangeFactor(tempFactor), 
          maxExecutionTime(maxTime),
          neighborMethod(neighborGen),
          initialPathMethod(pathGen) {
        this->matrix = matrix;
        this->towns = matrix[0].getTowns();
        this->bestPath = new int[towns];
        this->startingTemperature = generateStartingTemperature();
        
        // Log do kolsoli przed rozpoczęciem algorytmu i pomiaru czasu
        std::cout << "Temperatura poczatkowa: " << startingTemperature << std::endl;
        std::cout << "Funkcja schladzania:    " << 
            (coolingMethod == Exponential ? "wykladnicza" : 
             coolingMethod == Logarithmic ? "logarytmiczna" : 
             coolingMethod == Linear ? "liniowa" : "brak") << "\t\t";
        std::cout << "Stala alfa: " << temperatureChangeFactor << std::endl;
        std::cout << "Sposob generacji sasiada: " << 
            (neighborMethod == Swap ? "swap" : 
             neighborMethod == Insert ? "insert" : "brak") << "\t\t";
        std::cout << "Sposob wyboru sciezki poczatkowej: " << 
            (initialPathMethod == Random ? "losowa" : 
             initialPathMethod == NearestNeighbour ? "najblizszych sasiadow" : "brak") << std::endl;
    }

    struct SolutionRecord {
        int cost;
        std::vector<int> path;
        long long timeFound;
    };

    auto solve(bool output = false) {
        auto start_time = std::chrono::high_resolution_clock::now();
		std::vector<SolutionRecord> improvements;  // Zapisujemy każdą poprawę rozwiązania

        int* currentPath = new int[towns];
        int* newPath = new int[towns];
        
        // Generuj rozwiązanie początkowe
        generateInitialSolution(currentPath);
        int currentCost = calculatePathCost(currentPath);
        
        bestCost = currentCost;
        std::copy(currentPath, currentPath + towns, bestPath);
        
        // Główna pętla algorytmu
        improvements.push_back({
            bestCost,
            std::vector<int>(bestPath, bestPath + towns),
            0 
        });

        double temperature = startingTemperature;
		int iterationsPerTemperature = 1000;    // Zgodnie z zaleceniami od prowadzącego
        int iterationCount = 0;

        while (temperature > 0.05) {
            auto current_time = std::chrono::high_resolution_clock::now();
            auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(
                current_time - start_time).count();

            if (elapsed_seconds >= maxExecutionTime) break;

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
                        
                        // Zapisz w tabeli
                        auto improvement_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::high_resolution_clock::now() - start_time).count();
                        
                        improvements.push_back({
                            bestCost,
                            std::vector<int>(bestPath, bestPath + towns),
                            improvement_time
                        });
                    }
                }
                iterationCount++;
            }
            
            // Wybór metody chłodzenia
            switch (coolingMethod) {
                case Exponential:
                    temperature *= temperatureChangeFactor;
                    break;
                case Linear:
                    temperature -= temperatureChangeFactor;
                    break;
                case Logarithmic:
                    temperature = startingTemperature / (1 + log(temperatureChangeFactor*iterationCount + 1));
                    break;
                default:
					std::cout << "Niepoprawna metoda chłodzenia\n";
            }
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        delete[] currentPath;
        delete[] newPath;

        return std::make_tuple(improvements, total_time, temperature, iterationCount);
    }

    ~SimulatedAnnealing() {
        delete[] bestPath;
    }
};