#pragma once
enum CoolingMethod {
	Exponential,
	Linear,
	Logarithmic,
	EmptyCooling,
};

enum NeighborGeneration {
	Swap,
	Insert,
	EmptyNeighbor,
};

enum InitialPathGeneration {
	Random,
	NearestNeighbour,
	EmptyPath,
};