#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <mpi.h>
#include <iostream>
using namespace std;

static int ProcNum = 0; // Number of available processes
static int ProcRank = -1; // Rank of current process

// Function for distribution of the grid rows among the processes
void DataDistribution(double* pMatrix, double* pProcRows, int Size, int RowNum) {
    int *pSendNum = new int[ProcNum];
    int *pSendInd = new int[ProcNum];

    // обчислення sendcounts та offsets
    int baseRows = (Size - 2) / ProcNum;
    int extra = (Size - 2) % ProcNum;
    int start = 0;

    for (int i = 0; i < ProcNum; i++) {
        int rows = baseRows + 2;               // усі процеси мають дві граничні рядки
        if (i == 0) rows--;                    // перший не має верхньої межі
        if (i == ProcNum - 1) rows--;          // останній не має нижньої
        pSendNum[i] = rows * Size;
        pSendInd[i] = start * Size;
        start += (rows - 2);
    }

    // кількість елементів, які отримує поточний процес
    int recvCount = pSendNum[ProcRank];

    MPI_Scatterv(pMatrix, pSendNum, pSendInd, MPI_DOUBLE,
                 pProcRows, recvCount, MPI_DOUBLE,
                 0, MPI_COMM_WORLD);

    delete [] pSendNum;
    delete [] pSendInd;
}

// Function for computational process termination
void ProcessTermination(double* pMatrix, double* pProcRows) {
    if (ProcRank == 0 && pMatrix != nullptr) {
        delete [] pMatrix;
    }
    if (pProcRows != nullptr) {
        delete [] pProcRows;
    }
}

// Function for formatted matrix output
void PrintMatrix(double *pMatrix, int RowCount, int ColCount){
    int i,j; // Loop variables
    for(int i=0; i < RowCount; i++) {
    for(j=0; j < ColCount; j++)
    printf("%7.4f ", pMatrix[i*ColCount+j]);
    printf("\n");
    }
}

// Function for the execution of the Gauss-Seidel method iteration
double IterationCalculation(double* pProcRows, int Size, int RowNum) {
    int i, j; // Loop variables
    double dm, dmax,temp;
    dmax = 0;
    for (i = 1; i < RowNum-1; i++)
    for(j = 1; j < Size-1; j++) {
    temp = pProcRows[Size * i + j];
    pProcRows[Size * i + j] = 0.25 * (pProcRows[Size * i + j + 1] +
    pProcRows[Size * i + j - 1] +
    pProcRows[Size * (i + 1) + j] +
    pProcRows[Size * (i - 1) + j]);
    dm = fabs(pProcRows[Size * i + j] - temp);
    if (dmax < dm) dmax = dm;
    }
    return dmax;
}

