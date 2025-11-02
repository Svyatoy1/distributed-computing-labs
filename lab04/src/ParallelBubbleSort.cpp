#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <mpi.h>
using namespace std;

enum split_mode { KeepFirstHalf, KeepSecondHalf };

int ProcNum = 0; // Number of available processes
int ProcRank = -1; // Rank of current process
const double RandomDataMultiplier = 1000.0;

// Function for simple setting the initial data
void DummyDataInitialization(double*& pData, int& DataSize) {
    for(int i = 0; i < DataSize; i++)
        pData[i] = DataSize - i;
}

// Function for initializing the data by the random generator
void RandomDataInitialization(double *&pData, int& DataSize) {
    srand( (unsigned)time(0) );
    for(int i = 0; i < DataSize; i++)
    pData[i] = double(rand()) / RAND_MAX * RandomDataMultiplier;
}

// Function for allocating the memory and setting the initial values
void ProcessInitialization(double *&pData, int& DataSize, double *&pProcData, int& BlockSize) {
    setvbuf(stdout, 0, _IONBF, 0);
    if(ProcRank == 0) {
        do {
            printf("Enter the size of data to be sorted: ");
            scanf("%d", &DataSize);
            if(DataSize < ProcNum) {
                printf("Data size should be greater than number of processes\n");
            }
            if(DataSize % ProcNum != 0) {
                printf("Data size should be divisible by number of processes\n");
            }
        } while((DataSize < ProcNum) || (DataSize % ProcNum != 0));
        printf("Sorting %d data items\n", DataSize);
    }
    // Broadcasting the data size
    MPI_Bcast(&DataSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
    BlockSize = DataSize / ProcNum;

    pProcData = new double[BlockSize];
    if (ProcRank == 0) {
        pData = new double[DataSize];

    // Data initalization
    DummyDataInitialization(pData, DataSize);
    }
}

// Function for formatted data output
void PrintData(double *pData, int DataSize) {
    for(int i = 0; i < DataSize; i++)
        printf("%7.4f ", pData[i]);
    printf("\n");
}

// Data distribution among the processes
void DataDistribution(double *pData, int DataSize, double *pProcData, int BlockSize) {
    MPI_Scatter(pData, BlockSize, MPI_DOUBLE, pProcData, BlockSize, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

// Function for testing the data distribution
void TestDistribution(double *pData, int DataSize, double *pProcData, int BlockSize) {
    MPI_Barrier(MPI_COMM_WORLD);
    if (ProcRank == 0) {
        printf("Initial data:\n");
        PrintData(pData, DataSize);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    for (int i = 0; i < ProcNum; i++) {
        if (ProcRank == i) {
            printf("ProcRank = %d\n", ProcRank);
            printf("Block:\n");
            PrintData(pProcData, BlockSize);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

// Serial bubble sort algorithm
void SerialBubbleSort(double *pData, int DataSize) {
    double Tmp;
    for(int i = 1; i < DataSize; i++)
        for(int j = 0; j < DataSize - i; j++)
            if(pData[j] > pData[j + 1]) {
                Tmp = pData[j];
                pData[j] = pData[j + 1];
                pData[j + 1] = Tmp;
            }
}

// Function for parallel data output
void ParallelPrintData(double *pProcData, int BlockSize) {
    // Print the sorted data
    for(int i = 0; i < ProcNum; i++) {
        if (ProcRank == i) {
            printf("ProcRank = %d\n", ProcRank);
            printf("Proc sorted data:\n");
            PrintData(pProcData, BlockSize);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
}

// Function for data exchange between the neighboring processes
void ExchangeData(double *pProcData, int BlockSize, int DualRank, double *pDualData) {
    MPI_Status status;
    MPI_Sendrecv(pProcData, BlockSize, MPI_DOUBLE, DualRank, 0, pDualData, BlockSize, 
    MPI_DOUBLE, DualRank, 0, MPI_COMM_WORLD, &status);
}

// Parallel bubble sort algorithm
void ParallelBubble(double *pProcData, int BlockSize) {
    // Local sorting the process data
    SerialBubbleSort(pProcData, BlockSize);

    double *pDualData = new double[BlockSize];
    double *pMergedData = new double[2 * BlockSize];
    int Offset;
    split_mode SplitMode = KeepFirstHalf;

    for(int i = 0; i < 2 * ProcNum; i++) {
        if((i % 2) == 1) {
            if((ProcRank % 2) == 1) {
                Offset = 1;
                SplitMode = KeepFirstHalf;
            }
            else {
                Offset = -1;
                SplitMode = KeepSecondHalf;
            }
        }
        else {
            if((ProcRank % 2) == 1) {
                Offset = -1;
                SplitMode = KeepSecondHalf;
            }
            else {
                Offset = 1;
                SplitMode = KeepFirstHalf;
            }
        }
        // Check the first and last processes
        if((ProcRank == ProcNum - 1) && (Offset == 1)) continue;
        if((ProcRank == 0 ) && (Offset == -1)) continue;

        ExchangeData(pProcData, BlockSize, ProcRank + Offset, pDualData);
        // Data merging
        merge(pProcData, pProcData + BlockSize, pDualData, pDualData + BlockSize, pMergedData);
        // Data splitting
        if(SplitMode == KeepFirstHalf)
            copy(pMergedData, pMergedData + BlockSize, pProcData);
        else
            copy(pMergedData + BlockSize, pMergedData + 2*BlockSize, pProcData);
    }

    delete []pDualData;
    delete []pMergedData;

    // Print the sorted data
    ParallelPrintData(pProcData, BlockSize);
}

// Function for computational process termination
void ProcessTermination(double *pData, double *pProcData) {
    if(ProcRank == 0) delete []pData;
        delete []pProcData;
}

int main (int argc, char* argv[]) {
    double *pData = 0;
    double *pProcData = 0;
    int DataSize = 0;
    int BlockSize = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    if (ProcRank == 0)
        printf("Parallel bubble sort program\n");

    // Process initialization
    ProcessInitialization(pData, DataSize, pProcData, BlockSize);

    // Distributing the initial data among processes
    DataDistribution(pData, DataSize, pProcData, BlockSize);
    // Testing the data distribution
    TestDistribution(pData, DataSize, pProcData, BlockSize);

    // Parallel bubble sort
    ParallelBubble(pProcData, BlockSize);
    ParallelPrintData(pProcData, BlockSize);

    // Process termination
    ProcessTermination(pData, pProcData);

    MPI_Finalize();
    return 0;
}