#include "common.h"
#include <climits>
using namespace std;

int pret_minim, pret_maxim, iteratii;

// Functie de debug
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

// Functie de debugging
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

// Writes
void writeOutput(ofstream &file_out, const int n, Cell **& stats)
{
	int countResursaA = 0, countResursaB = 0;
	int pretMaxA = 0, pretMaxB = 0;

	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
		{
			if (!stats[i][j].resursa)
			{
				++countResursaA;
				if (stats[i][j].pret_resursa > pretMaxA)
					pretMaxA = stats[i][j].pret_resursa;
			}
			else
			{
				++countResursaB;
				if (stats[i][j].pret_resursa > pretMaxB)
					pretMaxB = stats[i][j].pret_resursa;
			}
		}
	file_out << countResursaA << " " << pretMaxA << " ";
	file_out << countResursaB << " " << pretMaxB << "\n";
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

inline int getMin2(const int a, const int b)
{
	return a > b ? b : a;
}

inline int getMin3(const int a, const int b, const int c)
{
	int aux = getMin2(a, b);
	return getMin2(aux, c);
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
	/*cout << "Before lines_left->right\n";
	printDebug(n, stats);*/
	// Calculez minimele pe linii
	for (int i = 0; i < n; ++i)
	{

		int min[2] = {INT_MAX - 2 * n, INT_MAX - 2 * n};
		stats[i][0].cost_minim_resursa = INT_MAX - 2 * n;
		stats[i][0].cost_compl = INT_MAX - 2 * n;

		//minime pe linii de la stanga la dreapta
		for (int j = 1; j < n; ++j)
		{
			bool res = stats[i][j - 1].resursa;
			int pret = stats[i][j - 1].pret_resursa;
			if (min[res] > pret)
				min[res] = pret + 1;
			else
				++min[res];

			++min[!res];
			if (res == false)		// resursa de tip A
			{
				stats[i][j].cost_minim_resursa = min[res];
				stats[i][j].cost_compl = min[!res];
			}
			else					// resursa de tip B
			{
				stats[i][j].cost_minim_resursa = min[!res];
				stats[i][j].cost_compl = min[res];
			}
		}

		min[0] = stats[i][n - 1].cost_minim_resursa;
		min[1] = stats[i][n - 1].cost_compl;
		//minime pe linii de la dreapta la stanga
		for (int j = n - 2; j >= 0; --j)
		{
			bool res = stats[i][j + 1].resursa;
			int pret = stats[i][j + 1].pret_resursa;
			if (min[res] > pret)
				min[res] = pret + 1;
			else
				++min[res];

			++min[!res];
			if (res == false)		// resursa de tip A
			{
				if (stats[i][j].cost_minim_resursa > min[res])
					stats[i][j].cost_minim_resursa = min[res];
				else
					min[res] = stats[i][j].cost_minim_resursa;
				if (stats[i][j].cost_compl > min[!res])
					stats[i][j].cost_compl = min[!res];
				else
					min[!res] = stats[i][j].cost_compl;
			}
			else					// resursa de tip B
			{
				if (stats[i][j].cost_compl > min[res])
					stats[i][j].cost_compl = min[res];
				else
					min[res] = stats[i][j].cost_compl;
				if (stats[i][j].cost_minim_resursa > min[!res])
					stats[i][j].cost_minim_resursa = min[!res];
				else
					min[!res] = stats[i][j].cost_minim_resursa;
			}
		}
	}

	// Salvez matricea obtinuta pentru a compara din nou cu ea la final
	copyMatrix(n, stats, aux);

	/*cout << "After lines left->Right + right->left\n";
	printDebug(n, stats);*/

	// Calculez minimele pe coloane, pe matricea obtinuta
	for (int j = 0; j < n; ++j)
	{
		int min[2];
		min[0] = stats[0][j].cost_minim_resursa;
		min[1] = stats[0][j].cost_compl;
		// Calculez minimele de sus in jos
		for (int i = 1; i < n; ++i)
		{
			bool res = stats[i - 1][j].resursa;
			int pret = stats[i - 1][j].pret_resursa;
			if (min[res] > pret)
				min[res] = pret + 1;
			else
				++min[res];
			++min[!res];

			if (res == false)		// resursa de tip A
			{
				if (stats[i][j].cost_minim_resursa > min[res])
					stats[i][j].cost_minim_resursa = min[res];
				else
					min[res] = stats[i][j].cost_minim_resursa;
				if (stats[i][j].cost_compl > min[!res])
					stats[i][j].cost_compl = min[!res];
				else
					min[!res] = stats[i][j].cost_compl;
			}
			else					// resursa de tip B
			{
				if (stats[i][j].cost_compl > min[res])
					stats[i][j].cost_compl = min[res];
				else
					min[res] = stats[i][j].cost_compl;
				if (stats[i][j].cost_minim_resursa > min[!res])
					stats[i][j].cost_minim_resursa = min[!res];
				else
					min[!res] = stats[i][j].cost_minim_resursa;
			}
		}

		// Calculez minimele de jos in sus
		min[0] = stats[n - 1][j].cost_minim_resursa;
		min[1] = stats[n - 1][j].cost_compl;
		for (int i = n - 2; i >= 0; --i)
		{
			bool res = stats[i + 1][j].resursa;
			int pret = stats[i + 1][j].pret_resursa;
			if (min[res] > pret)
				min[res] = pret + 1;
			else
				++min[res];
			++min[!res];

			if (res == false)		// resursa de tip A
			{
				if (stats[i][j].cost_minim_resursa > min[res])
					stats[i][j].cost_minim_resursa = min[res];
				else
					min[res] = stats[i][j].cost_minim_resursa;
				if (stats[i][j].cost_compl > min[!res])
					stats[i][j].cost_compl = min[!res];
				else
					min[!res] = stats[i][j].cost_compl;
			}
			else					// resursa de tip B
			{
				if (stats[i][j].cost_compl > min[res])
					stats[i][j].cost_compl = min[res];
				else
					min[res] = stats[i][j].cost_compl;
				if (stats[i][j].cost_minim_resursa > min[!res])
					stats[i][j].cost_minim_resursa = min[!res];
				else
					min[!res] = stats[i][j].cost_minim_resursa;
			}
		}

	}

	/*cout << "After columns up->down + down->up\n";
	printDebug(n, stats);*/

	// Compar ce am obtinut cu minimele pe linii
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
		{
			bool res = stats[i][j].resursa;
			if (res == 0)
			{
				stats[i][j].cost_minim_resursa = getMin3(stats[i][j].cost_minim_resursa,
						aux[i][j].cost_minim_resursa, stats[i][j].pret_resursa);
				stats[i][j].cost_compl = getMin2(stats[i][j].cost_compl,
						aux[i][j].cost_compl);
			}
			else
			{
				stats[i][j].cost_minim_resursa = getMin2(stats[i][j].cost_minim_resursa,
						aux[i][j].cost_minim_resursa);
				stats[i][j].cost_compl = getMin3(stats[i][j].cost_compl,
						aux[i][j].cost_compl, stats[i][j].pret_resursa);
			}
			/*stats[i][j].cost_minim_resursa = getMin3(stats[i][j].cost_minim_resursa,
					aux[i][j].cost_minim_resursa, stats[i][j].pret_resursa);
			stats[i][j].cost_compl = getMin3(stats[i][j].cost_compl,
					aux[i][j].cost_compl, stats[i][j].pret_resursa);*/
		}

	/*cout << "Final min\n";
	printDebug(n, stats);*/

}

void computeNextYear(const int n, Cell **stats, Cell **aux)
{
	computeMin(n, stats, aux);
	//printCostMin(n, stats);
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
		{
			int cost_compl, cost_min_res;
			bool res = stats[i][j].resursa;
			// Get minimum
			if (res == 0)
			{
				cost_compl = stats[i][j].cost_compl;
				cost_min_res = stats[i][j].cost_minim_resursa;
			}
			else
			{
				cost_compl = stats[i][j].cost_minim_resursa;
				cost_min_res = stats[i][j].cost_compl;
			}

			// Actualizare
			if (stats[i][j].buget < cost_compl)
			{
				stats[i][j].pret_resursa += cost_compl - stats[i][j].buget;
			}
			else if (stats[i][j].buget > cost_compl)
			{
				stats[i][j].pret_resursa += (cost_compl - stats[i][j].buget) / 2;
			}
			else
			{
				stats[i][j].pret_resursa = cost_min_res + 1;
			}
			stats[i][j].buget = cost_compl;

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

// Computes requested data for all years
void computeAllYears(const int n, Cell **stats, Cell **aux, ofstream &file_out)
{
	for (int k = 0; k < iteratii; ++k)
	{
		/*cout << "Pasul " << k << "\n";*/
		computeNextYear(n, stats, aux);

		writeOutput(file_out, n, stats);

		/*cout << "\n";
		printCosts(n, stats);*/
	}
	writeCosts(n, stats, file_out);
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
    //computeMin(n, stats, aux);
    //computeNextYear(n, stats, aux);
    computeAllYears(n, stats, aux, file_out);

    file_out.close();
    return 0;
}
