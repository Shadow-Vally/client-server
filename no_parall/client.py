import socket
import matplotlib.pyplot as plt
import numpy as np
import math
import struct
import threading
import queue
import time
import os

PORT = 5000
SERVER_ADDRESS = ('localhost', PORT)
BUFFER_SIZE = 4096

# Очередь для хранения результатов тестов
results_queue = queue.Queue()

def send_polygons(server_socket, test_left_points, test_right_points):
    send_polygon(server_socket, test_left_points)
    send_polygon(server_socket, test_right_points)

def send_polygon(server_socket, points):
    polygon_data = "\n".join([f"{point[0]} {point[1]}" for point in points])
    data_bytes = polygon_data.encode('utf-8')
    server_socket.send(struct.pack('I', len(data_bytes)))  # 'I' = unsigned int
    server_socket.sendall(data_bytes)

def receive_intermediate_results(server_socket):
    try:
        data = b''
        while True:
            chunk = server_socket.recv(BUFFER_SIZE)
            if not chunk:
                break
            data += chunk
            if b"INTERMEDIATE" in data:
                _, rest = data.split(b"INTERMEDIATE\n", 1)
                lines = rest.decode('utf-8').strip().split('\n')
                points = [tuple(map(float, line.split())) for line in lines]
                return points
    except Exception as e:
        print(f"[Error] Intermediate result: {e}")
        return []

def receive_result(server_socket):
    try:
        data = b''
        while True:
            chunk = server_socket.recv(BUFFER_SIZE)
            if not chunk:
                break
            data += chunk
            if b"RESULT" in data:
                _, rest = data.split(b"RESULT\n", 1)
                lines = rest.decode('utf-8').strip().split('\n')
                points = [tuple(map(float, line.split())) for line in lines]
                return points
    except Exception as e:
        print(f"[Error] Final result: {e}")
        return []

def generate_regular_polygon(n_sides, radius, center=(0, 0)):
    return [(center[0] + radius * math.cos(2 * math.pi * i / n_sides),
             center[1] + radius * math.sin(2 * math.pi * i / n_sides)) for i in range(n_sides)]

def generate_ellipse(a, b, num_points, center=(0, 0)):
    """Генерирует точки эллипса."""
    points = []
    for i in range(num_points):
        t = 2 * math.pi * i / num_points
        x = center[0] + a * math.cos(t)
        y = center[1] + b * math.sin(t)
        points.append((x, y))
    return points

def sort_points_clockwise(points):
    centroid = np.mean(points, axis=0)
    angles = np.arctan2(points[:, 1] - centroid[1], points[:, 0] - centroid[0])
    sorted_indices = np.argsort(angles)
    return points[sorted_indices]

def plot_and_save(test_number, left_points, right_points, intermediate_points=None, final_points=None):
    plt.figure(figsize=(8, 6))

    left_sorted = sort_points_clockwise(np.array(left_points))
    right_sorted = sort_points_clockwise(np.array(right_points))
    if intermediate_points is not None:
        intermediate_sorted = sort_points_clockwise(np.array(intermediate_points))
    if final_points is not None:
        final_sorted = sort_points_clockwise(np.array(final_points))

    x, y = zip(*np.append(left_sorted, [left_sorted[0]], axis=0))
    plt.plot(x, y, color='blue', linestyle='-', linewidth=2, alpha=0.7)

    x, y = zip(*np.append(right_sorted, [right_sorted[0]], axis=0))
    plt.plot(x, y, color='green', linestyle='-', linewidth=2, alpha=0.7)

    if intermediate_points is not None:
        x, y = zip(*np.append(intermediate_sorted, [intermediate_sorted[0]], axis=0))
        plt.plot(x, y, color='gray', linestyle='--', linewidth=1.5, alpha=0.5)

    if final_points is not None:
        x, y = zip(*np.append(final_sorted, [final_sorted[0]], axis=0))
        plt.plot(x, y, color='red', linestyle='-', linewidth=2.5, alpha=0.6)

    plt.grid(True)
    plt.title(f"Test {test_number}: Polygon Merge Result")
    plt.xlabel("X-axis")
    plt.ylabel("Y-axis")

    # Сохраняем график в файл
    os.makedirs("results", exist_ok=True)
    plt.savefig(f"results/test_{test_number}.png")
    plt.close()  # Закрываем текущую фигуру

