#pragma once
#include <string>
#include <map>
#include <vector>
#include <tuple>

class ConfigManager {
private:
    std::map<std::string, std::string> defaultConfig;
    std::vector<std::map<std::string, std::string>> testCases;
    
public:
    ConfigManager(const std::string& configFile) {
        readConfig(configFile);
    }

    void readConfig(const std::string& filename) {
        std::ifstream file(filename);
        std::string line;
        std::map<std::string, std::string>* currentConfig = &defaultConfig;
		std::getline(file, line); // Pomijamy pierwszą linię z instrukcjami
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 1);
                
				// Jeśli znajdziemy plik weściowy, tworzymy nową konfigurację
                if (key == "input_file" && !testCases.empty() && testCases.back().count("input_file")) {
                    testCases.push_back(defaultConfig);
                    currentConfig = &testCases.back();
                } else if (key == "input_file" && testCases.empty()) {
                    testCases.push_back(defaultConfig);
                    currentConfig = &testCases.back();
                }
                
                (*currentConfig)[key] = value;
            }
        }
    }

    void executeAlgorithms() {
        for (const auto& testConfig : testCases) {
            std::vector<std::string> algorithms;
            std::vector<std::ofstream> outFiles;
            
            if (testConfig.at("run_sann") == "true") {
                algorithms.push_back("SimulatedAnnealing");
                outFiles.emplace_back(testConfig.at("sann_output"), std::ios::app);
            }

            int numberOfRuns = 0;
            if (testConfig.count("number_of_runs")) {
                numberOfRuns = std::stoi(testConfig.at("number_of_runs"));
            }

			// Wykonujemy algorytm zadaną liczbę razy
            for (int run = 0; run < numberOfRuns; run++) {
                Matrix* matrix = nullptr;
                
                std::ifstream file(testConfig.at("input_file"));
                std::string line;
                int n = 0;

                while (getline(file, line)) {
                    if (line.substr(0, 9) == "DIMENSION") {
                        size_t colonPos = line.find(':');
                        if (colonPos != std::string::npos) {
                            n = std::stoi(line.substr(colonPos + 1));
                        }
                    }
                    if (line == "EDGE_WEIGHT_SECTION") break;
                }

                std::vector<std::vector<int>> distances(n, std::vector<int>(n));
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < n; j++) {
                        file >> distances[i][j];
                    }
                }

                matrix = new Matrix();
                matrix->fillFromVector(distances, n);

                // Odczyt aktualnej konfiguracji
                double maxTime = std::stod(testConfig.at("max_execution_time"));
                double tempFactor = std::stod(testConfig.at("temperature_change_factor"));
                CoolingMethod coolingMethod = testConfig.at("cooling_method") == "Exponential" ? 
                    Exponential : testConfig.at("cooling_method") == "Linear" ? 
                    Linear : testConfig.at("cooling_method") == "Logarithmic" ? 
                    Logarithmic : EmptyCooling;

				// Uruchomienie algorytmu z odpowiednimi parametrami
                SimulatedAnnealing sann(matrix, coolingMethod, tempFactor, maxTime);
                auto result = 
                    sann.solve(testConfig.at("show_results_disable_display") == "true");

                const auto& improvements = std::get<0>(result);
                long long total_time = std::get<1>(result);
                double final_temp = std::get<2>(result);
                int iterations = std::get<3>(result);

                // Zapis wszystkich poprawek do pliku - wykres schodkowy
                for (const auto& improvement : improvements) {
                    outFiles[0] << improvement.timeFound << ",";
                    outFiles[0] << improvement.cost << ",";
                    for (int i = 0; i < improvement.path.size(); i++) {
                        outFiles[0] << improvement.path[i] << " ";
                    }
                    outFiles[0] << "0," << std::endl;
                }
                auto last_found = improvements.back();
                
                // Oddzielenie wyników kolejnych uruchomień
                outFiles[0] << "===," << last_found.cost << "," << last_found.timeFound << ",";
                for (int i = 0; i < last_found.path.size(); i++) {
                    outFiles[0] << last_found.path[i] << " ";
                }
                outFiles[0] << "0, " << std::endl << std::endl;
                std::cout << last_found.cost << std::endl;


                // Zapisz do osobnego pliku - tylko najlepsze rozwiązania
                std::string bestFileName = testConfig.at("sann_output") + "Best";
                std::ofstream bestFile(bestFileName, std::ios::app);
                bestFile << last_found.cost << "," << last_found.timeFound << ",";
                for (int i = 0; i < last_found.path.size(); i++) {
                    bestFile << last_found.path[i] << " ";
                }
                bestFile << "0, " << std::endl;
                bestFile.close();

                delete matrix;
            }

            for (auto& file : outFiles) {
                file.close();
            }
        }
    }
};
