#pragma once
#include <string>
#include <map>
#include <vector>

class ConfigManager {
private:
    std::map<std::string, std::string> config;
    Display display;
    std::vector<std::pair<std::string, double>> algorithmAverageTimes;
    
    void readConfig(const std::string& filename) {
        std::ifstream file(filename);
        std::string line;
        while (std::getline(file, line)) {
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 1);
                config[key] = value;
            }
        }
    }

    long long runAlgorithm(const std::string& algoName, Matrix* matrix, bool output) {
        if (algoName == "SimulatedAnnealing") {
            double maxTime = std::stod(config["max_execution_time"]);
            double tempFactor = std::stod(config["temperature_change_factor"]);
            CoolingMethod coolingMethod = config["cooling_method"] == "Exponential" ? Exponential : config["cooling_method"] == "Linear" ? Linear : config["cooling_method"] == "Logarithmic" ? Logarithmic : Empty ;
            SimulatedAnnealing sann(matrix, coolingMethod, tempFactor, maxTime);
            return sann.solve(output);
        }
        return -1;
    }

    void runAllAlgorithms(Matrix* matrix, const std::vector<std::string>& algorithms, 
                          std::vector<std::ofstream>& outFiles, int currentIteration, int totalIterations, bool output) {
        for (size_t i = 0; i < algorithms.size(); ++i) {
            long long time = runAlgorithm(algorithms[i], matrix, output);
            outFiles[i] << time << "," << std::endl;
            
            algorithmAverageTimes[i].second = (algorithmAverageTimes[i].second * currentIteration + time) / (currentIteration + 1);
        }
        if (!output)
            display.updateScreen((float)(currentIteration + 1) / totalIterations, 
                                 matrix->getTowns(), 
                                 algorithmAverageTimes);
    }

public:
    ConfigManager(const std::string& configFile) : display(50) {
        readConfig(configFile);
    }

    void executeAlgorithms() {
        bool symmetrical = config["symmetrical_matrix"] == "true";
        Matrix* matrix = nullptr;
        int iterations;
        std::vector<std::string> algorithms;
        std::vector<std::ofstream> outFiles;
        bool output;

        if (config["run_sann"] == "true") {
            algorithms.push_back("SimulatedAnnealing");
            outFiles.emplace_back(config["sann_output"]);
        }
        output = config["show_results_disable_display"] == "true";

        if (config["input_type"] == "file") {
            matrix = new Matrix();
            matrix->fillFromFile(config["input_file"]);
            iterations = 1;
            
            algorithmAverageTimes.clear();
            for (const auto& algo : algorithms) {
                algorithmAverageTimes.emplace_back(algo, 0.0);
            }
            
            runAllAlgorithms(matrix, algorithms, outFiles, 0, iterations, output);
            delete matrix;
        } 
        else if (config["input_type"] == "random") {
            std::string townsStr = config["random_towns"];
            std::vector<int> townNumbers;
            
            size_t pos = 0;
            while ((pos = townsStr.find(',')) != std::string::npos) {
                townNumbers.push_back(std::stoi(townsStr.substr(0, pos)));
                townsStr.erase(0, pos + 1);
            }
            if (!townsStr.empty()) {
                townNumbers.push_back(std::stoi(townsStr));
            }

            iterations = std::stoi(config["iterations"]);
            
            for (int towns : townNumbers) {
                algorithmAverageTimes.clear();
                for (const auto& algo : algorithms) {
                    algorithmAverageTimes.emplace_back(algo, 0.0);
                }

                matrix = new Matrix(symmetrical);
                matrix->createEmpty(towns);
                matrix->fillRandom();

                for (int i = 0; i < iterations; ++i) {
                    runAllAlgorithms(matrix, algorithms, outFiles, i, iterations, output);
                    
                    if (i < iterations - 1) {
                        matrix->fillRandom();
                    }
                }
                delete matrix;
            }
        }
        else if (config["input_type"] == "tsplib") {
            std::ifstream file(config["input_file"]);
            std::string line;
            int n = 0;

            // Read header and get dimension
            while (getline(file, line)) {
                if (line.substr(0, 9) == "DIMENSION") {
                    size_t colonPos = line.find(':');
                    if (colonPos != std::string::npos) {
                        n = std::stoi(line.substr(colonPos + 1));
                    }
                }
                if (line == "EDGE_WEIGHT_SECTION") break;
            }

            // Read nxn matrix
            std::vector<std::vector<int>> distances(n, std::vector<int>(n));
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    file >> distances[i][j];
                }
            }

            matrix = new Matrix();

            matrix->fillFromVector(distances, n);
            iterations = 1;

            algorithmAverageTimes.clear();
            for (const auto& algo : algorithms) {
                algorithmAverageTimes.emplace_back(algo, 0.0);
            }

            runAllAlgorithms(matrix, algorithms, outFiles, 0, iterations, output);
            delete matrix;
        }

        for (auto& file : outFiles) {
            file.close();
        }
    }
};
