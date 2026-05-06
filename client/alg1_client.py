import threading
import socket
import struct
from utils import send_polygon, receive_intermediate_results, receive_result
from polygon_plotter import plot_and_save

def run_test_alg1(test_number, left_points, right_points, results_queue):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(('localhost', 5000))
        s.send(struct.pack('i', 1))  # P_Alg1

        # Сортируем перед отправкой
        from utils import sort_points_clockwise
        left_sorted = sort_points_clockwise(left_points)
        right_sorted = sort_points_clockwise(right_points)

        send_polygon(s, left_sorted)
        send_polygon(s, right_sorted)

        intermediate = receive_intermediate_results(s)
        final = receive_result(s)

        results_queue.put((test_number, left_sorted, right_sorted, intermediate, final))

        s.send(struct.pack('i', 3))  # P_End
        s.send(struct.pack('i', 4))  # P_Disconnect
        s.close()
    except Exception as e:
        print(f"[Test {test_number}] Error: {e}")

# def plot_all_results():
#     while True:
#         try:
#             # Получаем результаты из очереди
#             test_number, left_points, right_points, intermediate_points, final_points = results_queue.get(timeout=1)
#             print(f"[Test {test_number}] Plotting results...")
#             plot_and_save(test_number, left_points, right_points, intermediate_points, final_points)
#         except queue.Empty:
#             break