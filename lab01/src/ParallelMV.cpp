#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
using namespace std;

int ProcNum, ProcRank; // Number of available processes, rank of current process

void ProcessInitialization(double*& pMatrix, double*& pVector, double*& pResult, int& Size) {
    if (ProcRank == 0) {
        do {
            cout << "\nEnter size of the matrix and vector: ";
            cin >> Size;

            if (Size < ProcNum)
                cout << "Size must be greater than number of processes!\n";
            if (Size % ProcNum != 0)
                cout << "Size must be divisible by number of processes!\n";

        } while (Size < ProcNum || Size % ProcNum != 0);

        cout << "Chosen size = " << Size << "\n";
    }

    MPI_Bcast(&Size, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

int main(int argc, char* argv[]) {
    double* pMatrix; // First argument - initial matrix
    double* pVector; // Second argument - initial vector
    double* pResult; // Result vector for matrix-vector multiplication
    int Size; // Sizes of initial matrix and vector
    double Start, Finish, Duration;
    
    MPI_Init(&argc, &argv); // initialization of MPI envoriment
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum); //getting number of available processes
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);  // getting rank of current process

    if (ProcRank == 0)
        cout << "Parallel matrix-vector multiplication program\n";

    // Memory allocation and data initialization
    ProcessInitialization(pMatrix, pVector, pResult, Size);
    cout << "Process " << ProcRank << " received matrix of size " << Size << "\n";

    MPI_Finalize();
}