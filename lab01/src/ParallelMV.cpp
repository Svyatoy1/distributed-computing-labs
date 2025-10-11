#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
#include <cmath>
using namespace std;

int ProcNum, ProcRank;

void DummyDataInitialization(double* pMatrix, double* pVector, int Rows, int Cols) {
    for (int i = 0; i < Rows; ++i) {
    for (int j = 0; j < Cols; ++j)
        pMatrix[i * Cols + j] = static_cast<double>(i + 1);
    }

    for (int j = 0; j < Cols; ++j)
        pVector[j] = 1.0;
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
    double*& pProcRows, double*& pProcResult, int& Rows, int& Cols, int& RowNum) {
    if (ProcRank == 0) {
        do {
            cout << "\nEnter number of rows: ";
            cin >> Rows;
            cout << "Enter number of columns: ";
            cin >> Cols;
            if (Rows < ProcNum)
                cout << "Rows must be greater than number of processes!\n";
        } while (Rows < ProcNum);
    }

    MPI_Bcast(&Rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&Cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

    RowNum = Rows / ProcNum;

    pVector = new double[Cols];
    pResult = new double[Rows];
    pProcRows = new double[RowNum * Cols];
    pProcResult = new double[RowNum];

    if (ProcRank == 0) {
        pMatrix = new double[Rows * Cols];
        DummyDataInitialization(pMatrix, pVector, Rows, Cols);
        //cout << "\nMatrix size: " << Rows << "x" << Cols << endl;
        //cout << "\nInitial Matrix (on root process):\n";
        //PrintMatrix(pMatrix, Rows, Cols);
        //cout << "\nInitial Vector:\n";
        //PrintVector(pVector, Cols);
    }
}

void DataDistribution(double* pMatrix, double* pProcRows, double* pVector, int Rows, int Cols, int RowNum) {
    MPI_Bcast(pVector, Cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(pMatrix, RowNum*Cols, MPI_DOUBLE, pProcRows, RowNum*Cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);
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
void ParallelResultCalculation(double* pProcRows, double* pVector, double* pProcResult, int Rows, int Cols, int RowNum) {
    for (int i = 0; i < RowNum; i++) {
        pProcResult[i] = 0.0;
        for (int j = 0; j < Cols; j++)
            pProcResult[i] += pProcRows[i * Cols + j] * pVector[j];
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
void ResultReplication(double* pProcResult, double* pResult, int Rows, int RowNum) {
    MPI_Gather(pProcResult, RowNum, MPI_DOUBLE, pResult, RowNum, MPI_DOUBLE, 0, MPI_COMM_WORLD);
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
void TestResult(double* pMatrix, double* pVector, double* pResult, int Rows, int Cols) {
    double* pSerialResult;    // Result of serial matrix-vector multiplication
    int equal = 0;            // =0 if the serial and parallel results are identical
    int i;                    // Loop variable

    if (ProcRank == 0) {
        pSerialResult = new double[Rows]; // one value per row of the matrix

        // Perform serial matrix-vector multiplication
        for (i = 0; i < Rows; i++) {
            pSerialResult[i] = 0.0;
            for (int j = 0; j < Cols; j++)
                pSerialResult[i] += pMatrix[i * Cols + j] * pVector[j];
        }

        // Compare serial and parallel results
        for (i = 0; i < Rows; i++) {
            if (fabs(pResult[i] - pSerialResult[i]) > 1e-6) { // allow floating-point tolerance
                equal = 1;
                break;
            }
        }

        if (equal == 1)
            cout << "The results of serial and parallel algorithms are NOT identical. Check your code." << endl;
        else
            cout << "The results of serial and parallel algorithms are identical." << endl;

        delete[] pSerialResult;
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
    int Rows, Cols, RowNum;
    double Start, Finish, Duration;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    if (ProcRank == 0)
        cout << "Parallel matrix-vector multiplication program (rectangular)\n";

    ProcessInitialization(pMatrix, pVector, pResult, pProcRows, pProcResult, Rows, Cols, RowNum);
    MPI_Barrier(MPI_COMM_WORLD);
    Start = MPI_Wtime();
    DataDistribution(pMatrix, pProcRows, pVector, Rows, Cols, RowNum);
    ParallelResultCalculation(pProcRows, pVector, pProcResult, Rows, Cols, RowNum);
    ResultReplication(pProcResult, pResult, Rows, RowNum);
    MPI_Barrier(MPI_COMM_WORLD);
    Finish = MPI_Wtime();
    Duration = Finish - Start;

    //TestResult(pMatrix, pVector, pResult, Rows, Cols);
    if (ProcRank == 0) {
        cout << "Time of execution" << Duration << endl;
    }

    ProcessTermination(pMatrix, pVector, pResult, pProcRows, pProcResult);
    MPI_Finalize();
    return 0;
}