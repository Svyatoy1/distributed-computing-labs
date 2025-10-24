#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
#include <cmath>
using namespace std;

int ProcNum = 0; // Number of the available processes
int ProcRank = 0; // Rank of the current process

int* pSerialPivotPos; // Number of pivot rows selected at the iterations
int* pSerialPivotIter; // Iterations, at which the rows were pivots

int* pProcInd; // Number of the first row located on the processes
int* pProcNum; // Number of the linear system rows located on the processes

int *pParallelPivotPos; // Number of rows selected as the pivot ones
int *pProcPivotIter; // Number of iterations, at which the process rows were used as the pivot ones

// Function for simple initialization of the matrix and the vector elements
void DummyDataInitialization (double* pMatrix, double* pVector, int Size) {
    int i, j; // Loop variables
    for (i=0; i<Size; i++) {
        pVector[i] = i+1;
        for (j=0; j<Size; j++) {
            if (j <= i)
                pMatrix[i*Size+j] = 1;
            else
                pMatrix[i*Size+j] = 0;
        }
    }
}

// Function for random initialization of the matrix and the vector elements
void RandomDataInitialization(double* pMatrix, double* pVector, int Size) {
    int i, j; // Loop variables
    srand(unsigned(clock()));
    for (i=0; i<Size; i++) {
        pVector[i] = rand()/double(1000);
        for (j=0; j<Size; j++) {
            if (j <= i)
                pMatrix[i*Size+j] = rand()/double(1000);
            else
                pMatrix[i*Size+j] = 0;
        }
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

// Function for finding the pivot row
int FindPivotRow(double* pMatrix, int Size, int Iter) {
    int PivotRow = -1; // Index of the pivot row
    double MaxValue = 0; // Value of the pivot element
    int i; // Loop variable

    // Choose the row, that stores the maximum element
    for (i=0; i<Size; i++) {
        if ((pSerialPivotIter[i] == -1) && (fabs(pMatrix[i*Size+Iter]) > MaxValue)) {
            PivotRow = i;
            MaxValue = fabs(pMatrix[i*Size+Iter]);
        }
    }
    return PivotRow;
}

// Function for the column elimination
void SerialColumnElimination (double* pMatrix, double* pVector, int Pivot, int Iter, int Size) {
    double PivotValue, PivotFactor;
    PivotValue = pMatrix[Pivot*Size+Iter];
    for (int i=0; i<Size; i++) {
        if (pSerialPivotIter[i] == -1) {
            PivotFactor = pMatrix[i*Size+Iter] / PivotValue;
            for (int j=Iter; j<Size; j++) {
                pMatrix[i*Size + j] -= PivotFactor * pMatrix[Pivot*Size+j];
            }
            pVector[i] -= PivotFactor * pVector[Pivot];
        }
    }
}

// Function for the Gaussian elimination
void SerialGaussianElimination(double* pMatrix,double* pVector,int Size) {
    int Iter; // Number of the iteration of the Gaussian elimination
    int PivotRow; // Number of the current pivot row
        for (Iter=0; Iter<Size; Iter++) {
        // Finding the pivot row
            PivotRow = FindPivotRow(pMatrix, Size,Iter);
            pSerialPivotPos[Iter] = PivotRow;
            pSerialPivotIter[PivotRow] = Iter;
            SerialColumnElimination(pMatrix, pVector, PivotRow, Iter, Size);
        }
    //printf ("The matrix of the linear system after the elimination: \n");
    //PrintMatrix(pMatrix, Size, Size);
}

// Function for the back substution
void SerialBackSubstitution (double* pMatrix, double* pVector, double* pResult, int Size) {
    int RowIndex, Row;
    for (int i=Size-1; i>=0; i--) {
        RowIndex = pSerialPivotPos[i];
        pResult[i] = pVector[RowIndex]/pMatrix[Size*RowIndex+i];
        for (int j=0; j<i; j++) {
            Row = pSerialPivotPos[j];
            pVector[j] -= pMatrix[Row*Size+i]*pResult[i];
            pMatrix[Row*Size+i] = 0;
        }
    }
}

// Function for the execution of Gauss algorithm
void SerialResultCalculation(double* pMatrix, double* pVector, double* pResult, int Size) {
    // Memory allocation
    pSerialPivotPos = new int [Size];
    pSerialPivotIter = new int [Size];
    for (int i=0; i<Size; i++) {
        pSerialPivotIter[i] = -1;
    }

    // Gaussian elimination
    SerialGaussianElimination (pMatrix, pVector, Size);

    // Back substitution
    SerialBackSubstitution (pMatrix, pVector, pResult, Size);

    // Memory deallocation
    delete [] pSerialPivotPos;
    delete [] pSerialPivotIter;
}

// Function for memory allocation and data initialization
void ProcessInitialization (double* &pMatrix, double* &pVector, double* &pResult, double* &pProcRows, 
double* &pProcVector, double* &pProcResult, int &Size, int &RowNum) {
    if (ProcRank == 0) {
        do {
            printf("\nEnter the size of the matrix and the vector: ");
            scanf("%d", &Size);
            if (Size < ProcNum) {
                printf ("Size must be greater than number of processes! \n");
            }
        } while (Size < ProcNum);
    }
    MPI_Bcast(&Size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int RestRows = Size;
    for (int i=0; i<ProcRank; i++)
        RestRows = RestRows-RestRows/(ProcNum-i);

    RowNum = RestRows/(ProcNum-ProcRank);

    pProcRows = new double [RowNum*Size];
    pProcVector = new double [RowNum];
    pProcResult = new double [RowNum];

    if (ProcRank == 0) {
        pMatrix = new double [Size*Size];
        pVector = new double [Size];
        pResult = new double [Size];
        // Initialization of the matrix and the vector elements
        DummyDataInitialization (pMatrix, pVector, Size);
    }
}

// Function for the data distribution among the processes
void DataDistribution(double* pMatrix, double* pProcRows, double* pVector, double* pProcVector, int Size, int RowNum) {
    int *pSendNum; // Number of the elements sent to the process
    int *pSendInd; // Index of the first data element sent to the process
    int RestRows=Size; // Number of rows, that have not been distributed yet
    int i; // Loop variable

    // Alloc memory for temporary objects
    pSendInd = new int [ProcNum];
    pSendNum = new int [ProcNum];

    // Define the disposition of the matrix rows for the current process
    RowNum = (Size/ProcNum);
    pSendNum[0] = RowNum*Size;
    pSendInd[0] = 0;

    for (i=1; i<ProcNum; i++) {
        RestRows -= RowNum;
        RowNum = RestRows/(ProcNum-i);
        pSendNum[i] = RowNum*Size;
        pSendInd[i] = pSendInd[i-1]+pSendNum[i-1];
    }

    // Scatter the rows
    MPI_Scatterv(pMatrix, pSendNum, pSendInd, MPI_DOUBLE, pProcRows, pSendNum[ProcRank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Free the memory
    delete [] pSendNum;
    delete [] pSendInd;
}

// Function for testing the data distribution
void TestDistribution(double* pMatrix, double* pVector, double* pProcRows, double* pProcVector, int Size, int RowNum) {
    if (ProcRank == 0) {
        printf("Initial Matrix: \n");
        PrintMatrix(pMatrix, Size, Size);
        printf("Initial Vector: \n");
        PrintVector(pVector, Size);
    }

    for (int i=0; i<ProcNum; i++) {
        if (ProcRank == i) {
            printf("\nProcRank = %d \n", ProcRank);
            printf(" Matrix Stripe:\n");
            PrintMatrix(pProcRows, RowNum, Size);
            printf(" Vector: \n");
            PrintVector(pProcVector, RowNum);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

// Fuction for the column elimination
void ParallelEliminateColumns(double* pProcRows, double* pProcVector, double* pPivotRow, int Size, int RowNum, int Iter) {
    double PivotFactor;
    for (int i=0; i<RowNum; i++) {
        if (pProcPivotIter[i] == -1) {
            PivotFactor = pProcRows[i*Size+Iter] / pPivotRow[Iter];
            for (int j=Iter; j<Size; j++) {
                pProcRows[i*Size + j] -= PivotFactor* pPivotRow[j];
            }
            pProcVector[i] -= PivotFactor * pPivotRow[Size];
        }
    }
}

// Function for the Gaussian elimination
void ParallelGaussianElimination (double* pProcRows, double* pProcVector, int Size, int RowNum) {
    double MaxValue; // Value of the pivot element of thе process
    int PivotPos; // Position of the pivot row in the process stripe

    struct { double MaxValue; int ProcRank; } ProcPivot, Pivot;
    double *pPivotRow; // Pivot row of the current iteration
    pPivotRow = new double [Size+1];

    // The iterations of the Gaussian elimination
    for (int i=0; i<Size; i++) {
        // Calculating the local pivot row
        for (int j=0; j<RowNum; j++) {
            if ((pProcPivotIter[j] == -1) && (MaxValue < fabs(pProcRows[j*Size+i]))) {
                MaxValue = fabs(pProcRows[j*Size+i]);
                PivotPos = j;
            }
        }

        // Finding the global pivot row
        ProcPivot.MaxValue = MaxValue;
        ProcPivot.ProcRank = ProcRank;

        // Finding the pivot process
        MPI_Allreduce(&ProcPivot, &Pivot, 1, MPI_DOUBLE_INT, MPI_MAXLOC, MPI_COMM_WORLD);

        // Storing the number of the pivot row
        if ( ProcRank == Pivot.ProcRank ){
            pProcPivotIter[PivotPos]= i;
            pParallelPivotPos[i]= pProcInd[ProcRank] + PivotPos;
        }
        MPI_Bcast(&pParallelPivotPos[i], 1, MPI_INT, Pivot.ProcRank, MPI_COMM_WORLD);

        // Broadcasting the pivot row
        if ( ProcRank == Pivot.ProcRank ){
            // Fill the pivot row
            for (int j=0; j<Size; j++) {
                pPivotRow[j] = pProcRows[PivotPos*Size + j];
            }
            pPivotRow[Size] = pProcVector[PivotPos];
        }
        MPI_Bcast(pPivotRow, Size+1, MPI_DOUBLE, Pivot.ProcRank, MPI_COMM_WORLD);

        // Column elimination
        ParallelEliminateColumns(pProcRows, pProcVector, pPivotRow, Size, RowNum, i);
    }
    delete [] pPivotRow;
}

// Function for execution of the parallel Gauss algorithm
void ParallelResultCalculation(double* pProcRows, double* pProcVector, double* pProcResult, int Size, int RowNum) {
    // Memory allocation
    pParallelPivotPos = new int [Size];
    pProcPivotIter = new int [RowNum];
    for (int i=0; i<RowNum; i++)
        pProcPivotIter[i] = -1;

    // Gaussian elimination
    ParallelGaussianElimination (pProcRows, pProcVector, Size, RowNum);
    // Back substitution
    //ParallelBackSubstitution (pProcRows, pProcVector, pProcResult, Size, RowNum);

    // Memory deallocation
    delete [] pParallelPivotPos;
    delete [] pProcPivotIter;
}

// Function for computational process termination
void ProcessTermination (double* pMatrix, double* pVector, double* pResult,
double* pProcRows, double* pProcVector, double* pProcResult) {
    if (ProcRank == 0) {
        delete [] pMatrix;
        delete [] pVector;
        delete [] pResult;
    }

    delete [] pProcRows;
    delete [] pProcVector;
    delete [] pProcResult;
}

int main(int argc, char* argv[]) {
    double* pMatrix; // Matrix of the linear system
    double* pVector; // Right parts of the linear system
    double* pResult; // Result vector
    double *pProcRows; // Rows of the matrix A
    double *pProcVector; // Block of the vector b
    double *pProcResult; // Block of the vector x
    int Size; // Size of the matrix and the vectors
    int RowNum; // Number of the matrix rows
    double Start, Finish, Duration;

    setvbuf(stdout, 0, _IONBF, 0);
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    if (ProcRank == 0)
        printf("Parallel Gauss algorithm for solving linear systems\n");

    // Memory allocation and data initialization
    ProcessInitialization(pMatrix, pVector, pResult, pProcRows, pProcVector, pProcResult, Size, RowNum);

    // Distributing the initial data between the processes
    DataDistribution(pMatrix, pProcRows, pVector, pProcVector, Size, RowNum);
    TestDistribution(pMatrix, pVector, pProcRows, pProcVector, Size, RowNum);

    if (ProcRank == 0) {
        printf("Initial matrix \n");
        PrintMatrix(pMatrix, Size, Size);
        printf("Initial vector \n");
        PrintVector(pVector, Size);
    }

    // The execution of the parallel Gauss algorithm
    ParallelResultCalculation (pProcRows, pProcVector, pProcResult, Size, RowNum);
    TestDistribution(pMatrix, pVector, pProcRows, pProcVector, Size, RowNum);

    // Process termination
    ProcessTermination (pMatrix, pVector, pResult, pProcRows, pProcVector, pProcResult);
    MPI_Finalize();
}