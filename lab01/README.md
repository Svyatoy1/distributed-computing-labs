# EXERCISE 2 

## Task 1 - Open the Project (Стартове повідомлення)
**Що зроблено:** після оголошення змінних додано у main додано вивід "Serial matrix-vector multiplication program", а також додано інформацію на gitignore

## Task 2 - ProcessInitialization: ввід Size із перевіркою
**Додано:** функцію `ProcessInitialization(double*&, double*&, double*&, int&)`.
**Логіка:** цикл `do…while` запитує розмір доти, доки `Size > 0`. Некоректний ввід чистимо через `cin.clear()` + `cin.ignore(...)`.
**Вивід:** показує `Chosen objects size = N`

## Task 3 - Memory allocation + DummyDataInitialization
**Що зроблено:** у `ProcessInitialization(...)` після коректного вводу `Size` додаємо `new[]` для `pMatrix` (Size×Size), `pVector` (Size), `pResult` (Size) та викликаємо `DummyDataInitialization(pMatrix, pVector, Size)`, яка заповнює: рядок i — значення i, вектор — 1.
**Перевірка:** для Size=3 очікувано A = [0 0 0; 1 1 1; 2 2 2], b = [1 1 1]

## Task 4 - ProcessTermination: звільнення пам’яті
**Що зроблено:** додано `ProcessTermination(double*&, double*&, double*& )`, яка виконує`delete[]` для `pMatrix`, `pVector`, `pResult` та зануляє вказівники, щоб запобігти витокам пам’яті та подвійним звільненням.

## Task 5 - Implementation the Matrix-Vector Multiplication
**Що зроблено:** реалізовано добуток `c = A * b` у функції `ResultCalculation(...)` (подвійний цикл i,j, row-major).
**Перевірка:** для тестових даних (рядок i, вектор 1) маємо `c[i] = i * Size`. При Size=4 маємо результат `0 4 8 12`.

## Task 6 - Carrying out the Computational Experiments
**Що зроблено:** створено функцію `RandomDataInitialization(...)` для формування випадкових значень у матриці та векторі. Додано функції `clock_t`, `clock`, `CLOCKS_PER_SEC` з бібліотеки `ctime` для визначення часу реалізації функції.
**Перевірка:** дійсно, при тестуванні програми використовуються випадкові числа.

### Серіальні експерименти (clock)

| Test# | Matrix Size | Execution Time (sec) |
|:-----:|------------:|---------------------:|
| 1     | 10          | 0.000002             |
| 2     | 100         | 0.000057             |
| 3     | 1,000       | 0.005746             |
| 4     | 2,000       | 0.014782             |
| 5     | 3,000       | 0.025302             |
| 6     | 4,000       | 0.059550             |
| 7     | 5,000       | 0.070139             |
| 8     | 6,000       | 0.125510             |
| 9     | 7,000       | 0.180093             |
| 10    | 8,000       | 0.186502             |
| 11    | 9,000       | 0.240879             |
| 12    | 10,000      | 0.287437             |

# EXERCISE 4
## Task 1 - Open the New Project
**Що зроблено:** створено нову програму `ParallelMV.cpp` й для тесту виведено повідомлення "Serial matrix-vector multiplication program"