#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
#include <cmath>
using namespace std;

int ProcNum = 0; // Number of available processes
int ProcRank = 0; // Rank of current process

int GridSize; // Size of virtual processor grid
MPI_Comm GridComm; // Grid communicator
int GridCoords[2]; // Coordinates of current processor in grid
MPI_Comm ColComm; // Column communicator
MPI_Comm RowComm; // Row communicator

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

// Function for creating the two-dimensional grid communicator and
// communicators for each row and each column of the grid
void CreateGridCommunicators() {
    int DimSize[2]; // Number of processes in each dimension of the grid
    int Periodic[2]; // =1, if the grid dimension should be periodic
    int Subdims[2]; // =1, if the grid dimension should be fixed

    DimSize[0] = GridSize;
    DimSize[1] = GridSize;
    Periodic[0] = 1;
    Periodic[1] = 1;

    // Creation of the Cartesian communicator
    MPI_Cart_create(MPI_COMM_WORLD, 2, DimSize, Periodic, 1, &GridComm);

    // Determination of the cartesian coordinates for every process
    MPI_Cart_coords(GridComm, ProcRank, 2, GridCoords);

    // Creating communicators for rows
    Subdims[0] = 0; // Dimension is fixed
    Subdims[1] = 1; // Dimension belong to the subgrid
    MPI_Cart_sub(GridComm, Subdims, &RowComm);
    // Creating communicators for columns
    Subdims[0] = 1; // Dimension belong to the subgrid
    Subdims[1] = 0; // Dimension is fixed
    MPI_Cart_sub(GridComm, Subdims, &ColComm);
}

// Function for memory allocation and data initialization
void ProcessInitialization (double* &pAMatrix, double* &pBMatrix,
    double* &pCMatrix, double* &pAblock, double* &pBblock, double* &pCblock,
    double* &pMatrixAblock, int &Size, int &BlockSize ) {
    if (ProcRank == 0) {
        do {
            printf("\nEnter the size of the matrices: ");
            scanf("%d", &Size);
            if (Size%GridSize != 0) {
                printf ("Size of matrices must be divisible by the grid size! \n");
            }
        } while (Size%GridSize != 0);
    }
    MPI_Bcast(&Size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    BlockSize = Size/GridSize;
    pAblock = new double [BlockSize*BlockSize];
    pBblock = new double [BlockSize*BlockSize];
    pCblock = new double [BlockSize*BlockSize];
    pMatrixAblock = new double [BlockSize*BlockSize];

    if (ProcRank == 0) {
        pAMatrix = new double [Size*Size];
        pBMatrix = new double [Size*Size];
        pCMatrix = new double [Size*Size];
        DummyDataInitialization(pAMatrix, pBMatrix, Size);
    }

    for (int i=0; i<BlockSize*BlockSize; i++) {
        pCblock[i] = 0;
    }
}

// Function for checkerboard matrix decomposition
void CheckerboardMatrixScatter(double* pMatrix, double* pMatrixBlock, int Size, int BlockSize) {
    double * pMatrixRow = new double [BlockSize*Size];
    if (GridCoords[1] == 0) {
        MPI_Scatter(pMatrix, BlockSize*Size, MPI_DOUBLE, pMatrixRow,
        BlockSize*Size, MPI_DOUBLE, 0, ColComm);
    }

    for (int i=0; i<BlockSize; i++) {
        MPI_Scatter(&pMatrixRow[i*Size], BlockSize, MPI_DOUBLE,
        &(pMatrixBlock[i*BlockSize]), BlockSize, MPI_DOUBLE, 0, RowComm);
    }
    
    delete [] pMatrixRow;
}

// Function for data distribution among the processes
void DataDistribution(double* pAMatrix, double* pBMatrix, double* pMatrixAblock, double* pBblock, int Size, int BlockSize) {
    CheckerboardMatrixScatter(pAMatrix, pMatrixAblock, Size, BlockSize);
    CheckerboardMatrixScatter(pBMatrix, pBblock, Size, BlockSize);
}

// Function for computational process termination
void ProcessTermination (double* pAMatrix, double* pBMatrix,
double* pCMatrix, double* pAblock, double* pBblock, double* pCblock,
double* pMatrixAblock) {
    if (ProcRank == 0) {
        delete [] pAMatrix;
        delete [] pBMatrix;
        delete [] pCMatrix;
    }

    delete [] pAblock;
    delete [] pBblock;
    delete [] pCblock;
    delete [] pMatrixAblock;
}

// Test printing of the matrix block
void TestBlocks (double* pBlock, int BlockSize, const char str[]) {
    MPI_Barrier(MPI_COMM_WORLD);
    if (ProcRank == 0) {
        printf("%s \n", str);
    }
    for (int i=0; i<ProcNum; i++) {
        if (ProcRank == i) {
            printf ("ProcRank = %d \n", ProcRank);
            PrintMatrix(pBlock, BlockSize, BlockSize);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

int main(int argc, char* argv[]) {
    double* pAMatrix; // First argument of matrix multiplication
    double* pBMatrix; // Second argument of matrix multiplication
    double* pCMatrix; // Result matrix
    int Size; // Size of matrices
    int BlockSize; // Sizes of matrix blocks
    double *pMatrixAblock; // Initial block of matrix A
    double *pAblock; // Current block of matrix A
    double *pBblock; // Current block of matrix B
    double *pCblock; // Block of result matrix C    
    double Start, Finish, Duration;

    setvbuf(stdout, 0, _IONBF, 0);
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    GridSize = sqrt((double)ProcNum);
    if (ProcNum != GridSize*GridSize) {
        if (ProcRank == 0) {
            printf ("Number of processes must be a perfect square \n");
        }
    } else {
        if (ProcRank == 0)
            printf("Parallel matrix multiplication program\n");

        // Grid communicator creating
        CreateGridCommunicators();
    }

    // Memory allocation and initialization of matrix elements
    ProcessInitialization ( pAMatrix, pBMatrix, pCMatrix, pAblock, pBblock,
    pCblock, pMatrixAblock, Size, BlockSize );

    if (ProcRank == 0) {
        printf("Initial matrix A \n");
        PrintMatrix(pAMatrix, Size, Size);
        printf("Initial matrix B \n");
        PrintMatrix(pBMatrix, Size, Size);
    }

    // Data distribution among the processes
    DataDistribution(pAMatrix, pBMatrix, pMatrixAblock, pBblock, Size, BlockSize);
    TestBlocks(pMatrixAblock, BlockSize, "Initial blocks of matrix A");
    TestBlocks(pBblock, BlockSize, "Initial blocks of matrix B");

    // Process termination
    ProcessTermination(pAMatrix, pBMatrix, pCMatrix, pAblock, pBblock, pCblock, pMatrixAblock);

    MPI_Finalize();
}