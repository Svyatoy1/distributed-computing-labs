#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
using namespace std;

int ProcNum, ProcRank; // Number of available processes, rank of current process

// Function for simple definition of matrix and vector elements
void DummyDataInitialization(double* pMatrix, double* pVector, int Size) {
    for (int i = 0; i < Size; ++i) {
        pVector[i] = 1.0;
        for (int j = 0; j < Size; ++j)
            pMatrix[i * Size + j] = static_cast<double>(i);
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
    
// Function for formatted vector output
void PrintVector (double* pVector, int Size) {
    int i;
    for (i=0; i<Size; i++)
    printf("%7.4f ", pVector[i]);
    printf("\n");
}

void ProcessInitialization(double*& pMatrix, double*& pVector, double*& pResult, 
    double*& pProcRows, double*& pProcResult, int& Size, int& RowNum) {
    if (ProcRank == 0) {
        do {
            cout << "\nEnter size of the matrix and vector: ";
            cin >> Size;
            if (Size < ProcNum)
                cout << "Size must be greater than number of processes!\n";
            if (Size % ProcNum != 0)
                cout << "Size must be divisible by number of processes!\n";
        } while (Size < ProcNum || Size % ProcNum != 0);
    }

    MPI_Bcast(&Size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    RowNum = Size / ProcNum;

    pVector = new double[Size];
    pResult = new double[Size];
    pProcRows = new double[RowNum * Size];
    pProcResult = new double[RowNum];

    if (ProcRank == 0) {
        pMatrix = new double[Size * Size];
        DummyDataInitialization(pMatrix, pVector, Size);

        cout << "\nInitial Matrix (on root process):\n";
        PrintMatrix(pMatrix, Size, Size);

        cout << "\nInitial Vector:\n";
        PrintVector(pVector, Size);
    }
}

// Function for computational process termination
void ProcessTermination (double* pMatrix, double* pVector, double* pResult, double* pProcRows, double* pProcResult) {
    if (ProcRank == 0) {
        delete [] pMatrix;
        delete [] pVector;
        delete [] pResult;
        delete [] pProcRows;
        delete [] pProcResult;
    }
}

int main(int argc, char* argv[]) {
    double* pMatrix; // First argument - initial matrix
    double* pVector; // Second argument - initial vector
    double* pResult; // Result vector for matrix-vector multiplication
    double* pProcRows;
    double* pProcResult;
    int Size; // Sizes of initial matrix and vector
    int RowNum;
    double Start, Finish, Duration;
    
    MPI_Init(&argc, &argv); // initialization of MPI envoriment
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum); //getting number of available processes
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);  // getting rank of current process

    if (ProcRank == 0)
        cout << "Parallel matrix-vector multiplication program\n";

    ProcessInitialization(pMatrix, pVector, pResult, pProcRows, pProcResult, Size, RowNum);

    ProcessTermination(pMatrix, pVector, pResult, pProcRows, pProcResult);

    MPI_Finalize();
}