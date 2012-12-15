/*
 ============================================================================
 Name        : Tema3.cpp
 Author      : Constantin Șerban-Rădoi
 Version     :
 Copyright   : 2012
 Description : Compute Pi in MPI C++
 ============================================================================
 */
#include <cmath>
#include <cstdlib>
#include <cstring>
#include "mpi.h"
#include "stddef.h"
#include <fstream>
#include <iostream>
#include "Complex.h"
using namespace std;

#define DBG 0
#define NUM_COLORS 256

typedef unsigned int uint;

struct Input
{
	bool type;
	double x_min, x_max, y_min, y_max;
	double step;
	int max_steps;
	double x_julia, y_julia;
};

ostream& operator<<(ostream& os, const Input &in)
{
	os << "Input: { type(" << in.type << "), space[ (" << in.x_min << ","
			<< in.x_max << "), (" << in.y_min << "," << in.y_max << ") ]"
			<< " step(" << in.step << ") max_steps(" << in.max_steps << ")";
	if (in.type)
		os << " julia(" << in.x_julia << "," << in.y_julia << ")";
	os << " }\n";
	return os;
}

// Read input from file
bool readInput(Input &input, ifstream &fin)
{
	if (fin.good())
	{
		fin >> input.type;
		fin >> input.x_min >> input.x_max >> input.y_min >> input.y_max;
		fin >> input.step;
		fin >> input.max_steps;
		if (input.type) // Julia
			fin >> input.x_julia >> input.y_julia;
		return true;
	}
	else
		return false;
}

// Compute a side for image resolution
uint computeSide(double max, double min, double resolution)
{
	return floor((max - min) / resolution);
}

// Print matrix
void printMatrix(unsigned char *m, uint width, uint height)
{
	for (uint i = 0; i < height; ++i)
	{
		for (uint j = 0; j < width; ++j)
			cerr << (uint) m[i * width + j] << " ";
		cerr << "\n";
	}
}

// Set Matrix Line lineNumber with numbers in line array
void mergeMatrix(unsigned char *m, uint width, uint height, unsigned char *line, uint lineNumber)
{
	for (uint i = 0; i < width; ++i)
	{
		m[i + lineNumber * width] = line[i];
	}
}

// Mandelbrot
void generateImage(unsigned char *img, uint width, double step,
		double max_steps, Complex cmin, Complex cmax, bool type, Complex julia)
{
	uint i = 0;
	uint j = 0;
	for (double cre = cmin.re; cre < cmax.re; cre += step, ++i)
	{
		j = 0;
		for (double cim = cmin.im; cim < cmax.im; cim += step, ++j)
		{
			Complex z;
			if (type == 1)	// Julia
				z = Complex(cre, cim);
			uint generationStep = 0;
			while (z.squareModule() < 4 && generationStep < max_steps)
			{
				if (type == 0) // Mandelbrot
					z = z * z + Complex(cre, cim);
				else
					// Julia
					z = z * z + julia;
				++generationStep;
			}
			img[j * width + i] = generationStep % NUM_COLORS;

		}
	}
	if (DBG == 1)
		cerr << "(i, j) :(" << i << "," << j << ")";
}

void writeImage(const char* filename, unsigned char *img, unsigned int width,
		unsigned int height)
{
	ofstream fout(filename);
	if (fout.good())
	{
		fout << "P2\n" << width << " " << height << "\n";
		fout << NUM_COLORS - 1 << "\n";

		for (int i = height - 1; i >= 0; --i)
		{
			for (uint j = 0; j < width; ++j)
				fout << (uint) img[(int) i * width + j] << " ";
			fout << "\n";
		}
	}
	fout.close();
}

