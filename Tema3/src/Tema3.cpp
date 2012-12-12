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
#include <fstream>
#include <iostream>
#include "Complex.h"
using namespace std;

#define DBG 1
#define NUM_COLORS 256

typedef unsigned int uint;

struct Input
{
	bool type;
	double x_min, x_max, y_min, y_max;
	double step;
	int max_stepts;
	double x_julia, y_julia;
};

ostream& operator<<(ostream& os, const Input &in)
{
	os << "Input: { type(" << in.type << "), space[ (" << in.x_min << ","
			<< in.x_max << "), (" << in.y_min << "," << in.y_max << ") ]"
			<< " step(" << in.step << ") max_steps(" << in.max_stepts << ")";
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
		fin >> input.max_stepts;
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

// Mandelbrot
void mandelbrot(unsigned char *img, unsigned int width, double step,
		double max_steps, Complex cmin, Complex cmax)
{
	uint i = 0;
	uint j = 0;
	for (double cre = cmin.re; cre < cmax.re; cre += step, ++i)
	{
		j = 0;
		for (double cim = cmin.im; cim < cmax.im; cim += step, ++j)
		{
			Complex z;
			uint step = 0;
			while (z.squareModule() < 4 && step < max_steps)
			{
				z = z * z + Complex(cre, cim);
				++step;
			}
			img[i * width + j] = step % NUM_COLORS;

		}
	}
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

		//for (uint i = 0; i < height; ++i)
		for (int i = height - 1; i >= 0; --i)
		{
			for (uint j = 0; j < width; ++j)
				//for (int j = width - 1; j >= 0; --j)
				fout << (uint) img[(int) j * width + i] << " ";
			fout << "\n";
		}
	}
	fout.close();
}

int main(int argc, char *argv[])
{
	int rank, size;

	ifstream fin(argv[1]); // Input file
	ofstream fout(argv[2]); // Output file

	MPI::Init(argc, argv);
	size = MPI::COMM_WORLD.Get_size();
	rank = MPI::COMM_WORLD.Get_rank();

	if (rank == 0)
	{
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

		cerr << "width, height: (" << width << ", " << height << ")\n";

		unsigned char *m = new unsigned char[(height + 1) * (width + 1)];

		//cerr << "Before\n";
		//printMatrix(m, width, height);
		mandelbrot(m, width, input.step, input.max_stepts,
				Complex(input.x_min, input.y_min),
				Complex(input.x_max, input.y_max));
		//cerr << "After\n";
		//printMatrix(m, width, height);

		writeImage(argv[2], m, width, height);

		delete[] m;
	}

	MPI::Finalize();
	return 0;
}

