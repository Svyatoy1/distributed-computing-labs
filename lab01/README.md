# EX2 
## Task 1 - Open the Project (Стартове повідомлення)
**Що зроблено:** після оголошення змінних додано у main додано вивід "Serial matrix-vector multiplication program", а також додано інформацію на gitignore

## Task 2 - ProcessInitialization: ввід Size із перевіркою
**Додано:** функцію `ProcessInitialization(double*&, double*&, double*&, int&)`.
**Логіка:** цикл `do…while` запитує розмір доти, доки `Size > 0`. Некоректний ввід чистимо через `cin.clear()` + `cin.ignore(...)`.
**Вивід:** показує `Chosen objects size = N`

## Task 3 - Memory allocation + DummyDataInitialization
**Що зроблено:** у `ProcessInitialization(...)` після коректного вводу `Size` додаємо `new[]` для `pMatrix` (Size×Size), `pVector` (Size), `pResult` (Size) та викликаємо `DummyDataInitialization(pMatrix, pVector, Size)`, яка заповнює: рядок i — значення i, вектор — 1.
**Перевірка:** для Size=3 очікувано A =
[0 0 0; 1 1 1; 2 2 2], b = [1 1 1]

## Task 4 - ProcessTermination: звільнення пам’яті
**Що зроблено:** додано `ProcessTermination(double*&, double*&, double*& )`, яка виконує`delete[]` для `pMatrix`, `pVector`, `pResult` та зануляє вказівники, щоб запобігти витокам пам’яті та подвійним звільненням.