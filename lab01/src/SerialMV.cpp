#include <iostream>
#include <iomanip>
#include <ctime> // для функцій clock_t, clock, CLOCKS_PER_SEC
#include <cstdlib>
#include <limits>
using namespace std;

double* pMatrix; // First argument - initial matrix
double* pVector; // Second argument - initial vector
double* pResult; // Result vector for matrix-vector multiplication
int Size; // Sizes of initial matrix and vector

// Function for simple definition of matrix and vector elements
void DummyDataInitialization(double* pMatrix, double* pVector, int Size) {
    for (int i = 0; i < Size; ++i) {
        pVector[i] = 1.0;
        for (int j = 0; j < Size; ++j)
            pMatrix[i * Size + j] = static_cast<double>(i);
    }
}

// Function for random setting of the matrix and vector elements
void RandomDataInitialization (double* pMatrix,double* pVector,int Size) {
    int i, j; // Loop variables
    
    srand(unsigned(clock()));
    for (i=0; i<Size; i++) {
        pVector[i] = rand()/double(1000);

        for (j=0; j<Size; j++)
            pMatrix[i*Size+j] = rand()/double(1000);
    }
}

// Function for matrix-vector multiplication
void ResultCalculation(double* pMatrix, double* pVector, double* pResult,int Size) {
    int i, j; // Loop variables

    for (i=0; i<Size; i++) {
        pResult[i] = 0;

        for (j=0; j<Size; j++)
            pResult[i] += pMatrix[i*Size+j]*pVector[j];
    }
}

// Function for memory allocation and data initialization
void ProcessInitialization(double* &pMatrix, double* &pVector, double* &pResult, int &Size) {
    do {
        cout << "\nEnter size of the initial objects: ";
        if (!(cin >> Size)) {
            cout << "Please enter a positive integer!\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            Size = 0;
            continue;
        }
        cout << "\nChosen objects size = " << Size << '\n';
        if (Size <= 0)
            cout << "Size of objects must be greater than 0!\n";
    } while (Size <= 0);

    //Memory allocation
    pMatrix = new double [Size*Size];
    pVector = new double [Size];
    pResult = new double [Size];

    // Random definition of matrix and vector elements
    RandomDataInitialization(pMatrix, pVector, Size);
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

// Function for computational process termination
void ProcessTermination(double* pMatrix,double* pVector,double* pResult) {
    delete [] pMatrix;
    delete [] pVector;
    delete [] pResult;
}

int main() {
    // Memory allocation and data initialization
    ProcessInitialization(pMatrix, pVector, pResult, Size);

    clock_t start, finish;
    double duration;

    // Matrix and vector output
    printf ("Initial Matrix: \n");
    PrintMatrix (pMatrix, Size, Size);
    printf ("Initial Vector: \n");
    PrintVector (pVector, Size);

    // Matrix-vector multiplication
    start = clock();
    ResultCalculation (pMatrix, pVector, pResult, Size);
    finish = clock();
    duration = (finish-start)/double(CLOCKS_PER_SEC);

    // Printing the result vector
    printf ("\nResult Vector: \n");
    PrintVector(pResult, Size);
    // Printing the time spent by matrix-vector multiplication
    printf("\nTime of execution: %f", duration);

    // Computational process termination
    ProcessTermination(pMatrix, pVector, pResult);
}