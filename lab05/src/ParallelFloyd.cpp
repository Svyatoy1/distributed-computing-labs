#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <mpi.h>

int ProcNum; // Number of available processes
int ProcRank; // Rank of current process

int main (int argc, char* argv[]) {
    int *pMatrix; // Adjacency matrix
    int Size; // Size of adjacency matrix
    int *pProcRows; // Process rows
    int RowNum; // Number of process rows

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    if(ProcRank == 0)
        printf("Parallel Floyd algorithm \n");
        
    MPI_Finalize();
}