// Function for testing the data distribution
void TestDistribution(double* pMatrix, double* pProcRows, int Size, int RowNum) {
    int i;
    if (ProcRank == 0) {
    printf("Initial Matrix: \n");
    //PrintMatrix(pMatrix, Size, Size);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    for (i=0; i<ProcNum; i++) {
    if (ProcRank == i) {
    printf("\nProcRank = %d \n", ProcRank);
    // fprintf(" Matrix Stripe:\n");
    //PrintMatrix(pProcRows, RowNum, Size);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    }
}
// Function for simple setting the grid node values
void DummyDataInitialization (double* pMatrix, int Size) {
    int i, j; // Loop variables
    double h = 1.0 / (Size - 1);
    // Setting the grid node values
    for (i=0; i<Size; i++) {
    for (j=0; j<Size; j++)
    if ((i==0) || (i== Size-1) || (j==0) || (j==Size-1))
    pMatrix[i*Size+j] = 100;
    else
    pMatrix[i*Size+j] = 0;
    }
}

// Function for memory allocation and initialization of grid nodes
void ProcessInitialization (double* &pMatrix, double* &pProcRows,int &Size, int &RowNum, double &Eps) {
    int RestRows; // Number of rows, that haven’t been distributed yet
    // Setting the grid size
    if (ProcRank == 0) {
    do {
    printf("\nEnter the grid size: ");
    scanf("%d", &Size);
    if (Size <= 2) {
    printf("\n Size of grid must be greater than 2! \n");
    }
    if (Size < ProcNum) {
    printf("Size of grid must be greater than"
    "the number of processes! \n ");
    }
    }
    while ( (Size <= 2) || (Size < ProcNum));
    // Setting the required accuracy
    do {
    printf("\nEnter the required accuracy: ");
    scanf("%lf", &Eps);
    printf("\nChosen accuracy = %lf", Eps);
    if (Eps <= 0)
    printf("\nAccuracy must be greater than 0!\n");
    }
    while (Eps <= 0);
    }
    MPI_Bcast(&Size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&Eps, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    // Define the number of matrix rows stored on each process
    RestRows = Size;
    for (int i=0; i<ProcRank; i++)
    RestRows = RestRows-RestRows/(ProcNum-i);
    RowNum = (RestRows-2)/(ProcNum-ProcRank)+2;
    // Memory allocation
    pProcRows = new double [RowNum*Size];
    // Define the values of initial objects’ elements
    if (ProcRank == 0) {
    // Initial matrix exists only on the pivot process
    pMatrix = new double [Size*Size];
    // Values of elements are defined only on the pivot process
    DummyDataInitialization(pMatrix, Size);
    }
}

// Function for exchanging the boundary rows of the process stripes
void ExchangeData(double* pProcRows, int Size, int RowNum) {
    MPI_Status status;
    int NextProcNum = (ProcRank == ProcNum-1)? MPI_PROC_NULL : ProcRank + 1;
    int PrevProcNum = (ProcRank == 0)? MPI_PROC_NULL : ProcRank - 1;
    // Send to NextProcNum and receive from PrevProcNum
    MPI_Sendrecv(pProcRows + Size * (RowNum - 2),Size, MPI_DOUBLE,
    NextProcNum, 4, pProcRows, Size, MPI_DOUBLE, PrevProcNum, 4,
    MPI_COMM_WORLD, &status);
    // Send to PrevProcNum and receive from NextProcNum
    MPI_Sendrecv(pProcRows + Size, Size, MPI_DOUBLE, PrevProcNum, 5,
    pProcRows + (RowNum - 1) * Size, Size,MPI_DOUBLE, NextProcNum, 5,
    MPI_COMM_WORLD, &status);
}

// Function for the parallel Gauss - Seidel method
void ParallelResultCalculation (double *pProcRows, int Size, int RowNum, double Eps, int &Iterations) {
    double ProcDelta,Delta;
    Iterations=0;
    do {
    Iterations++;
    // Exchanging the boundary rows of the process stripe
    ExchangeData(pProcRows, Size,RowNum);
    // The Gauss-Seidel method iteration
    ProcDelta = IterationCalculation(pProcRows, Size, RowNum);
    // Calculating the maximum value of the deviation
    MPI_Allreduce(&ProcDelta, &Delta, 1,MPI_DOUBLE, MPI_MAX,
    MPI_COMM_WORLD);
    } while ( Delta > Eps);
}

// Function for gathering the result vector
void ResultCollection(double* &pMatrix, double* &pProcRows, int &Size, int &RowNum) {
    int *pReceiveNum = new int[ProcNum];
    int *pReceiveInd = new int[ProcNum];

    int baseRows = (Size - 2) / ProcNum;
    int extra = (Size - 2) % ProcNum;
    int start = 0;

    for (int i = 0; i < ProcNum; i++) {
        int rows = baseRows + 2;
        if (i == 0) rows--;
        if (i == ProcNum - 1) rows--;
        pReceiveNum[i] = rows * Size;
        pReceiveInd[i] = start * Size;
        start += (rows - 2);
    }

    if (!pMatrix)
        pMatrix = new double[Size * Size];

    MPI_Allgatherv(pProcRows, pReceiveNum[ProcRank], MPI_DOUBLE,
                   pMatrix, pReceiveNum, pReceiveInd, MPI_DOUBLE,
                   MPI_COMM_WORLD);

    delete [] pReceiveNum;
    delete [] pReceiveInd;
}

// Function for the serial Gauss – Seidel method
void SerialResultCalculation(double *pMatrixCopy, int Size, double Eps, int &Iter){
    int i, j; // Loop variables
    double dm, dmax,temp;
    Iter = 0;
    do {
    dmax = 0;
    for (i = 1; i < Size - 1; i++)
    for(j = 1; j < Size - 1; j++) {
    temp = pMatrixCopy[Size * i + j];
    pMatrixCopy[Size * i + j] = 0.25 * (pMatrixCopy[Size * i + j + 1] +
    pMatrixCopy[Size * i + j - 1] +
    pMatrixCopy[Size * (i + 1) + j] +
    pMatrixCopy[Size * (i - 1) + j]);
    dm = fabs(pMatrixCopy[Size * i + j] - temp);
    if (dmax < dm) dmax = dm;
    }
    Iter++;
    }
    while (dmax > Eps);
}

// Function to copy the initial data
void CopyData(double *pMatrix, int Size, double *pSerialMatrix) {
    copy(pMatrix, pMatrix + Size, pSerialMatrix);
}

// Function for testing the computation result
void TestResult(double* pMatrix, double* pSerialMatrix, int Size, double Eps) {
    int equal = 0; // =1, if the matrices are not equal
    int Iter;
    if (ProcRank == 0) {
    SerialResultCalculation(pSerialMatrix, Size, Eps, Iter);
    for (int i=0; i<Size*Size; i++) {
    if (fabs(pSerialMatrix[i]-pMatrix[i]) >= Eps)
    equal = 1;break;
    }
    if (equal == 1)
    printf("The results of serial and parallel algorithms"
    "are NOT identical. Check your code.");
    else
    printf("The results of serial and parallel algorithms"
    "are identical.");
    }
}

// Function for setting the grid node values by a random generator
void RandowmDataInitialization (double* pMatrix, int Size) {
    int i, j; // Loop variables
    srand(unsigned(clock()));
    // Setting the grid node values
    for (i=0; i<Size; i++) {
    for (j=0; j<Size; j++)
    if ((i==0) || (i== Size-1) || (j==0) || (j==Size-1))
    pMatrix[i*Size+j] = 100;
    else
    pMatrix[i*Size+j] = rand()/double(1000);
    }
}

int main(int argc, char* argv[]) {
    double* pMatrix = nullptr;
    double* pProcRows = nullptr;
    double* pSerialMatrix = nullptr;
    int Size; // Matrix size
    int RowNum; // Number of rows in matrix stripe
    double Eps; // Required accuracy
    int Iterations; // Iteration number
    double currDelta, delta;
    double Start, Finish;
    double Duration = 0.0;
    setvbuf(stdout, 0, _IONBF, 0);
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
    if(ProcRank == 0) {
    printf("Parallel Gauss - Seidel algorithm \n");
    fflush(stdout);
    }
    // Process initialization
    ProcessInitialization (pMatrix,pProcRows,Size,RowNum,Eps);
    // Creating the copy of the initial data
    if (ProcRank == 0) {
    pSerialMatrix = new double[Size*Size];
    CopyData(pMatrix, Size, pSerialMatrix);
    }
    Start = MPI_Wtime();
    // Data distribution among the processes
    DataDistribution(pMatrix, pProcRows, Size,RowNum);
    // Paralle Gauss-Seidel method
    ParallelResultCalculation(pProcRows, Size,RowNum,Eps, Iterations);
    //TestDistribution(pMatrix, pProcRows, Size,RowNum);
    // Gathering the calculation results
    ResultCollection(pMatrix, pProcRows, Size, RowNum);
    Finish = MPI_Wtime();
    Duration = Finish-Start;
    printf("Proc %d: finished ResultCollection, pMatrix=%p\n", ProcRank, (void*)pMatrix);
    TestDistribution(pMatrix, pProcRows, Size,RowNum);
    // Printing the result
    printf("\n Iter %d \n", Iterations);
    printf("\nResult matrix: \n");
    if (ProcRank==0) {
    //TestResult(pMatrix,Size,pMatrixCopy,Eps);
    //PrintMatrix(pMatrix,Size,Size);
    }
    // Process termination
    if (ProcRank == 0) delete []pSerialMatrix;
    if(ProcRank == 0)
        printf("Time of execution: %f\n", Duration);
    ProcessTermination(pMatrix, pProcRows);
    MPI_Barrier(MPI_COMM_WORLD);
    printf("Proc %d reached finalize\n", ProcRank);
    fflush(stdout);
    MPI_Finalize();
}