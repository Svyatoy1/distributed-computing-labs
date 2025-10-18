#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
#include <cmath>
using namespace std;

int ProcNum = 0; // Number of available processes
int ProcRank = 0; // Rank of current process

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

int main(int argc, char* argv[]) {
    double* pAMatrix; // First argument of matrix multiplication
    double* pBMatrix; // Second argument of matrix multiplication
    double* pCMatrix; // Result matrix

    int Size; // Size of matrices
    double Start, Finish, Duration;

    setvbuf(stdout, 0, _IONBF, 0);
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    if (ProcRank == 0)
        printf("Parallel matrix multiplication program\n");

    MPI_Finalize();
}