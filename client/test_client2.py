import threading
import queue
import time

from alg2_client import run_test_alg2
from graph_plotter import draw_graph

# --- Тестовые графы ---

# Тест 1: Линейный граф — все рёбра — мосты
test_1 = {
    0: [1],
    1: [0, 2],
    2: [1]
}

# Тест 2: Цикл из 4 вершин — нет мостов
test_2 = {
    0: [1, 3],
    1: [0, 2],
    2: [1, 3],
    3: [2, 0]
}

# Тест 3: Дерево — все рёбра — мосты
test_3 = {
    0: [1],
    1: [0, 2, 3],
    2: [1],
    3: [1, 4],
    4: [3]
}

# Тест 4: Граф с несколькими компонентами связности
test_4 = {
    0: [1],
    1: [0],
    2: [3, 4],
    3: [2],
    4: [2],
    5: [7],
    6: [],
    7: [5]
}

# Список тестовых случаев
test_cases = [
    ("Linear Graph", test_1),
    ("Cycle Graph", test_2),
    ("Tree Graph", test_3),
    ("Multi-component Graph", test_4)
]

results_queue = queue.Queue()

if __name__ == "__main__":
    print("=== Запуск тестирования алгоритма P_Alg2 ===")

    threads = []

    for idx, (name, graph) in enumerate(test_cases):
        print(f"Запуск теста {idx + 1}: {name}")
        thread = threading.Thread(
            target=run_test_alg2,
            args=(idx + 1, graph, results_queue)
        )
        threads.append(thread)
        thread.start()
        time.sleep(0.2)

    for thread in threads:
        thread.join()

    print("\nВсе тесты выполнены. Сохранение результатов...")

    while not results_queue.empty():
        test_number, graph, bridges = results_queue.get()
        print(f"[Test {test_number}] Найдено {len(bridges)} мост(а/ов).")
        draw_graph(test_number, graph, bridges)

    print("=== Тестирование завершено ===")