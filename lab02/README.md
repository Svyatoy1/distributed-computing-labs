# EXERCISE 2 
Запускаємо програму в наступній послідовності:
`g++ src/SerialMM.cpp -o SerialMM`або `g++ -O3 -march=native -funroll-loops -fopenmp -DNDEBUG -w src/SerialMM.cpp -o SerialMM`
`./SerialMM`

### Перевірка послідовного алгоритму множення матриць

| Test# | Matrix Size | Execution Time (sec) |
|:-----:|------------:|---------------------:|
| 1     | 10          | 0.000011             |
| 2     | 100         | 0.000815             |
| 3     | 500         | 0.074763             |
| 4     | 1,000       | 0.488525             |
| 5     | 1,500       | 1.650440             |
| 6     | 2,000       | 3.952903             |
| 7     | 2,500       | 7.912124             |
| 8     | 3,000       | 13.522789            |

# EXERCISE 4 
Запускаємо програму в наступній послідовності:
`mpic++ src/ParallelMM.cpp -o ParallelMM`
`mpirun -np 4 ./ParallelMM`