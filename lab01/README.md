## EX2 
# Task 1 - Open the Project (Стартове повідомлення)
**Що зроблено:** після оголошення змінних додано у main додано вивід "Serial matrix-vector multiplication program", а також додано інформацію на gitignore

# Task 2 — ProcessInitialization: ввід Size із перевіркою
**Додано:** функцію `ProcessInitialization(double*&, double*&, double*&, int&)`.
**Логіка:** цикл 'do…while' запитує розмір доти, доки 'Size > 0'. Некоректний ввід чистимо через 'cin.clear()' + 'cin.ignore(...)'.
**Вивід:** показує 'Chosen objects size = N'