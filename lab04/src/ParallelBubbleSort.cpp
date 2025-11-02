#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <mpi.h>

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

    // Process termination
    ProcessTermination(pData, pProcData);

    MPI_Finalize();
    return 0;
}