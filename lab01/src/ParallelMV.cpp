#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
using namespace std;

int ProcNum, ProcRank; // Кількість процесів і ранг поточного процесу

// --- 1. Ініціалізація тестових даних ---
void DummyDataInitialization(double* pMatrix, double* pVector, int Size) {
    for (int i = 0; i < Size; ++i) {
        pVector[i] = 1.0;
        for (int j = 0; j < Size; ++j)
            pMatrix[i * Size + j] = static_cast<double>(i);
    }
}

// --- 2. Форматований вивід матриці ---
void PrintMatrix(double* pMatrix, int RowCount, int ColCount) {
    for (int i = 0; i < RowCount; i++) {
        for (int j = 0; j < ColCount; j++)
            printf("%7.4f ", pMatrix[i * ColCount + j]);
        printf("\n");
    }
}

// --- 3. Форматований вивід вектора ---
void PrintVector(double* pVector, int Size) {
    for (int i = 0; i < Size; i++)
        printf("%7.4f ", pVector[i]);
    printf("\n");
}

// --- 4. Ініціалізація даних ---
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

// --- 5. Розподіл даних між процесами ---
void DataDistribution(double* pMatrix, double* pProcRows, double* pVector,
    int Size, int RowNum) {
    
    MPI_Bcast(pVector, Size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(pMatrix, RowNum * Size, MPI_DOUBLE, pProcRows,
        RowNum * Size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

// --- 6. Тестування розподілу даних ---
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

// --- 7. Завершення процесу ---
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

// --- 8. Основна функція ---
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
    ProcessTermination(pMatrix, pVector, pResult, pProcRows, pProcResult);

    MPI_Finalize();
    return 0;
}