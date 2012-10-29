#include "common.h"
using namespace std;




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

    cerr << "manhattan (0,0) -> (3, 4) = " << manhattan(0, 0, 3, 4);

    return 0;
}
