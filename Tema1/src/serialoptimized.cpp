#include "common.h"
#include <climits>
using namespace std;

int pret_minim, pret_maxim, iteratii;

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

void printDebug(const int n, Cell **& mat)
{
	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < n; ++j)
			cout << "(" << mat[i][j].resursa << ","
				<< mat[i][j].cost_minim_resursa << ","
				<< mat[i][j].cost_compl << ","
				<< mat[i][j].pret_resursa << ","
				<< mat[i][j].buget << ") ";
		cout << "\n";
	}
}

/**
 * Copy from a to b
 */
void copyMatrix(const int n, Cell **a, Cell **b)
{
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			b[i][j] = a[i][j];
}


void computeMin(const int n, Cell **& stats, Cell **& aux)
{
	copyMatrix(n, stats, aux);
	cout << "Before\n";
	printDebug(n, stats);
	for (int i = 0; i < n; ++i)
	{

		int min[2] = {INT_MAX - 2 * n, INT_MAX - 2 * n};
		stats[i][0].cost_minim_resursa = INT_MAX - 2 * n;
		stats[i][0].cost_compl = INT_MAX - 2 * n;

		//minime pe linii la stanga
		for (int j = 1; j < n; ++j)
		{
			bool res = stats[i][j - 1].resursa;
			int pret = stats[i][j - 1].pret_resursa;
			if (min[res] > pret)
				min[res] = pret + 1;
			else
			{
				++min[res];
			}
			++min[!res];
			if (res == false)
			{
				stats[i][j].cost_minim_resursa = min[res];
				stats[i][j].cost_compl = min[!res];
			}
			else
			{
				stats[i][j].cost_minim_resursa = min[!res];
				stats[i][j].cost_compl = min[res];
			}
		}
	}
	cout << "After\n";
	printDebug(n, stats);
}

int main(int argc, char *argv[])
{
	int n;
	Cell **stats;					// Matricea pentru anul curent
	Cell **aux;						// Aux matrix

    checkArgs(argc, argv, iteratii);

    ifstream file_in(argv[2], ios::in);
    readInputSize(file_in, n, pret_minim, pret_maxim);

    //cerr << "Nr iteratii: " << iteratii << "\n";
    //cerr << "n: " << n << " pmin: " << pret_minim << " pmax: " << pret_maxim << endl;

    createMatrix(stats, n);
    createMatrix(aux, n);

    readInput(file_in, n, stats);
    file_in.close();

    ofstream file_out(argv[3], ios::out);

    /*cout << "Costurile initiale \n";
    printCosts(n, stats);
    cout << endl;*/
    //copyMatrix(n, stats, aux);
    /*cout << "Auxiliar \n";
    printCosts(n, aux);*/

    //computeAllYears(n, stats, file_out);
    computeMin(n, stats, aux);


    file_out.close();
    return 0;
}
