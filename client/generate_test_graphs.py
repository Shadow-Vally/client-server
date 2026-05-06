def get_test_graph(graph_type):
    """
    Возвращает тестовый граф по имени
    """
    graphs = {
        "linear": {
            0: [1],
            1: [0, 2],
            2: [1]
        },
        "cycle": {
            0: [1, 3],
            1: [0, 2],
            2: [1, 3],
            3: [2, 0]
        },
        "tree": {
            0: [1],
            1: [0, 2, 3],
            2: [1],
            3: [1, 4],
            4: [3]
        },
        "multi": {
            0: [1],
            1: [0],
            2: [3, 4],
            3: [2],
            4: [2],
            5: [7],
            7: [5]
        },
        "complex": None  # генерируется динамически
    }

    if graph_type == "complex":
        graph = {}
        # Компонента 1: цикл из 6 вершин (0-5)
        for i in range(6):
            graph[i] = [(i + 1) % 6, (i - 1 + 6) % 6]

        # Компонента 2: дерево из 5 вершин (6-10)
        tree_edges = [
            (6, 7),
            (7, 8),
            (7, 9),
            (9, 10)
        ]
        for u, v in tree_edges:
            if u not in graph:
                graph[u] = []
            if v not in graph:
                graph[v] = []
            graph[u].append(v)
            graph[v].append(u)

        # Компонента 3: мост между 11 и 12
        graph[11] = [12]
        graph[12] = [11]

        # Компонента 4: изолированная вершина
        graph[13] = []

        return graph

    return graphs[graph_type]