#include "common.h"
#include <climits>
#include <omp.h>
using namespace std;

int pret_minim, pret_maxim, iteratii;

// Gets the minimum prices for the colonist at (celli, cellj)
void calcCostMinim(const bool resursa, Cell **& stats, const int n, const int celli, const int cellj,
		int &min_compl, int &min_resursa)
{
	int min[2] = {INT_MAX, INT_MAX};
	// Pentru colonistul de la (celli, cellj) se parcurge matricea si se calculeaza minimele sale
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
		{
			bool res = stats[i][j].resursa;
			// Distanta Manhattan + pretul resursei
			int calculus = manhattan(celli, cellj, i, j) + stats[i][j].pret_resursa;
			if (calculus < min[res])
				min[res] = calculus;
		}
	min_compl = min[!resursa];
	min_resursa = min[resursa];
}

// Computes and adds minimums to matrix
void addMinCostToMatrix(const int n, Cell **& stats)
{
	// Calculeaza minimele pentru fiecare colonist si le adaug la matrice
	// Sparg aceasta operatie pe mai multe thread-uri, datele cu care se opereaza fiind independente
	// intre thread-uri.
	// Implicit ce este declarat in interiorul directivei #pragma omp parallel
	// este definit private
	#pragma omp parallel for
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
		{
			calcCostMinim(stats[i][j].resursa, stats, n, i, j, stats[i][j].cost_compl,
					stats[i][j].cost_minim_resursa);
		}
}

// Functie de debugging
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
	#ifdef DEBUG
	printCostMin(n, stats);
	#endif
	// Updatez informatiile referitoare la buget, tip resursa si pret
	// Paralelizez operatiile ce se fac pentru fiecare colonist in parte, fiind independente intre ele
	// De asemeni, variabilele declarate in interior sunt implicit private, fiecare thread
	// utilizand resurse independente de celelalte threaduri
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

			// Corectez pretul in caz ca se depasesc limitele legale
			if (stats[i][j].pret_resursa < pret_minim)
				stats[i][j].pret_resursa = pret_minim;
			else if (stats[i][j].pret_resursa > pret_maxim)
			{
				// Respecializare pe resursa complementara
				stats[i][j].resursa = !stats[i][j].resursa;
				stats[i][j].buget = pret_maxim;
				stats[i][j].pret_resursa = (pret_minim + pret_maxim) / 2;
			}
		}
}

// Calculeaza si apoi scrie in fisier informatiile agregate despre colonisti
// la sfarsitul unui an
void writeOutput(ofstream &file_out, const int n, Cell **& stats)
{
	int countResursaA = 0, countResursaB = 0;
	int pretMaxA = 0, pretMaxB = 0;

	// Paralelizez aceasta operatie, si am nevoie de reduction pentru cele 4 valori agregate,
	// dar fiindca pentru min/max nu exista reduction in implementarea OpenMP cu C++, am
	// rescris reduction-ul folosindu-ma de #pragma omp flush, care actualizeaza valoarea unui
	// intreg pentru toate thread-urile, si apoi se intra in zona critica doar daca acea conditie
	// de maxim este indeplinita
	#pragma omp parallel for reduction(+: countResursaA, countResursaB)
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
		{
			if (!stats[i][j].resursa)
			{
				++countResursaA;
				// Reduction pentru maximul resursei de tip A
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
				// Reduction pentru maximul resursei de tip B
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

// Functie pentru debugging
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

// Write final costs to file. There is nothing to paralelize here, because writing to files is serial
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
	// This is the main loop, for each year
	for (int k = 0; k < iteratii; ++k)
	{
		#ifdef DEBUG
		cout << "Pasul " << k << "\n";
		#endif

		// Compute values for next year and write output to file
		computeNextYear(n, stats);
		writeOutput(file_out, n, stats);

		#ifdef DEBUG
		cout << "\n";
		printCosts(n, stats);
		#endif
	}

	// Write final data to file
	writeCosts(n, stats, file_out);
}

int main(int argc, char *argv[])
{
	int n;
	Cell **stats;					// Matricea pentru anul curent

    checkArgs(argc, argv, iteratii);

    ifstream file_in(argv[2], ios::in);
    readInputSize(file_in, n, pret_minim, pret_maxim);

	#ifdef DEBUG
    cerr << "Nr iteratii: " << iteratii << "\n";
    cerr << "n: " << n << " pmin: " << pret_minim << " pmax: " << pret_maxim << endl;
	#endif

    createMatrix(stats, n);

    readInput(file_in, n, stats);
    file_in.close();

    ofstream file_out(argv[3], ios::out);

	#ifdef DEBUG
	cout << "Costurile initiale \n";
	printCosts(n, stats);
	cout << endl;
	#endif

	// This contains the main loop
    computeAllYears(n, stats, file_out);

    file_out.close();
    return 0;
}
