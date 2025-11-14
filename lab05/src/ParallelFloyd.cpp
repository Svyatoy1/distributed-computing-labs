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

// Function for comparing the matrices
bool CompareMatrices(int *pMatrix1, int *pMatrix2, int Size) {
    return equal(pMatrix1, pMatrix1 + Size * Size, pMatrix2);
}

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

// Function for formatted output of all stripes
void ParallelPrintMatrix(int *pProcRows, int Size, int RowNum) {
    for(int i = 0; i < ProcNum; i++) {
        if (ProcRank == i) {
            printf("ProcRank = %d\n", ProcRank);
            printf("Proc rows:\n");
            PrintMatrix(pProcRows, RowNum, Size);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

// Function for the data distribution among the processes
void DataDistribution(int *pMatrix, int *pProcRows, int Size, int RowNum) {
    MPI_Scatter(pMatrix, RowNum * Size, MPI_INT, pProcRows, RowNum * Size, MPI_INT, 0, MPI_COMM_WORLD);
}

// Function for testing the data distribution
void TestDistribution(int *pMatrix, int *pProcRows, int Size, int RowNum) {
    MPI_Barrier(MPI_COMM_WORLD);
    if (ProcRank == 0) {
        printf("Initial adjacency matrix:\n");
        PrintMatrix(pMatrix, Size, Size);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    ParallelPrintMatrix(pProcRows, Size, RowNum);
}

// Function for row broadcasting among all processes
void RowDistribution(int *pProcRows, int Size, int RowNum, int k, int *pRow) {
    int ProcRowRank = k / RowNum; // Process rank with the row k
    int ProcRowNum = k - ProcRowRank * RowNum; // Process row number

    if(ProcRowRank == ProcRank)
        // Copy the row to pRow array
        copy(&pProcRows[ProcRowNum*Size],&pProcRows[(ProcRowNum+1)*Size],pRow);

    // Broadcast row to all processes
    MPI_Bcast(pRow, Size, MPI_INT, ProcRowRank, MPI_COMM_WORLD);
}

// Function for the parallel Floyd algorithm
void ParallelFloyd(int *pProcRows, int Size, int RowNum) {
    int *pRow = new int[Size];
    int t1, t2;

    for(int k = 0; k < Size; k++) {
        // Distribute row among all processes
        // Update adjacency matrix elements
        for(int i = 0; i < RowNum; i++)
            for(int j = 0; j < Size; j++)
                if( (pProcRows[i * Size + k] != -1) && (pRow [j] != -1)) {
                    t1 = pProcRows[i * Size + j];
                    t2 = pProcRows[i * Size + k] + pRow[j];
                    pProcRows[i * Size + j] = Min(t1, t2);
                }
    }
    
    delete []pRow;
}

// Function for the serial Floyd algorithm
void SerialFloyd(int *pMatrix, int Size) {
    int t1, t2;
    for(int k = 0; k < Size; k++)
        for(int i = 0; i < Size; i++)
            for(int j = 0; j < Size; j++)
                if((pMatrix[i * Size + k] != -1) && (pMatrix[k * Size + j] != -1)) {
                    t1 = pMatrix[i * Size + j];
                    t2 = pMatrix[i * Size + k] + pMatrix[k * Size + j];
                    pMatrix[i * Size + j] = Min(t1, t2);
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
    //DummyDataInitialization(pMatrix, Size);
    RandomDataInitialization(pMatrix, Size);
    }
}

// Function for process result collection
void ResultCollection(int *pMatrix, int *pProcRows, int Size, int RowNum) {
    MPI_Gather(pProcRows, RowNum * Size, MPI_INT, pMatrix, RowNum * Size, MPI_INT, 0, MPI_COMM_WORLD);
}

// Function for copying the matrix
void CopyMatrix(int *pMatrix, int Size, int *pMatrixCopy) {
    copy(pMatrix, pMatrix + Size * Size, pMatrixCopy);
}

// Function for testing the result of parallel Floyd algorithm
void TestResult(int *pMatrix, int *pSerialMatrix, int Size) {
    MPI_Barrier(MPI_COMM_WORLD);
    if(ProcRank == 0) {
        SerialFloyd(pSerialMatrix, Size);
        if(!CompareMatrices (pMatrix, pSerialMatrix, Size)) {
            printf("The results of serial and parallel algorithms are NOT identical. Check your code\n");
        }
        else {
            printf("The results of serial and parallel algorithms are identical\n");
        }
    }
}

// Function for computational process termination
void ProcessTermination(int *pMatrix, int *pProcRows) {
    if(ProcRank == 0)
        delete []pMatrix;
    delete []pProcRows;
}


int main (int argc, char* argv[]) {
    int *pMatrix; // Adjacency matrix
    int Size; // Size of adjacency matrix
    int *pProcRows; // Process rows
    int RowNum; // Number of process rows
    int *pSerialMatrix = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    if(ProcRank == 0)
        printf("Parallel Floyd algorithm \n");

    // Process initialization
    ProcessInitialization(pMatrix, pProcRows, Size, RowNum); 

    if (ProcRank == 0) {
        // Matrix copying
        pSerialMatrix = new int[Size * Size];
        CopyMatrix(pMatrix, Size, pSerialMatrix);
    }

    // Distributing the initial data among processes
    DataDistribution(pMatrix, pProcRows, Size, RowNum);

    // Parallel Floyd algorithm
    ParallelFloyd(pProcRows, Size, RowNum);
    ParallelPrintMatrix(pProcRows, Size, RowNum);

    // Process data collection
    ResultCollection(pMatrix, pProcRows, Size, RowNum);
    
    // Process termination
    ProcessTermination(pMatrix, pProcRows);
    if (ProcRank == 0)
        delete []pSerialMatrix;

    MPI_Finalize();
}