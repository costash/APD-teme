#include "common.h"
#include <climits>
#include <omp.h>
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

// Calculeaza minimul a 2 intregi
inline int getMin2(const int a, const int b)
{
	return a > b ? b : a;
}

// Calculeaza minimul a 3 intregi
inline int getMin3(const int a, const int b, const int c)
{
	int aux = getMin2(a, b);
	return getMin2(aux, c);
}

// Copy all elements from matrix a to matrix b
void copyMatrix(const int n, Cell **a, Cell **b)
{
	#pragma omp parallel for
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			b[i][j] = a[i][j];
}


void computeMin(const int n, Cell **& stats, Cell **& aux)
{
	#ifdef DEBUG
	cout << "Before lines_left->right\n";
	printDebug(n, stats);
	#endif
	// Calculez minimele pe linii
	#pragma omp parallel for
	for (int i = 0; i < n; ++i)
	{

		int min[2] = {INT_MAX - 2 * n, INT_MAX - 2 * n};
		stats[i][0].cost_minim_resursa = INT_MAX - 2 * n;
		stats[i][0].cost_compl = INT_MAX - 2 * n;

		// Minime pe linii de la stanga la dreapta
		for (int j = 1; j < n; ++j)
		{
			// pentru fiecare colonist, ma uit daca minimul calculat global este
			// mai mic decat pretul resursei produse de vecinul din stanga + 1
			bool res = stats[i][j - 1].resursa;
			int pret = stats[i][j - 1].pret_resursa;
			if (min[res] > pret)
				min[res] = pret + 1;
			else
				++min[res];

			// Minimul pentru resursa opusa creste cu 1 (distanta de la vecin la mine)
			++min[!res];

			// Updatez minimele conform minimului calculat
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

		// Minime pe linii de la dreapta la stanga
		min[0] = stats[i][n - 1].cost_minim_resursa;
		min[1] = stats[i][n - 1].cost_compl;
		for (int j = n - 2; j >= 0; --j)
		{
			// Ma uit la vecinul din dreapta mereu
			bool res = stats[i][j + 1].resursa;
			int pret = stats[i][j + 1].pret_resursa;
			if (min[res] > pret)
				min[res] = pret + 1;
			else
				++min[res];

			++min[!res];
			if (res == false)		// resursa de tip A
			{
				// Trebuiesc considerate si minimele anterioare, repsectiv updatate
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
				// Trebuiesc considerate si minimele anterioare, repsectiv updatate
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

	#ifdef DEBUG
	cout << "After lines left->Right + right->left\n";
	printDebug(n, stats);
	#endif

	// Calculez minimele pe coloane, pe matricea obtinuta
	#pragma omp parallel for
	for (int j = 0; j < n; ++j)
	{
		int min[2];
		min[0] = stats[0][j].cost_minim_resursa;
		min[1] = stats[0][j].cost_compl;
		// Calculez minimele de sus in jos
		for (int i = 1; i < n; ++i)
		{
			// Ma uit mereu la vecinul de deasupra
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
			// Ma uit mereu la vecinul de dedesubt
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

	#ifdef DEBUG
	cout << "After columns up->down + down->up\n";
	printDebug(n, stats);
	#endif

	// Compar ce am obtinut cu minimele pe linii
	// Aceasta operatie poate fi de asemeni paralelizata, datele fiind independente intre threaduri
	#pragma omp parallel for
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
		{
			bool res = stats[i][j].resursa;
			// Trebuie sa consider si resursa pe care o produc eu, deci fac minim intre
			// costul minim pentru resursa curenta calculat pe linii, respectiv linii + coloane
			// si pretul resursei produse de mine
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
		}

	#ifdef DEBUG
	cout << "Final min\n";
	printDebug(n, stats);
	#endif

}

// Computes the matrix for next year
void computeNextYear(const int n, Cell **stats, Cell **aux)
{
	// Calculez minimele pentru fiecare colonist
	computeMin(n, stats, aux);
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
			int cost_compl, cost_min_res;
			bool res = stats[i][j].resursa;
			// Get minimum for this element's resource type
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

			// Actualizare informatii
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

// Computes requested data for all years
void computeAllYears(const int n, Cell **stats, Cell **aux, ofstream &file_out)
{
	for (int k = 0; k < iteratii; ++k)
	{
		#ifdef DEBUG
		cout << "Pasul " << k << "\n";
		#endif

		// Compute values for next year and write output to file
		computeNextYear(n, stats, aux);
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
	Cell **aux;						// Aux matrix

    checkArgs(argc, argv, iteratii);

    ifstream file_in(argv[2], ios::in);
    readInputSize(file_in, n, pret_minim, pret_maxim);

	#ifdef DEBUG
    cerr << "Nr iteratii: " << iteratii << "\n";
    cerr << "n: " << n << " pmin: " << pret_minim << " pmax: " << pret_maxim << endl;
	#endif

    createMatrix(stats, n);
    createMatrix(aux, n);

    readInput(file_in, n, stats);
    file_in.close();

    ofstream file_out(argv[3], ios::out);

	#ifdef DEBUG
    cout << "Costurile initiale \n";
    printCosts(n, stats);
    cout << endl;

    cout << "Auxiliar \n";
    printCosts(n, aux);
	#endif

    // This contains the main loop
    computeAllYears(n, stats, aux, file_out);

    file_out.close();
    return 0;
}
