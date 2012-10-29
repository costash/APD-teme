#include "common.h"
#include <climits>
using namespace std;

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
				minim = stats[i][j].pret_resursa;
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
					minim = stats[i][j].pret_resursa;
			}
		return minim;
}

// Computes and adds minimums to matrix
void addMinCostToMatrix(const int n, Cell **& stats)
{
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
		{
			stats[i][j].cost_compl = getCostMinimResursaCompl(!stats[i][j].resursa,
					stats, n, i, j);
			stats[i][j].cost_minim_resursa = getCostMinimResursaProprie(stats[i][j].resursa,
					stats, n, i, j);
		}
}

int main(int argc, char *argv[])
{
	int n, pret_minim, pret_maxim, iteratii;
	Cell **stats;					// Matrice de structuri ce reprezinta un colonist

    checkArgs(argc, argv, iteratii);

    ifstream file_in(argv[2], ios::in);
    readInputSize(file_in, n, pret_minim, pret_maxim);

    cerr << "Nr iteratii: " << iteratii << "\n";
    cerr << "n: " << n << " pmin: " << pret_minim << " pmax: " << pret_maxim << endl;

    createMatrix(stats, n);

    readInput(file_in, n, stats);

    //cerr << "manhattan (0,0) -> (3, 4) = " << manhattan(0, 0, 3, 4);
    addMinCostToMatrix(n, stats);

    return 0;
}
