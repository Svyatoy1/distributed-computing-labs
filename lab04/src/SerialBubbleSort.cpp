#include <iostream>
#include <iomanip>
#include <ctime> // для функцій clock_t, clock, CLOCKS_PER_SEC
#include <cstdlib>
#include <limits>
#include <cmath>
using namespace std;

double *pData; // Data to be sorted
int DataSize; // Size of data to be sorted

// Function for simple setting the initial data
void DummyDataInitialization(double*& pData, int& DataSize) {
    for(int i = 0; i < DataSize; i++)
        pData[i] = DataSize - i;
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
    DummyDataInitialization(pData, DataSize);
}

int main() {
    printf("Serial bubble sort program\n");
    // Process initialization
    ProcessInitialization(pData, DataSize);

    printf("Data before sorting\n");
    PrintData(pData, DataSize);

}