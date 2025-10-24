#include <iostream>
#include <iomanip>
#include <ctime> // для функцій clock_t, clock, CLOCKS_PER_SEC
#include <cstdlib>
#include <limits>
#include <cmath>
using namespace std;

int* pSerialPivotPos; // Number of pivot rows selected at the iterations
int* pSerialPivotIter; // Iterations, at which the rows were pivots

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
    printf ("The matrix of the linear system after the elimination: \n");
    PrintMatrix(pMatrix, Size, Size);
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
void ProcessInitialization (double* &pMatrix, double* &pVector, double* &pResult, int &Size) {
    // Setting the size of the matrix and the vector
    do {
        printf("\nEnter the size of the matrix and the vector: ");
        scanf("%d", &Size);
        printf("\nChosen size = %d", Size);
        if (Size <= 0)
            printf("\nSize of objects must be greater than 0!\n");
    } while (Size <= 0);

    // Memory allocation
    pMatrix = new double [Size*Size];
    pVector = new double [Size];
    pResult = new double [Size];

    // Initialization of the matrix and the vector elements
    DummyDataInitialization(pMatrix, pVector, Size);
}

// Function for computational process termination
void ProcessTermination (double* pMatrix,double* pVector,double* pResult) {
    delete [] pMatrix;
    delete [] pVector;
    delete [] pResult;
}

int main () {
    double* pMatrix; // Matrix of the linear system
    double* pVector; // Right parts of the linear system
    double* pResult; // Result vector
    int Size; // Size of the matrix and the vector

    time_t start, finish;
    double duration;

    printf("Serial Gauss algorithm for solving linear systems\n");
    ProcessInitialization(pMatrix, pVector, pResult, Size);

    // Matrix and vector output
    printf ("\nInitial Matrix \n");
    PrintMatrix(pMatrix, Size, Size);
    printf("Initial Vector \n");
    PrintVector(pVector, Size);

    // Execution of Gauss algorithm
    SerialResultCalculation(pMatrix, pVector, pResult, Size);

    // Printing the result vector
    printf ("\n Result Vector: \n");
    PrintVector(pResult, Size);

    // Process termination
    ProcessTermination(pMatrix, pVector, pResult);

    return 0;
}