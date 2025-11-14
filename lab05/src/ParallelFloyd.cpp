#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <mpi.h>
#include <iostream>
using namespace std;

int ProcNum; // Number of available processes
int ProcRank; // Rank of current process
int *pMatrix; // Adjacency matrix
int Size; // Size of adjacency matrix
const double InfinitiesPercent = 50.0;
const double RandomDataMultiplier = 10;

// Function for simple setting the initial data
void DummyDataInitialization(int *pMatrix, int Size) {
    for(int i = 0; i < Size; i++)
        for(int j = i; j < Size; j++) {
            if(i == j) 
                pMatrix[i * Size + j] = 0;
            else
                if(i == 0) 
                    pMatrix[i * Size + j] = j;
            else 
                pMatrix[i * Size + j] = -1;
                pMatrix[j * Size + i] = pMatrix[i * Size + j];
        }
}

// Function for initializing the data by the random generator
void RandomDataInitialization(int *pMatrix, int Size) {
    srand( (unsigned)time(0) );
    for(int i = 0; i < Size; i++)
        for(int j = 0; j < Size; j++)
            if(i != j) {
                if((rand() % 100) < InfinitiesPercent)
                    pMatrix[i * Size + j] = -1;
                else
                    pMatrix[i * Size + j] = rand() + 1;
            }
            else
                pMatrix[i * Size + j] = 0;
}

int Min(int A, int B) {
    int Result = (A < B) ? A : B;
    if((A < 0) && (B >= 0)) Result = B;
    if((B < 0) && (A >= 0)) Result = A;
    if((A < 0) && (B < 0)) Result = -1;
    return Result;
}

// Function for formatted matrix output
void PrintMatrix(int *pMatrix, int RowCount, int ColCount) {
    for(int i = 0; i < RowCount; i++) {
        for(int j = 0; j < ColCount; j++)
            printf("%7d", pMatrix[i * ColCount + j]);
        printf("\n");
    }
}

// Function for allocating the memory and setting the initial values
void ProcessInitialization(int *&pMatrix, int *&pProcRows, int& Size, int& RowNum) {
    setvbuf(stdout, 0, _IONBF, 0);
    if(ProcRank == 0) {
        do {
            printf("Enter the number of vertices: ");
            scanf("%d", &Size);
            if(Size < ProcNum)
                printf("The number of vertices should be greater than the number of processes\n");
            if(Size % ProcNum != 0)
                printf("The number of vertices should be divisible by the number of processes\n");
        } while((Size < ProcNum) || (Size % ProcNum != 0));

        printf("Using the graph with %d vertices\n", Size);
    }
    // Broadcast the number of vertices
    MPI_Bcast(&Size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Number of rows for each process
    RowNum = Size / ProcNum;

    // Allocate memory for the current process rows
    pProcRows = new int[Size * RowNum];

    if(ProcRank == 0) {
        // Allocate memory for the adjacency matrix
        pMatrix = new int[Size * Size];

    // Data initalization
    DummyDataInitialization(pMatrix, Size);
    }
}

// Function for computational process termination
void ProcessTermination(int *pMatrix) {
    delete []pMatrix;
}


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

    // Process initialization
    ProcessInitialization(pMatrix, pProcRows, Size, RowNum); 

    MPI_Finalize();
    return 0;
}