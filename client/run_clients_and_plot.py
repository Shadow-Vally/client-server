import os
import subprocess
import time
import json


CLIENT_SCRIPT = r"C:\Users\so-ta\Desktop\Valla\мгу\prog\4_sem\client-server4\client\client.py"
RESULT_DIR = r"C:\Users\so-ta\Desktop\Valla\мгу\prog\4_sem\client-server4\results\alg2"

# --- Функция для отслеживания новых результатов ---
def monitor_results():
    processed = set()
    while True:
        files = os.listdir(RESULT_DIR)
        new_files = [f for f in files if f.startswith("result_") and f.endswith(".json") and f not in processed]
        for filename in new_files:
            path = os.path.join(RESULT_DIR, filename)
            try:
                with open(path, 'r') as f:
                    data = json.load(f)
                
                # Убедимся, что все данные корректны
                test_number = data["test_number"]
                graph = data["graph"]
                bridges = data.get("bridges", [])

                # Преобразуем ключи и значения графа в int, если они строки
                graph = {int(u): [int(v) for v in neighbors] for u, neighbors in graph.items()}

                print(f"[Main] Processing results from {filename}")
                draw_graph(test_number, graph, bridges)
                processed.add(filename)
            except Exception as e:
                print(f"[Main] Error reading {filename}: {e}")
        time.sleep(1)  # проверяем каждые 1 секунду     

# --- Визуализация графа ---
def draw_graph(test_number, graph, bridges=None):
    from graph_plotter import draw_graph as plot_graph
    print(f"[Main] Рисую тест {test_number}...")
    plot_graph(test_number, graph, bridges or [])
    print(f"[Test {test_number}] Граф построен.")

# --- Запуск клиента в отдельном процессе ---
def launch_client(test_number, graph_type="linear"):
    cmd = ["python", CLIENT_SCRIPT, str(test_number), graph_type]
    print(f"[Main] Запускаю клиентский процесс: {' '.join(cmd)}")
    subprocess.run(cmd)

# --- Тестовые данные ---
TEST_GRAPH_TYPES = ["linear", "cycle", "tree", "multi", "complex"]

# --- Точка входа ---
if __name__ == "__main__":
    import multiprocessing

    # Создаем директорию под результаты, если её нет
    os.makedirs(RESULT_DIR, exist_ok=True)

    # Запускаем поток/процесс мониторинга
    monitor_process = multiprocessing.Process(target=monitor_results)
    monitor_process.start()

    # Номера тестов
    test_numbers = list(range(1, len(TEST_GRAPH_TYPES) + 1))

    print("[Main] Запускаю клиентские процессы...")
    processes = []

    for i, graph_type in enumerate(TEST_GRAPH_TYPES):
        test_number = i + 1
        p = multiprocessing.Process(target=launch_client, args=(test_number, graph_type))
        processes.append(p)
        p.start()
        time.sleep(0.5) 

    # Ждём завершения всех клиентских процессов
    for p in processes:
        p.join()

    print("[Main] Все клиентские процессы завершены.")
    input("Нажмите Enter для выхода...")
    monitor_process.terminate()
    monitor_process.join()