#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
using namespace std;

int ProcNum, ProcRank;

void DummyDataInitialization(double* pMatrix, double* pVector, int Size) {
    for (int i = 0; i < Size; ++i) {
        pVector[i] = 1.0;
        for (int j = 0; j < Size; ++j)
            pMatrix[i * Size + j] = static_cast<double>(i);
    }
}

void PrintMatrix(double* pMatrix, int RowCount, int ColCount) {
    for (int i = 0; i < RowCount; i++) {
        for (int j = 0; j < ColCount; j++)
            printf("%7.4f ", pMatrix[i * ColCount + j]);
        printf("\n");
    }
}

void PrintVector(double* pVector, int Size) {
    for (int i = 0; i < Size; i++)
        printf("%7.4f ", pVector[i]);
    printf("\n");
}

void ProcessInitialization(double*& pMatrix, double*& pVector, double*& pResult,
    double*& pProcRows, double*& pProcResult, int& Size, int& RowNum) {
    
    if (ProcRank == 0) {
        do {
            cout << "\nEnter size of the matrix and vector: ";
            cin >> Size;
            if (Size < ProcNum)
                cout << "Size must be greater than number of processes!\n";
            if (Size % ProcNum != 0)
                cout << "Size must be divisible by number of processes!\n";
        } while (Size < ProcNum || Size % ProcNum != 0);
    }

    MPI_Bcast(&Size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    RowNum = Size / ProcNum;

    pVector = new double[Size];
    pResult = new double[Size];
    pProcRows = new double[RowNum * Size];
    pProcResult = new double[RowNum];

    if (ProcRank == 0) {
        pMatrix = new double[Size * Size];
        DummyDataInitialization(pMatrix, pVector, Size);

        cout << "\nInitial Matrix (on root process):\n";
        PrintMatrix(pMatrix, Size, Size);

        cout << "\nInitial Vector:\n";
        PrintVector(pVector, Size);
    }
}

void DataDistribution(double* pMatrix, double* pProcRows, double* pVector,
    int Size, int RowNum) {
    
    MPI_Bcast(pVector, Size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(pMatrix, RowNum * Size, MPI_DOUBLE, pProcRows,
        RowNum * Size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

void TestDistribution(double* pMatrix, double* pVector, double* pProcRows, int Size, int RowNum) {
    if (ProcRank == 0) {
        cout << "\nInitial Matrix (on root process):\n";
        PrintMatrix(pMatrix, Size, Size);
        cout << "\nInitial Vector:\n";
        PrintVector(pVector, Size);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    for (int i = 0; i < ProcNum; i++) {
        if (ProcRank == i) {
            cout << "\nProcRank = " << ProcRank << endl;
            cout << "Matrix Stripe:\n";
            PrintMatrix(pProcRows, RowNum, Size);
            cout << "Vector:\n";
            PrintVector(pVector, Size);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

// Process rows and vector mulriplication
void ParallelResultCalculation(double* pProcRows, double* pVector, double* pProcResult, int Size, int RowNum) {
    int i, j;
    for (i=0; i<RowNum; i++) {
        pProcResult[i] = 0;
        for (j=0; j<Size; j++) {
            pProcResult[i] += pProcRows[i*Size+j]*pVector[j];
        }
    }
}

// Function for testing the multiplication result of matrix stripe and vector
void TestPartialResults(double* pProcResult, int RowNum) {
    for (int i = 0; i < ProcNum; i++) {
        if (ProcRank == i) {
            cout << "\nProcRank = " << ProcRank << endl;
            cout << "Part of result vector:" << endl;
            PrintVector(pProcResult, RowNum);
        }
        MPI_Barrier(MPI_COMM_WORLD); // Synchronize processes before next output
    }
}

// Function for result vector replication
void ResultReplication(double* pProcResult, double* pResult, int Size, int RowNum) {
    MPI_Allgather(pProcResult, RowNum, MPI_DOUBLE, pResult, RowNum, MPI_DOUBLE, MPI_COMM_WORLD);
}

void SerialResultCalculation(double* pMatrix, double* pVector, double* pResult,int Size) {
    int i, j; // Loop variables

    for (i=0; i<Size; i++) {
        pResult[i] = 0;

        for (j=0; j<Size; j++)
            pResult[i] += pMatrix[i*Size+j]*pVector[j];
    }
}

// Testing the result of parallel matrix-vector multiplication
void TestResult(double* pMatrix, double* pVector, double* pResult, int Size) {
    double* pSerialResult; // Result of serial matrix-vector multiplication
    int equal = 0; // =0, if the serial and parallel results are identical
    int i; // Loop variable

    if (ProcRank == 0) {
        pSerialResult = new double [Size];
        SerialResultCalculation(pMatrix, pVector, pSerialResult, Size);
        for (i=0; i<Size; i++) {
            if (pResult[i] != pSerialResult[i])
                equal = 1;
        }
        if (equal == 1)
            cout << "The results of serial and parallel algorithms are NOT identical. Check your code." ;
        else
            cout << "The results of serial and parallel algorithms are identical." ;
        delete [] pSerialResult;
    }
}

void ProcessTermination(double* pMatrix, double* pVector, double* pResult,
    double* pProcRows, double* pProcResult) {
    if (ProcRank == 0) {
        delete[] pMatrix;
    }
    delete[] pVector;
    delete[] pResult;
    delete[] pProcRows;
    delete[] pProcResult;
}

int main(int argc, char* argv[]) {
    double* pMatrix;
    double* pVector;
    double* pResult;
    double* pProcRows;
    double* pProcResult;
    int Size, RowNum;
    double Start, Finish, Duration;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    if (ProcRank == 0)
        cout << "Parallel matrix-vector multiplication program\n";

    ProcessInitialization(pMatrix, pVector, pResult, pProcRows, pProcResult, Size, RowNum);
    DataDistribution(pMatrix, pProcRows, pVector, Size, RowNum);
    TestDistribution(pMatrix, pVector, pProcRows, Size, RowNum);
    ParallelResultCalculation(pProcRows, pVector, pProcResult, Size, RowNum);
    ResultReplication(pProcResult, pResult, Size, RowNum);
    TestPartialResults(pProcResult, RowNum);
    TestResult(pMatrix, pVector, pResult, Size);
    ProcessTermination(pMatrix, pVector, pResult, pProcRows, pProcResult);

    MPI_Finalize();
    return 0;
}