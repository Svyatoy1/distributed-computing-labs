#include <iostream>
#include <iomanip>
#include <ctime> // для функцій clock_t, clock, CLOCKS_PER_SEC
#include <cstdlib>
#include <limits>
using namespace std;

double* pMatrix; // First argument - initial matrix
double* pVector; // Second argument - initial vector
double* pResult; // Result vector for matrix-vector multiplication
int Size; // Sizes of initial matrix and vector

void ProcessInitialization(double* &pMatrix, double* &pVector, double* &pResult, int &Size) {
    do {
        cout << "\nEnter size of the initial objects: ";
        if (!(cin >> Size)) {
            cout << "Please enter a positive integer!\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            Size = 0;
            continue;
        }
        cout << "\nChosen objects size = " << Size << '\n';
        if (Size <= 0)
            cout << "Size of objects must be greater than 0!\n";
    } while (Size <= 0);
}

int main() {
    cout << "Serial matrix-vector multiplication program\n";

    ProcessInitialization(pMatrix, pVector, pResult, Size);
    cout << "Press Enter to exit";

    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // прибрати \n після числа
    cin.get();
    return 0;
}