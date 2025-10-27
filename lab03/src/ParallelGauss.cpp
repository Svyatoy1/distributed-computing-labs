#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
#include <cmath>
using namespace std;

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define GRAY    "\033[90m"
#define BOLD    "\033[1m"

int ProcNum = 0; // Number of the available processes
int ProcRank = 0; // Rank of the current process

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

void PrintMatrix(double* pMatrix, int RowCount, int ColCount) {
    for (int i=0; i<RowCount; i++) {
        for (int j=0; j<ColCount; j++) {
            double val = pMatrix[i*ColCount+j];
            if (i == j)
                printf(CYAN "%7.4f " RESET, val);               // diagonal
            else if (fabs(val) < 1e-9)
                printf(GRAY "%7.4f " RESET, val);               // near zero
            else if (val < 0)
                printf(RED "%7.4f " RESET, val);                // negative
            else
                printf("%7.4f ", val);                          // normal
        }
        printf("\n");
    }
}
    
// print vector
void PrintVector(double* pVector, int Size) {
    for (int i=0; i<Size; i++) {
        if (fabs(pVector[i]) < 1e-9)
            printf(GRAY "%7.4f " RESET, pVector[i]);
        else if (pVector[i] < 0)
            printf(RED "%7.4f " RESET, pVector[i]);
        else
            printf("%7.4f ", pVector[i]);
    }
    printf("\n");
}

