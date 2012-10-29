#ifndef COMMON_H_
#define COMMON_H_

using namespace std;

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>

// Checks for arguments format
bool checkArgs(int argc, char *argv[], int &iteratii)
{
	if (argc != 4)
	{
		cerr << "Error: Usage\n";
		cerr << argv[0] << " T file_in file_out\n";
		exit(1);
	}
	iteratii = atoi(argv[1]);

	ifstream file_in(argv[2], ios::in);
	ofstream file_out(argv[3], ios::out);

	if(!file_in.is_open())
	{
		cerr << "Error: " << argv[0] << "\nInput file \"" << argv[2] << "\" does not exist\n";
		exit(1);
	}
	file_in.close();
	file_out.close();
   return true;
}

// Informatiile pentru un colonist Cij
struct Cell
{
	bool resursa;			// resursa produsa in anul curent
	int pret_resursa;		// pretul resursei produse in anul curent
	int cost_compl;			// costul cu care cumpara resursa complementara
	int cost_minim_resursa;	// costul minim al resursei _resursa_
	int buget;				// bugetul pe anul curent
};

// Citeste dimensiunile
void readInputSize(ifstream &file_in, int &n, int &pmin, int &pmax)
{
	//ifstream file_in(filename, ios::in);
	if(file_in.good())
	{
		file_in >> pmin >> pmax >> n;
	}
}

// Aloca memorie pentru n x n colonisti
void createMatrix(Cell **& stats, const int n)
{
    stats = new Cell*[n];
    for(int i = 0;i < n;++i)
        stats[i] = new Cell[n];
}

// Reads resources, prices and buget for colonists
void readInput(ifstream &file_in, const int n, Cell **& stats)
{
	if (file_in.good())
	{
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
				file_in >> stats[i][j].resursa;

		for (int i = 0; i < n; ++i)
					for (int j = 0; j < n; ++j)
						file_in >> stats[i][j].pret_resursa;

		for (int i = 0; i < n; ++i)
					for (int j = 0; j < n; ++j)
						file_in >> stats[i][j].buget;
	}
}

// Computes Manhattan distance between two points (i1, j1), (i2, j2)
int manhattan(const int i1, const int j1, const int i2, const int j2)
{
	return abs(i2 - i1) + abs(j2 - j1);
}

// Returns max of two ints
int max(const int a, const int b)
{
	return a < b ? b : a;
}


#endif