def run_test(test_number, test_left_points, test_right_points):
    try:
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.connect(SERVER_ADDRESS)
        print(f"[Test {test_number}] Connected to server at {SERVER_ADDRESS}")

        # Сортируем точки по часовой стрелке перед отправкой
        test_left_points = sort_points_clockwise(np.array(test_left_points))
        test_right_points = sort_points_clockwise(np.array(test_right_points))

        # Отправляем тип пакета
        server_socket.send(struct.pack('i', 1))  # P_Alg1

        # Отправляем полигоны серверу
        send_polygons(server_socket, test_left_points, test_right_points)

        # Получаем промежуточные результаты
        intermediate = receive_intermediate_results(server_socket)

        # Получаем окончательный результат
        final = receive_result(server_socket)

        # Добавляем результаты в очередь
        results_queue.put((test_number, test_left_points, test_right_points, intermediate, final))

        # Закрываем соединение
        # Сообщаем, что данные закончились
        server_socket.send(struct.pack('i', 3))  # P_End
        server_socket.close()

    except Exception as e:
        print(f"[Test {test_number}] Error: {e}")

def plot_all_results():
    while True:
        try:
            # Получаем результаты из очереди
            test_number, left_points, right_points, intermediate_points, final_points = results_queue.get(timeout=1)
            print(f"[Test {test_number}] Plotting results...")
            plot_and_save(test_number, left_points, right_points, intermediate_points, final_points)
        except queue.Empty:
            break

def main():
    tests = [
        ([(0, 0), (1, 0), (1, 1), (0, 1)],  # Левый прямоугольник
        [(2, 0), (3, 0), (3, 1), (2, 1)]),  # Правый прямоугольник

        # Тест 2: Восьмиугольники
        ([
        (0, 2), (1, 3), (2, 3), (3, 2),
        (3, 0), (2, -1), (1, -1), (0, 0)
        ],[
        (4, 2), (5, 3), (6, 3), (7, 2),
        (7, 0), (6, -1), (5, -1), (4, 0)
        ]),


        # Тест 3: Два правильных многоугольника разных размеров
        (generate_regular_polygon(8, 3, center=(0, 0)),  # Многоугольник с радиусом 3
        generate_regular_polygon(8, 5, center=(12, 0))),  # Многоугольник с радиусом 5

        # Тест 4: Эллипс и многоугольник
        (generate_ellipse(5, 3, 20, center=(0, 0)),  # Эллипс с полуосями 5 и 3
        generate_regular_polygon(6, 4, center=(12, 0))),  # Шестиугольник с радиусом 4
        ([(-1, 0), (0, 1), (1, 0), (0, -1)], [(2, 0), (3, 1), (4, 0), (3, -1)]),
        (generate_regular_polygon(6, 2), generate_regular_polygon(4, 3, (5, 0))),
        (generate_regular_polygon(10, 4), generate_regular_polygon(10, 3, (8, 0))),
        (generate_regular_polygon(8, 3), [(10, 0)])
    ]

    threads = []
    for i in range(130):  # Запускаем 100 тестов
        left, right = tests[i % len(tests)]
        thread = threading.Thread(target=run_test, args=(i + 1, left, right))
        threads.append(thread)
        thread.start()
        time.sleep(0.1)  # Небольшая пауза между запусками

    # Ждём завершения всех тестов
    for thread in threads:
        thread.join()

    # Отрисовываем все результаты в основном потоке
    plot_all_results()

if __name__ == "__main__":
    main()