// Function for memory allocation and data initialization
void ProcessInitialization (double* &pMatrix, double* &pVector, double* &pResult, double* &pProcRows, 
double* &pProcVector, double* &pProcResult, int &Size, int &RowNum) {
    if (ProcRank == 0) {
        do {
            printf(YELLOW "\nEnter the size of the matrix and the vector: " RESET);
            scanf("%d", &Size);
            if (Size < ProcNum) {
                printf (RED "Size must be greater than number of processes! \n" RESET);
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
        RandomDataInitialization (pMatrix, pVector, Size);
    }
}

// Function for the data distribution among the processes
void DataDistribution(double* pMatrix, double* pProcRows,
                      double* pVector, double* pProcVector,
                      int Size, int RowNum)
{
    int *pSendNum; // Number of elements sent to each process (for matrix)
    int *pSendInd; // Starting index of data sent to each process (for matrix)
    int *pSendNumRows; // Number of rows sent to each process (for vector)
    int *pSendIndRows; // Starting index of data sent to each process (for vector)
    int RestRows = Size; // Number of rows remaining to distribute
    int i; // Loop variable

    // Alloc memory for temporary arrays
    pSendInd = new int [ProcNum];
    pSendNum = new int [ProcNum];
    pSendIndRows = new int [ProcNum];
    pSendNumRows = new int [ProcNum];

    // Define distribution of matrix rows among processes
    RowNum = Size / ProcNum;
    pSendNum[0] = RowNum * Size;
    pSendInd[0] = 0;
    pSendNumRows[0] = RowNum;
    pSendIndRows[0] = 0;

    for (i = 1; i < ProcNum; i++) {
        RestRows -= RowNum;
        RowNum = RestRows / (ProcNum - i);
        pSendNum[i] = RowNum * Size;
        pSendInd[i] = pSendInd[i - 1] + pSendNum[i - 1];
        pSendNumRows[i] = RowNum;
        pSendIndRows[i] = pSendIndRows[i - 1] + pSendNumRows[i - 1];
    }

    // Scatter rows of matrix A
    MPI_Scatterv(pMatrix, pSendNum, pSendInd, MPI_DOUBLE,
                 pProcRows, pSendNum[ProcRank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Scatter corresponding parts of vector B
    MPI_Scatterv(pVector, pSendNumRows, pSendIndRows, MPI_DOUBLE,
                 pProcVector, pSendNumRows[ProcRank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Free temporary arrays
    delete [] pSendNum;
    delete [] pSendInd;
    delete [] pSendNumRows;
    delete [] pSendIndRows;
}

// Function for testing the data distribution
void TestDistribution(double* pMatrix, double* pVector, double* pProcRows, double* pProcVector, int Size, int RowNum) {
    if (ProcRank == 0) {
        printf("Initial Matrix: \n");
        //PrintMatrix(pMatrix, Size, Size);
        printf("Initial Vector: \n");
        //PrintVector(pVector, Size);
    }

    for (int i=0; i<ProcNum; i++) {
        if (ProcRank == i) {
            printf("\nProcRank = %d \n", ProcRank);
            printf(" Matrix Stripe:\n");
            //PrintMatrix(pProcRows, RowNum, Size);
            printf(" Vector: \n");
            //PrintVector(pProcVector, RowNum);
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
void ParallelGaussianElimination(double* pProcRows, double* pProcVector, int Size, int RowNum) {
    double MaxValue = 0.0;
    int PivotPos = -1;
    double LocalTime = 0.0; 
    double IterStart, IterEnd;

    struct { double MaxValue; int ProcRank; } ProcPivot, Pivot;
    double *pPivotRow = new double [Size + 1];

    for (int i = 0; i < Size; i++) {
        MaxValue = 0.0;
        PivotPos = -1;
        IterStart = MPI_Wtime();

        // iteration start
        if (ProcRank == 0)
            printf(YELLOW BOLD "\n Iteration %d started\n" RESET, i);

        // local search of pivot
        for (int j = 0; j < RowNum; j++) {
            if ((pProcPivotIter[j] == -1) && (fabs(pProcRows[j*Size+i]) > MaxValue)) {
                MaxValue = fabs(pProcRows[j*Size+i]);
                PivotPos = j;
            }
        }

        if (PivotPos != -1) {
            printf(CYAN "Proc %d found local pivot %.4f at local row %d\n" RESET,
                   ProcRank, MaxValue, PivotPos);
        }

        ProcPivot.MaxValue = MaxValue;
        ProcPivot.ProcRank = (PivotPos == -1) ? -1 : ProcRank;

        MPI_Allreduce(&ProcPivot, &Pivot, 1, MPI_DOUBLE_INT, MPI_MAXLOC, MPI_COMM_WORLD);

        // глобальний півот
        if (ProcRank == 0)
            printf(GREEN " Global pivot: process %d (value %.4f)\n" RESET, Pivot.ProcRank, Pivot.MaxValue);

        // saving pivot number
        if (ProcRank == Pivot.ProcRank) {
            pProcPivotIter[PivotPos] = i;
            pParallelPivotPos[i] = pProcInd[ProcRank] + PivotPos;
        }
        MPI_Bcast(&pParallelPivotPos[i], 1, MPI_INT, Pivot.ProcRank, MPI_COMM_WORLD);

        // pivot row broadcast
        if (ProcRank == Pivot.ProcRank) {
            for (int j = 0; j < Size; j++)
                pPivotRow[j] = pProcRows[PivotPos * Size + j];
            pPivotRow[Size] = pProcVector[PivotPos];
        }
        MPI_Bcast(pPivotRow, Size + 1, MPI_DOUBLE, Pivot.ProcRank, MPI_COMM_WORLD);

        // elimination
        for (int r = 0; r < RowNum; r++) {
            if (pProcPivotIter[r] == -1) {
                double factor = pProcRows[r*Size+i] / pPivotRow[i];
                for (int c = i; c < Size; c++)
                    pProcRows[r*Size+c] -= factor * pPivotRow[c];
                pProcVector[r] -= factor * pPivotRow[Size];
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        IterEnd = MPI_Wtime();
        LocalTime += (IterEnd - IterStart);

        if (ProcRank == 0)
            printf(GRAY "Iteration %d completed (%.6f s)\n" RESET, i, IterEnd - IterStart);
    }

    // final output
    double TotalTime;
    MPI_Reduce(&LocalTime, &TotalTime, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    printf(BOLD "\nProcess %d summary\n" RESET, ProcRank);
    printf(" Rows processed: %d\n", RowNum);
    printf(" Local time: %.6f s\n", LocalTime);

    MPI_Barrier(MPI_COMM_WORLD);
    if (ProcRank == 0) {
        printf(BOLD CYAN "\nGlobal Summary\n" RESET);
        printf(" Total time across all processes: %.6f s\n", TotalTime);
        printf(" Average per process: %.6f s\n", TotalTime / ProcNum);
        printf(" Processes used: %d\n", ProcNum);
        printf(BOLD GREEN "---------------------------\n\n" RESET);
    }

    delete [] pPivotRow;
}

// Function for finding the pivot row of the back substitution
void FindBackPivotRow(int RowIndex, int &IterProcRank, int &IterPivotPos) {
    for (int i=0; i<ProcNum-1; i++) {
        if ((pProcInd[i]<=RowIndex) && (RowIndex<pProcInd[i+1]))
            IterProcRank = i;
    }
    if (RowIndex >= pProcInd[ProcNum-1])
        IterProcRank = ProcNum-1;
    IterPivotPos = RowIndex - pProcInd[IterProcRank];
}

// Function for the back substitution
void ParallelBackSubstitution (double* pProcRows, double* pProcVector, double* pProcResult, int Size, int RowNum) {
    int IterProcRank; // Rank of the process with the current pivot row
    int IterPivotPos; // Position of the pivot row of the process
    double IterResult; // Calculated value of the current unknown
    double val;
    
    // The iterations of the back substitution
    for (int i=Size-1; i>=0; i--) {
        // Calculating the rank of the process, which holds the pivot row
        FindBackPivotRow(pParallelPivotPos[i],IterProcRank,IterPivotPos);

    // Calculating the unknown
    if (ProcRank == IterProcRank) {
        IterResult = pProcVector[IterPivotPos] / pProcRows[IterPivotPos*Size+i];
        pProcResult[IterPivotPos] = IterResult;
    }

    // Broadcasting the value of the current unknown
    MPI_Bcast(&IterResult, 1, MPI_DOUBLE, IterProcRank, MPI_COMM_WORLD);

    // Updating the values of the vector
    for (int j=0; j<RowNum; j++)
        if ( pProcPivotIter[j] < i ) {
            val = pProcRows[j*Size + i] * IterResult;
            pProcVector[j]=pProcVector[j] - val;
        }
    }
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
    ParallelBackSubstitution (pProcRows, pProcVector, pProcResult, Size, RowNum);
}

// Function for gathering the result vector
void ResultCollection(double* pProcResult, double* pResult) {
    //Gathering the result vector on the pivot processor
    MPI_Gatherv(pProcResult, pProcNum[ProcRank], MPI_DOUBLE, pResult, pProcNum, pProcInd, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

// Function for formatted result vector output
void PrintResultVector (double* pResult, int Size) {
    int i;
    for (i=0; i<Size; i++)
        printf("%7.4f ", pResult[pParallelPivotPos[i]]);
}

// Function for testing the result
void TestResult(double* pMatrix, double* pVector, double* pResult, int Size) {
    /* Buffer for storing the vector, that is a result of multiplication
    of the linear system matrix by the vector of unknowns */
    double* pRightPartVector;
    // Flag, that shows wheather the right parts vectors are identical or not
    int equal = 0;
    double Accuracy = 1.e-6; // Comparison accuracy
    if (ProcRank == 0) {
        pRightPartVector = new double [Size];
        for (int i=0; i<Size; i++) {
            pRightPartVector[i] = 0;
            for (int j=0; j<Size; j++) {
                pRightPartVector[i] += pMatrix[i*Size+j]*pResult[pParallelPivotPos[j]];
            }
        }
        for (int i=0; i<Size; i++) {
            if (fabs(pRightPartVector[i]-pVector[i]) > Accuracy)
                equal = 1;
        }
        if (equal == 1)
            printf(RED BOLD "The result of the parallel Gauss algorithm is NOT correct. Check your code." RESET);
        else
            printf(GREEN BOLD "The result of the parallel Gauss algorithm is correct." RESET);
        delete [] pRightPartVector;
    }
}

// Function for computational process termination
void ProcessTermination (double* pMatrix, double* pVector, double* pResult,
double* pProcRows, double* pProcVector, double* pProcResult) {
    if (ProcRank == 0) {
        delete [] pMatrix;
        delete [] pVector;
        delete [] pResult;
    }

    if (pProcRows) delete [] pProcRows;
    if (pProcVector) delete [] pProcVector;
    if (pProcResult) delete [] pProcResult;
    delete[] pProcInd;
    delete[] pProcNum;
    delete [] pParallelPivotPos;
    delete [] pProcPivotIter;
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
        printf(GREEN BOLD "Parallel Gauss algorithm for solving linear systems\n" RESET);

    // Memory allocation and data initialization
    ProcessInitialization(pMatrix, pVector, pResult, pProcRows, pProcVector, pProcResult, Size, RowNum);

    pProcInd = new int[ProcNum];
    pProcNum = new int[ProcNum];
    int RestRows = Size;
    for (int i = 0; i < ProcNum; i++) {
        pProcNum[i] = RestRows / (ProcNum - i);
        RestRows -= pProcNum[i];
    }
    pProcInd[0] = 0;
    for (int i = 1; i < ProcNum; i++)
        pProcInd[i] = pProcInd[i - 1] + pProcNum[i - 1];

    Start = MPI_Wtime();
    // Distributing the initial data between the processes
    DataDistribution(pMatrix, pProcRows, pVector, pProcVector, Size, RowNum);

    // The execution of the parallel Gauss algorithm
    ParallelResultCalculation (pProcRows, pProcVector, pProcResult, Size, RowNum);

    // Gathering the result vector
    ResultCollection(pProcResult, pResult);

    Finish = MPI_Wtime();
    Duration = Finish-Start;

    // Testing the result
    TestResult(pMatrix, pVector, pResult, Size);

    // Printing the time spent by parallel Gauss algorithm
    if (ProcRank == 0)
        printf("\n Time of execution: %f\n", Duration);

    // Distributing the initial data between the processes
    //TestDistribution(pMatrix, pProcRows, pVector, pProcVector, Size, RowNum);

    // Process termination
    ProcessTermination (pMatrix, pVector, pResult, pProcRows, pProcVector, pProcResult);
    MPI_Finalize();
}