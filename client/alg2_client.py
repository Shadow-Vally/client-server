import socket
import struct
import threading
from graph_plotter import draw_graph

def send_graph(sock, graph):
    """
    @brief Отправляет граф на сервер
    @param sock Сокет для связи с сервером
    @param graph Граф в формате словаря {node: [neighbors]}

    @details
    Граф сериализуется в строку, где каждая строка — это пара вершин:
    ```
    u v
    ```

    Например:
    ```
    0 1
    1 2
    2 3
    ```

    Затем данные отправляются на сервер.
    """
    data_str = ""
    for u in graph:
        for v in graph[u]:
            if u < v:  # избегаем дубликатов
                data_str += f"{u} {v}\n"
    data_bytes = data_str.encode('utf-8')
    sock.send(struct.pack('I', len(data_bytes)))
    sock.sendall(data_bytes)

def receive_bridges(sock):
    """
    @brief Получает список мостов от сервера
    @param sock Сокет для связи с сервером
    @return list of tuples Список мостов в виде пар вершин [(u, v)]

    @details
    Ожидает ответ от сервера в формате:
    ```
    BRIDGES
    u1 v1
    u2 v2
    ...
    ```

    Если мостов нет, возвращает пустой список.
    """
    data = b''
    while True:
        chunk = sock.recv(4096)
        if not chunk:
            break
        data += chunk
        if b"BRIDGES" in data:
            _, rest = data.split(b"BRIDGES\n", 1)
            lines = rest.decode('utf-8').strip().split('\n')
            return [tuple(map(int, line.split())) for line in lines if line.strip()]
    return []

def run_test_alg2(test_number, graph_data, results_queue):
    """
    @brief Выполняет тест алгоритма P_Alg2 для заданного графа
    @param test_number Номер теста
    @param graph_data Граф в формате словаря {node: [neighbors]}
    @param results_queue Очередь для хранения результатов

    @details
    1. Подключается к серверу
    2. Отправляет граф
    3. Получает список мостов
    4. Сохраняет результаты в очередь
    """
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(('localhost', 5000))
        s.send(struct.pack('i', 2))  # P_Alg2

        send_graph(s, graph_data)

        bridges = receive_bridges(s)

        # Проверяем, что все вершины мостов есть в графе
        all_nodes = set(graph_data.keys())
        for u, v in bridges:
            if u not in all_nodes or v not in all_nodes:
                print(f"[Test {test_number}] Warning: Bridge ({u}, {v}) contains unknown nodes!")
                continue

        # results_queue.put((test_number, graph_data, bridges))

        # Сохраняем полный граф и мосты
        results_queue.put((test_number, graph_data, bridges))

        s.send(struct.pack('i', 3))  # P_End
        s.send(struct.pack('i', 4))  # P_Disconnect
        s.close()
    except Exception as e:
        print(f"[Test {test_number}] Error: {e}")