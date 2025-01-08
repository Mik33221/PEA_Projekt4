#pragma once
#include <fstream>
#include <vector>

class Matrix
{
private:
	int towns;
	int** matrix;
	bool symmetric;

	void set(int i, int j, int value) {
		if (symmetric) {
			if (i < j) {
				return;
			}
		}
		matrix[i][j] = value;
	}

public:
	Matrix(bool symmetric = false) {
		towns = 0;
		matrix = nullptr;
		this->symmetric = symmetric;
	}

	void createEmpty(int towns) {
		this->towns = towns;
		matrix = new int* [towns];
		if (symmetric) {
			for (int i = 0; i < towns; i++) {
				matrix[i] = new int[i + 1];
			}
		}
		else {
			for (int i = 0; i < towns; i++) {
				matrix[i] = new int[towns];
			}
		}
	}

	void fillRandom() {
		for (int i = 0; i < towns; i++) {
			for (int j = 0; j < towns; j++) {
				if (i == j) {
					set(i, j, -1);
				}
				else {
					set(i, j, rand());
				}
			}
		}
	}

	void fillFromFile(std::string filename) {
		symmetric = false;
		std::string expected;
		std::ifstream file(filename);
		if (!file.is_open()) {
			std::cout << "Error: File not found" << std::endl;
			return;
		}
		file >> towns;
		std::cout << "Towns: " << towns << std::endl;

		createEmpty(towns);
		for (int i = 0; i < towns; i++) {
			for (int j = 0; j < towns; j++) {
				file >> matrix[i][j];
			}
		}
		print();

		file >> expected;
		std::cout << "Expected solution: " << expected << std::endl;
	}

	void fillFromVector(std::vector<std::vector<int>> townsVector, int towns) {
		symmetric = false;
		
		std::cout << "Towns: " << towns << std::endl;

		createEmpty(towns);
		for (int i = 0; i < towns; i++) {
			for (int j = 0; j < towns; j++) {
				matrix[i][j] = townsVector[i][j];
			}
		}
		//if (towns < 100)
		//	print();

	}

	int get(int i, int j) {
		if (symmetric) {
			if (i < j) {
				return matrix[j][i];
			}
		}
		return matrix[i][j];
	}

	int getTowns() {
		return towns;
	}

	void print() {
		for (int i = 0; i < towns; i++) {
			if (symmetric) {
				for (int j = 0; j < i + 1; j++) {
					if (matrix[i][j] < 10 && matrix[i][j] > 0) {
						std::cout << " ";
					}
					std::cout << matrix[i][j] << " ";
				}
				std::cout << std::endl;
			}
			else {
				for (int j = 0; j < towns; j++) {
					if (matrix[i][j] < 10 && matrix[i][j] > 0) {
						std::cout << " ";
					}
					std::cout << matrix[i][j] << " ";
				}
				std::cout << std::endl;
			}
		}
	}

	~Matrix() {
		for (int i = 0; i < towns; i++) {
			delete[] matrix[i];
		}
		delete[] matrix;
	}

};
