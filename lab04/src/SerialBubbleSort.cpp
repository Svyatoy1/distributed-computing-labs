#include <iostream>
#include <algorithm>
#include <ctime> // для функцій clock_t, clock, CLOCKS_PER_SEC
#include <cstdlib>
#include <limits>
#include <cmath>
using namespace std;

double *pData; // Data to be sorted
int DataSize; // Size of data to be sorted
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

// Function for sorting by the standard library algorithm
void SerialStdSort(double *pData, int DataSize) {
    sort(pData, pData + DataSize);
}

// Function for the serial bubble sort algorithm
void SerialBubble(double *pData, int DataSize) {
    double Tmp;
    for(int i = 1; i < DataSize; i++)
        for(int j = 0; j < DataSize - i; j++)
            if(pData[j] > pData[j + 1]) {
                Tmp = pData[j];
                pData[j] = pData[j + 1];
                pData[j + 1] = Tmp;
        }
}

// Function for formatted data output
void PrintData(double *pData, int DataSize) {
    for(int i = 0; i < DataSize; i++)
        printf("%7.4f ", pData[i]);
    printf("\n");
}

// Function for allocating the memory and setting the initial values
void ProcessInitialization(double *&pData, int& DataSize) {
    do {
        printf("Enter the size of data to be sorted: ");
        scanf("%d", &DataSize);
        if(DataSize <= 0)
            printf("Data size should be greater than zero\n");
    } while(DataSize <= 0);
    printf("Sorting %d data items\n", DataSize);

    pData = new double[DataSize];
    //DummyDataInitialization(pData, DataSize);
    RandomDataInitialization(pData, DataSize);
}

// Function for computational process termination
void ProcessTermination(double *pData) {
    delete []pData;
}

int main() {
    double *pData = 0;
    int DataSize = 0;
    time_t start, finish;
    double duration = 0.0;

    printf("Serial bubble sort program\n");

    // Process initialization
    ProcessInitialization(pData, DataSize);

    printf("Data before sorting\n");
    PrintData(pData, DataSize);

    // Serial bubble sort
    start = clock();
    //SerialBubble(pData, DataSize);
    SerialStdSort(pData, DataSize);
    finish = clock();

    printf("Data after sorting\n");
    PrintData(pData, DataSize);

    duration = (finish - start) / double(CLOCKS_PER_SEC);
    printf("Time of execution: %f\n", duration);

    // Process termination
    ProcessTermination(pData);
    return 0;
}