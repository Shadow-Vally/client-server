import matplotlib.pyplot as plt
import numpy as np
import queue
import os
from utils import sort_points_clockwise

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



