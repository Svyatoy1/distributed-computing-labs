#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
#include <cmath>
using namespace std;

int ProcNum = 0; // Number of the available processes
int ProcRank = 0; // Rank of the current process

int main(int argc, char* argv[]) {
    double* pMatrix; // Matrix of the linear system
    double* pVector; // Right parts of the linear system
    double* pResult; // Result vector
    int Size; // Size of the matrix and the vectors
    double Start, Finish, Duration;

    setvbuf(stdout, 0, _IONBF, 0);
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    if (ProcRank == 0)
        printf("Parallel Gauss algorithm for solving linear systems\n");
        
    MPI_Finalize();
}