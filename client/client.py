"""
@file client.py
@brief Клиент для отправки графов на сервер и получения мостов

Этот клиент:
- Подключается к серверу
- Отправляет граф
- Получает список мостов
- Сохраняет результат в файл или очередь
"""

import socket
import struct
import json
import os
from graph_plotter import draw_graph

# Функция отправки графа
def send_graph(sock, graph):
    data_str = ""
    for u in graph:
        for v in graph[u]:
            if u < v:  # чтобы не дублировать рёбра
                data_str += f"{u} {v}\n"
    data_bytes = data_str.encode('utf-8')
    sock.send(struct.pack('I', len(data_bytes)))
    sock.sendall(data_bytes)

# Получение мостов от сервера
def receive_bridges(sock):
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

# Основная функция клиента
def run_client(test_number, graph_data, output_dir="results/alg2"):
    print(f"[Test {test_number}] Connecting to server...")
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(('localhost', 5000))
        s.send(struct.pack('i', 2))  # P_Alg2

        send_graph(s, graph_data)  # сериализует и отправляет граф
        bridges = receive_bridges(s)  # ожидает ответ BRIDGES от сервера

        print(f"[Test {test_number}] Received bridges: {bridges}")

        # Преобразуем граф, чтобы все ключи и значения были int
        graph_data = {int(u): [int(v) for v in neighbors] for u, neighbors in graph_data.items()}

        # Сохраняем результат в JSON
        os.makedirs(output_dir, exist_ok=True)
        result_path = os.path.join(output_dir, f"result_{test_number}.json")
        with open(result_path, "w") as f:
            json.dump({
                "test_number": test_number,
                "graph": graph_data,
                "bridges": bridges
            }, f, indent=2)

        print(f"[Test {test_number}] Results saved to {result_path}")

        # Завершаем соединение
        s.send(struct.pack('i', 3))  # P_End
        s.send(struct.pack('i', 4))  # P_Disconnect
        s.close()
    except Exception as e:
        print(f"[Test {test_number}] Error: {e}")
        return False
    return True


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Клиент для отправки графа на сервер")
    parser.add_argument("test_number", type=int, help="Номер теста")
    parser.add_argument("graph_name", choices=["linear", "cycle", "tree", "multi", "complex"], help="Тип графа")

    args = parser.parse_args()

    # Импортируем графы
    from generate_test_graphs import get_test_graph # импортируем тестовые графы из другого файла

    graph = get_test_graph(args.graph_name)
    success = run_client(args.test_number, graph)

    if success:
        print(f"[Test {args.test_number}] Finished successfully.")
    else:
        print(f"[Test {args.test_number}] Failed or no results received.")