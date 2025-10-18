#include <iostream>
#include <iomanip>
#include <ctime> // для функцій clock_t, clock, CLOCKS_PER_SEC
#include <cstdlib>
#include <limits>
using namespace std;

// Function for simple initialization of matrix elements
void DummyDataInitialization(double* pAMatrix, double* pBMatrix, int Size){
    int i, j; // Loop variables
    for (i=0; i<Size; i++) {
        for (j=0; j<Size; j++) {
            pAMatrix[i*Size+j] = 1;
            pBMatrix[i*Size+j] = 1;
        }
    }
}

// Function for random initialization of matrix elements
void RandomDataInitialization (double* pAMatrix, double* pBMatrix, int Size) {
    int i, j; // Loop variables
    srand(unsigned(clock()));
    for (i=0; i<Size; i++)
        for (j=0; j<Size; j++) {
            pAMatrix[i*Size+j] = rand()/double(1000);
            pBMatrix[i*Size+j] = rand()/double(1000);
    }
}

// Function for formatted matrix output
void PrintMatrix (double* pMatrix, int RowCount, int ColCount) {
    int i, j; // Loop variables
    for (i=0; i<RowCount; i++) {
        for (j=0; j<ColCount; j++)
        printf("%7.4f ", pMatrix[i*ColCount+j]);
        printf("\n");
    }
}

// Optimised function for matrix multiplication
void SerialResultCalculation(double* pAMatrix, double* pBMatrix, double* pCMatrix, int Size) {
    // transpose of matrix B to improve cache locality
    double* pBTransposed = new double[Size * Size];
    for (int i = 0; i < Size; i++) {
        for (int j = 0; j < Size; j++) {
            pBTransposed[j * Size + i] = pBMatrix[i * Size + j];
        }
    }

    // block multiplication for cache storage
    const int BLOCK = 64; // optimal block size for L1/L2 cash

    for (int i0 = 0; i0 < Size; i0 += BLOCK) {
        for (int j0 = 0; j0 < Size; j0 += BLOCK) {
            for (int k0 = 0; k0 < Size; k0 += BLOCK) {

                int iMax = min(i0 + BLOCK, Size);
                int jMax = min(j0 + BLOCK, Size);
                int kMax = min(k0 + BLOCK, Size);

                for (int i = i0; i < iMax; i++) {
                    for (int j = j0; j < jMax; j++) {
                        double sum = pCMatrix[i * Size + j];
                        const double* aRow = &pAMatrix[i * Size + k0];
                        const double* bRow = &pBTransposed[j * Size + k0];
                        for (int k = k0; k < kMax; k++) {
                            sum += aRow[k - k0] * bRow[k - k0];
                        }
                        pCMatrix[i * Size + j] = sum;
                    }
                }
            }
        }
    }

    delete[] pBTransposed;
}

// Function for memory allocation and initialization of matrix elements
void ProcessInitialization (double* &pAMatrix, double* &pBMatrix, double* &pCMatrix, int &Size) {
    // Setting the size of matrices
    do {
        printf("\nEnter size of matrices: ");
        if (scanf("%d", &Size) != 1) {
            fprintf(stderr, "Invalid input.\n");
            exit(1);
        };
        printf("\nChosen matrices' size = %d", Size);
        if (Size <= 0)
            printf("\nSize of objects must be greater than 0!\n");
    }
    while (Size <= 0);

    // Memory allocation
    pAMatrix = new double [Size*Size];
    pBMatrix = new double [Size*Size];
    pCMatrix = new double [Size*Size];

    // Random initialization of matrix elements
    RandomDataInitialization(pAMatrix, pBMatrix, Size);
    for (int i=0; i<Size*Size; i++) {
        pCMatrix[i] = 0;
    }
}

// Function for computational process termination
void ProcessTermination (double* pAMatrix, double* pBMatrix,double* pCMatrix) {
    delete [] pAMatrix;
    delete [] pBMatrix;
    delete [] pCMatrix;
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

    // Matrix output
    //cout << "\nInitial A Matrix \n";
    //PrintMatrix(pAMatrix, Size, Size);
    //cout << "Initial B Matrix \n";
    //PrintMatrix(pBMatrix, Size, Size);

    // Matrix multiplication
    start = clock();
    SerialResultCalculation(pAMatrix, pBMatrix, pCMatrix, Size);
    finish = clock();
    duration = (finish-start)/double(CLOCKS_PER_SEC);
    
    // Printing the result matrix
    //cout << "\n Result Matrix: \n";
    //PrintMatrix(pCMatrix, Size, Size);

    // Printing the time spent by matrix multiplication
    printf("\n Time of execution: %f\n", duration);

    // Computational process termination
    ProcessTermination(pAMatrix, pBMatrix, pCMatrix);
}