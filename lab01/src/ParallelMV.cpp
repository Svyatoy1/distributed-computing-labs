#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
using namespace std;

int ProcNum, ProcRank; // Number of available processes, rank of current process

int main(int argc, char* argv[]) {
    double* pMatrix; // First argument - initial matrix
    double* pVector; // Second argument - initial vector
    double* pResult; // Result vector for matrix-vector multiplication
    int Size; // Sizes of initial matrix and vector
    double Start, Finish, Duration;
    
    MPI_Init(&argc, &argv); // initialization of MPI envoriment
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum); //getting number of available processes
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);  // getting rank of current process

    if (ProcRank == 0) {
        cout << "Parallel matrix-vector multiplication program" << endl;
        cout << "Number of available processes = " << ProcNum << endl;
    }

    cout << "Rank of current process = " << ProcRank << endl;

    MPI_Finalize();
}