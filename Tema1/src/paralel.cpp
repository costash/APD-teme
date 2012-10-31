#include "common.h"
#include <climits>
#include <omp.h>
using namespace std;

int pret_minim, pret_maxim, iteratii;

// Gets the minimum price for own resource from (celli, cellj) perspective
int getCostMinimResursaProprie(const bool resursa, Cell **& stats, const int n,
		const int celli, const int cellj)
{
	int minim = INT_MAX;
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
		{
			if (stats[i][j].resursa == resursa &&
					manhattan(celli, cellj, i, j) + stats[i][j].pret_resursa < minim)
				minim = stats[i][j].pret_resursa + manhattan(celli, cellj, i, j);
		}
	return minim;
}

// Gets the minimum price for complementary resource from (celli, cellj) perspective
int getCostMinimResursaCompl(const bool resursa, Cell **& stats, const int n,
		const int celli, const int cellj)
{
	int minim = INT_MAX;
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
		{
			if (i == celli && j == cellj)
				continue;
			if (stats[i][j].resursa == resursa &&
					manhattan(celli, cellj, i, j) + stats[i][j].pret_resursa < minim)
				minim = stats[i][j].pret_resursa + manhattan(celli, cellj, i, j);
		}
	return minim;
}

// Computes and adds minimums to matrix
void addMinCostToMatrix(const int n, Cell **& stats)
{
	// Paralelizez calculul minimelor, un thread calculeaza minimele unui singur colonist
	#pragma omp parallel for
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
		{
			stats[i][j].cost_compl = getCostMinimResursaCompl(!stats[i][j].resursa,
					stats, n, i, j);
			stats[i][j].cost_minim_resursa = getCostMinimResursaProprie(stats[i][j].resursa,
					stats, n, i, j);
		}
}

void printCostMin(const int n, Cell **& stats)
{
	cout << "Costuri minime: \n";
	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < n; ++j)
		{
			cout << "(" << stats[i][j].cost_minim_resursa << ","
					<< stats[i][j].cost_compl << ") ";
		}
		cout << endl;
	}
}

// Computes the matrix for next year
//void computeNextYear(const int n, Cell **& stats, Cell **& next_year)
void computeNextYear(const int n, Cell **& stats)
{
	addMinCostToMatrix(n, stats);
	//printCostMin(n, stats);
	// Paralelizez modificarile ce se fac bugetului, etc pentru fiecare colonist in parte
	#pragma omp parallel for
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
		{
			if (stats[i][j].buget < stats[i][j].cost_compl)
			{
				stats[i][j].pret_resursa += stats[i][j].cost_compl - stats[i][j].buget;
			}
			else if (stats[i][j].buget > stats[i][j].cost_compl)
			{
				stats[i][j].pret_resursa += (stats[i][j].cost_compl - stats[i][j].buget) / 2;
			}
			else
			{
				stats[i][j].pret_resursa = stats[i][j].cost_minim_resursa + 1;
			}
			stats[i][j].buget = stats[i][j].cost_compl;

			if (stats[i][j].pret_resursa < pret_minim)
				stats[i][j].pret_resursa = pret_minim;
			else if (stats[i][j].pret_resursa > pret_maxim)
			{
				// respecializare
				stats[i][j].resursa = !stats[i][j].resursa;
				stats[i][j].buget = pret_maxim;
				stats[i][j].pret_resursa = (pret_minim + pret_maxim) / 2;
			}
		}
}

// Writes
void writeOutput(ofstream &file_out, const int n, Cell **& stats)
{
	int countResursaA = 0, countResursaB = 0;
	int pretMaxA = 0, pretMaxB = 0;

	#pragma omp parallel for reduction(+: countResursaA, countResursaB)// reduction(max: pretMaxA, pretMaxB)
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
		{
			if (!stats[i][j].resursa)
			{
				++countResursaA;
				#pragma omp flush(pretMaxA)
				if (stats[i][j].pret_resursa > pretMaxA)
				{
					#pragma omp critical
					{
						if (stats[i][j].pret_resursa > pretMaxA)
							pretMaxA = stats[i][j].pret_resursa;
					}
				}
			}
			else
			{
				++countResursaB;
				#pragma omp flush(pretMaxB)
				if (stats[i][j].pret_resursa > pretMaxB)
				{
					#pragma omp critical
					{
						if (stats[i][j].pret_resursa > pretMaxB)
							pretMaxB = stats[i][j].pret_resursa;
					}
				}
			}
		}
	file_out << countResursaA << " " << pretMaxA << " ";
	file_out << countResursaB << " " << pretMaxB << "\n";
}

void printCosts(const int n, Cell **& stats)
{
	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < n; ++j)
			cout << "(" << stats[i][j].resursa << ","
				<< stats[i][j].pret_resursa << ","
				<< stats[i][j].buget << ") ";
		cout << "\n";
	}
}

// Write final costs.
void writeCosts(const int n, Cell **& stats, ofstream &file_out)
{
	for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
				file_out << "(" << stats[i][j].resursa << ","
					<< stats[i][j].pret_resursa << ","
					<< stats[i][j].buget << ") ";
			file_out << "\n";
		}
}

// Computes requested data for all years
void computeAllYears(const int n, Cell **& stats, ofstream &file_out)
{
	for (int k = 0; k < iteratii; ++k)
	{
		//cout << "Pasul " << k << "\n";
		//computeNextYear(n, stats, next_year);
		computeNextYear(n, stats);

		writeOutput(file_out, n, stats);

		//cout << "\n";
		//printCosts(n, stats);
	}
	writeCosts(n, stats, file_out);
}

int main(int argc, char *argv[])
{
	int n;
	Cell **stats;					// Matricea pentru anul curent

    checkArgs(argc, argv, iteratii);

    ifstream file_in(argv[2], ios::in);
    readInputSize(file_in, n, pret_minim, pret_maxim);

    //cerr << "Nr iteratii: " << iteratii << "\n";
    //cerr << "n: " << n << " pmin: " << pret_minim << " pmax: " << pret_maxim << endl;

    createMatrix(stats, n);

    readInput(file_in, n, stats);
    file_in.close();

    ofstream file_out(argv[3], ios::out);

    //cout << "Costurile initiale \n";
    //printCosts(n, stats);
    //cout << endl;
    computeAllYears(n, stats, file_out);

    file_out.close();
    return 0;
}
