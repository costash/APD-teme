#include <iostream>
#include <fstream>
#include <cstdlib>
using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        cerr << "Error: Usage\n";
        cerr << argv[0] << " T file_in file_out\n";
        return 1;
    }
    int iteratii = atoi(argv[1]);
    cout << "Nr iteratii: " << iteratii << "\n";

    ifstream file_in(argv[2], ios::in);
    ofstream file_out(argv[3], ios::out);

    if (!file_in.is_open())
    {
    	cerr << "Error: " << argv[0] << "\nInput file \"" << argv[2] << "\" does not exist\n";
    	return 1;
    }

    return 0;
}
