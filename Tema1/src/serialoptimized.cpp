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

void computeMin(const int n, Cell **& stats, Cell **& aux)
{
	for (int i = 0; i < n; ++i)
	{
		//minime pe linii la stanga
		int minimLeftA = INT_MAX;
		int minimLeftB = INT_MAX;
		for (int j = 1; j < n; ++j)
		{
			bool res = stats[i][j].resursa;
			if (res == false)	// resursa A
			{

			}
		}
	}
}

// Copy from a to b
void copyMatrix(const int n, Cell **a, Cell **b)
{
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			b[i][j] = a[i][j];
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
    copyMatrix(n, stats, aux);
    /*cout << "Auxiliar \n";
    printCosts(n, aux);*/

    //computeAllYears(n, stats, file_out);


    file_out.close();
    return 0;
}
