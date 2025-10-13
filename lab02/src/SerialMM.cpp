#include <iostream>
#include <iomanip>
#include <ctime> // для функцій clock_t, clock, CLOCKS_PER_SEC
#include <cstdlib>
#include <limits>
using namespace std;

// Function for memory allocation and initialization of matrix elements
void ProcessInitialization (double* &pAMatrix, double* &pBMatrix, double* &pCMatrix, int &Size) {
    // Setting the size of matrices
    do {
        printf("\nEnter size of matrices: ");
        scanf("%d", &Size);
        printf("\nChosen matrices' size = %d", Size);
        if (Size <= 0)
            printf("\nSize of objects must be greater than 0!\n");
    }
    while (Size <= 0);
}

int main(){
    double* pAMatrix; // First argument of matrix multiplication
    double* pBMatrix; // Second argument of matrix multiplication
    double* pCMatrix; // Result matrix
    int Size; // Sizes of matrices

    time_t start, finish;
    double duration;

    cout << "Serial matrix multiplication program\n";
    ProcessInitialization(pAMatrix, pBMatrix, pCMatrix, Size);

    return 0;
}