import matplotlib.pyplot as plt
import networkx as nx
import os


def draw_graph(test_number, graph, bridges=None):
    """
    @brief Рисует граф с выделением мостов красным цветом
    @param test_number Номер теста
    @param graph Граф в формате словаря {node: [neighbors]}
    @param bridges Список мостов в виде пар вершин [(u, v)]

    @details
    Использует библиотеку networkx для построения графа:
    - Все рёбра чёрные
    - Мосты красные
    - Вершины голубые
    """
    if bridges is None:
        bridges = []

    # Создаем граф в networkx
    G = nx.Graph()

    # Добавляем все вершины, даже изолированные
    all_nodes = set(graph.keys())
    for node in all_nodes:
        G.add_node(node)

    # Добавляем рёбра
    for u in graph:
        for v in graph[u]:
            if u < v:  # чтобы не дублировать
                G.add_edge(u, v)

    # Расположение вершин — можно использовать другие layout: spring, circular, shell и т.д.
    pos = nx.spring_layout(G, seed=42)  # фиксируем seed для воспроизводимости

    plt.figure(figsize=(10, 8))

    # Рисуем все рёбра
    nx.draw_networkx_edges(G, pos, edgelist=G.edges(), alpha=0.5, edge_color='black', width=1)

    # Рисуем мосты отдельно — красные и потолще
    valid_bridges = [(u, v) for u, v in bridges if u in pos and v in pos]
    if valid_bridges:
        nx.draw_networkx_edges(G, pos, edgelist=valid_bridges, edge_color='red', width=2)

    # Рисуем вершины
    nx.draw_networkx_nodes(G, pos, node_size=400, node_color='skyblue', alpha=0.9)

    # Подписи к вершинам
    nx.draw_networkx_labels(G, pos, font_size=10, font_color='black')

    plt.title(f"Test {test_number}: Graph with Bridges", fontsize=14)
    plt.axis('off')  # скрываем оси

    result_dir = "results/alg2"
    os.makedirs(result_dir, exist_ok=True)
    plt.savefig(f"{result_dir}/test_{test_number}.png", dpi=150, bbox_inches='tight')
    plt.close()

    print(f"[Test {test_number}] Graph saved to {result_dir}/test_{test_number}.png")