тут создается возможность одним сервером обрабатывать два алгоритма, и одним клиентом посылать тесты на два алгоритма

тут еще должен быть скрипт вызывающий клиент, чтобы распараллелить отрисовку


компиляция пока:

g++ -std=c++20 -Wall -Wextra -fno-sanitize=float-cast-overflow -pthread server.cpp dir1.cpp -o server -lws2_32

g++ -std=c++20 -Wall -Wextra -fno-sanitize=float-cast-overflow -pthread .\server\server.cpp .\dir\Alg1.cpp .\dir\Alg2.cpp -o server -lws2_32


& C:\Users\so-ta\AppData\Local\Programs\Python\Python313\python.exe client.py

& C:\Users\so-ta\AppData\Local\Programs\Python\Python313\python.exe .\client\client.py


