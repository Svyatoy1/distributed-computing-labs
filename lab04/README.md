# EXERCISE 2 

Запускаємо програму в наступній послідовності:
`g++ src/SerialBubbleSort.cpp -o SerialBubbleSort` `./SerialBubbleSort`

### Перевірка послідовного алгоритму сортування "бульбашкою"

| Test# | Array Size  | Execution Time (sec) | Execution Time from Library (sec) |
|:-----:|------------:|---------------------:|----------------------------------:|
| 1     | 10          | 0.000002             | 0.000004
| 2     | 100         | 0.000049             | 0.000013
| 3     | 10,000      | 0.219409             | 0.001990
| 4     | 20,000      | 0.939857             | 0.004401
| 5     | 30,000      | 2.503198             | 0.006534
| 6     | 40,000      | 4.695313             | 0.009103
| 7     | 50,000      | 7.653179             | 0.009725

# EXERCISE 4
Запускаємо програму в наступній послідовності:
`mpic++ src/ParallelBubbleSort.cpp -o ParallelBubbleSort`, `mpirun -np 1 ./ParallelBubbleSort`, `mpirun --oversubscribe -np 8 ./ParallelBubbleSort`