#include <iostream>
#include <iomanip>
#include <ctime> // для функцій clock_t, clock, CLOCKS_PER_SEC
#include <cstdlib>
using namespace std;

double* pMatrix; // First argument - initial matrix
double* pVector; // Second argument - initial vector
double* pResult; // Result vector for matrix-vector multiplication
int Size; // Sizes of initial matrix and vector

int main () {
    cout << "Serial matrix-vector multiplication program" << '\n';
    cin.get(); // крос-платформена заміна getch()
}