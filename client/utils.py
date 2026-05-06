import numpy as np
import math
import struct
import socket

def sort_points_clockwise(points):
    points = np.array(points)
    centroid = np.mean(points, axis=0)
    angles = np.arctan2(points[:, 1] - centroid[1], points[:, 0] - centroid[0])
    sorted_indices = np.argsort(angles)
    return points[sorted_indices].tolist()

def send_polygon(sock, points):
    polygon_data = "\n".join([f"{x} {y}" for x, y in points])
    data_bytes = polygon_data.encode('utf-8')
    sock.send(struct.pack('I', len(data_bytes)))
    sock.sendall(data_bytes)

def receive_intermediate_results(sock):
    try:
        data = b''
        while True:
            chunk = sock.recv(4096)
            if not chunk:
                break
            data += chunk
            if b"INTERMEDIATE\n" in data:
                _, rest = data.split(b"INTERMEDIATE\n", 1)
                lines = rest.decode('utf-8').strip().split('\n')
                return [tuple(map(float, line.split())) for line in lines]
    except Exception as e:
        print(f"[Error] Intermediate result: {e}")
        return []

def receive_result(sock):
    try:
        data = b''
        while True:
            chunk = sock.recv(4096)
            if not chunk:
                break
            data += chunk
            if b"RESULT\n" in data:
                _, rest = data.split(b"RESULT\n", 1)
                lines = rest.decode('utf-8').strip().split('\n')
                return [tuple(map(float, line.split())) for line in lines]
    except Exception as e:
        print(f"[Error] Final result: {e}")
        return []

def generate_regular_polygon(n_sides, radius, center=(0, 0)):
    return [(center[0] + radius * math.cos(2 * math.pi * i / n_sides),
             center[1] + radius * math.sin(2 * math.pi * i / n_sides)) for i in range(n_sides)]

def generate_ellipse(a, b, num_points, center=(0, 0)):
    return [(center[0] + a * math.cos(t), center[1] + b * math.sin(t))
            for t in [2 * math.pi * i / num_points for i in range(num_points)]]