int main(int argc, char *argv[])
{
	int rank, size;

	MPI::Init(argc, argv);
	size = MPI::COMM_WORLD.Get_size();
	rank = MPI::COMM_WORLD.Get_rank();

	// Create MPI type for Input struct
	const int nitems = 9;
	int blocklengths[nitems] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
	MPI_Datatype types[nitems] = {MPI::BOOL, MPI::DOUBLE, MPI::DOUBLE, MPI::DOUBLE, MPI::DOUBLE, MPI::DOUBLE, MPI::INT, MPI::DOUBLE, MPI::DOUBLE};
	MPI_Datatype mpi_input_type;
	MPI_Aint     offsets[nitems];

	offsets[0] = offsetof(Input, type);
	offsets[1] = offsetof(Input, x_min);
	offsets[2] = offsetof(Input, x_max);
	offsets[3] = offsetof(Input, y_min);
	offsets[4] = offsetof(Input, y_max);
	offsets[5] = offsetof(Input, step);
	offsets[6] = offsetof(Input, max_steps);
	offsets[7] = offsetof(Input, x_julia);
	offsets[8] = offsetof(Input, y_julia);

	MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_input_type);
	MPI_Type_commit(&mpi_input_type);

	if (rank == 0)
	{
		ifstream fin(argv[1]); // Input file
		//ofstream fout(argv[2]); // Output file

		// Read input
		if (DBG == 1)
			cerr << "Input file name: " << argv[1] << "\n";

		Input input;
		memset(&input, 0, sizeof(input));

		if (!readInput(input, fin))
		{
			cerr << "Input file not found \n";
			exit(1);
		}

		if (DBG == 1)
			cerr << input;

		fin.close();

		uint width = computeSide(input.x_max, input.x_min, input.step);
		uint height = computeSide(input.y_max, input.y_min, input.step);

		if (DBG == 1)
			cerr << "width, height: (" << width << ", " << height << ")\n";

		unsigned char *m = new unsigned char[(height + 1) * (width + 1)];

		//cerr << "Before\n";
		//printMatrix(m, width, height);
//		generateImage(m, width, input.step, input.max_steps,
//				Complex(input.x_min, input.y_min),
//				Complex(input.x_max, input.y_max), input.type,
//				Complex(input.x_julia, input.y_julia));
		//cerr << "After\n";
		//printMatrix(m, width, height);

		// Send input info to other processes
		for (int i = 0; i < size; ++i)
			if (i != rank)
			{
				MPI_Send(&input, 1, mpi_input_type, i, 0, MPI::COMM_WORLD);
				if (DBG == 1)
					cerr << "Rank: " << rank << " sent input to " << i << "\n";
			}

		uint chunkHeight = ceil((double)height / (double)size);
		uint adjustedHeight = chunkHeight;

		for (int i = 0; i < size; ++i)
			if (i != rank)
			{
				MPI_Status status;
				unsigned char *buff = new unsigned char[(chunkHeight + 1) * (width + 1)];
				memset(buff, 0, sizeof(buff));

				MPI_Recv(buff, (chunkHeight + 1) * (width + 1), MPI::UNSIGNED_CHAR, MPI::ANY_SOURCE, 1, MPI::COMM_WORLD, &status);
				if (status.MPI_SOURCE == size - 1)
					adjustedHeight = height - (size - 1) * chunkHeight;

				//printMatrix(buff, width, chunkHeight);

				for (uint lineNr = 0; lineNr < adjustedHeight; ++lineNr)
				{
					//cerr << "rank: " << rank << " lineToMerge: " << status.MPI_SOURCE * chunkHeight + lineNr << "\n";
					mergeMatrix(m, width, height, buff + width * lineNr, status.MPI_SOURCE * chunkHeight + lineNr);
				}
				//printMatrix(m, width, height);

				delete[] buff;
			}

		Complex myMin(input.x_min, input.y_min);
		uint idx = 0;
		double dstep;
		for (dstep = input.y_min; dstep < input.y_max && idx < chunkHeight; dstep += input.step, ++idx)
			;
		Complex myMax(input.x_max, dstep);

		if (DBG == 1)
			cerr << "Rank: " << rank << " myMin: " << myMin << " myMax: " << myMax << "\n";

		unsigned char *imageChunk = new unsigned char[(chunkHeight + 1) * (width + 1)];
		generateImage(imageChunk, width, input.step, input.max_steps, myMin, myMax, input.type, Complex(input.x_julia, input.y_julia));

		//printMatrix(imageChunk, width, chunkHeight);
		//printMatrix(m, width, height);

		for (uint lineNr = 0; lineNr < adjustedHeight; ++lineNr)
			mergeMatrix(m, width, height, imageChunk + width * lineNr, lineNr);


		//printMatrix(m, width, height);

		writeImage(argv[2], m, width, height);

		delete[] m;
	}

	else	// Not rank 0
	{
		MPI_Status status;
		Input in;
		MPI_Recv(&in, 1, mpi_input_type, MPI::ANY_SOURCE, 0, MPI::COMM_WORLD, &status);
		if (DBG == 1)
			cerr << "Rank: " << rank << " received input from " << status.MPI_SOURCE << " " << in;

		uint width = computeSide(in.x_max, in.x_min, in.step);
		uint height = computeSide(in.y_max, in.y_min, in.step);

		uint chunkHeight = ceil((double)height / (double)size);
		uint adjustedHeight = chunkHeight;
		if (rank == size - 1)		// Last process
			adjustedHeight = height - (size - 1) * chunkHeight;

		if (DBG == 1)
			cerr << "Rank: " << rank << " chunkHeight: " <<  adjustedHeight << "\n";

		unsigned char *imageChunk = new unsigned char[(chunkHeight + 1) * (width + 1)];

		uint idx = 0;
		double dstep;
		for (dstep = in.y_min; dstep < in.y_max && idx < (rank * chunkHeight); dstep += in.step, ++idx)
			;
		Complex myMin(in.x_min, dstep);
		for (; dstep < in.y_max && idx < ((rank + 1) * chunkHeight); dstep += in.step, ++idx)
			;
		Complex myMax(in.x_max, dstep);

		if (DBG == 1)
			cerr << "Rank: " << rank << " myMin: " << myMin << " myMax: " << myMax << "\n";


		generateImage(imageChunk, width, in.step, in.max_steps, myMin, myMax, in.type, Complex(in.x_julia, in.y_julia));

		MPI_Send(imageChunk, chunkHeight * width, MPI::UNSIGNED_CHAR, 0, 1, MPI::COMM_WORLD);

		delete[] imageChunk;
	}

	MPI_Type_free(&mpi_input_type);
	MPI::Finalize();
	return 0;
}

