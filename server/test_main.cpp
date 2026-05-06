#include <iostream>
#include <vector>
#include "..\dir\ALg2.h"

int main() {
    // Пример графа: список смежности
    std::vector<std::vector<int>> graph = {
        {1},         // 0
        {0, 2},      // 1
        {1, 3, 4},   // 2
        {2, 4},      // 3
        {2, 3, 5},   // 4
        {4}          // 5
    };

    int n = graph.size();

    std::cout << "Testing Bridge Finding Algorithm..." << std::endl;
    std::cout << "Graph has " << n << " nodes." << std::endl;

    BridgeFinder bridgeFinder;
    auto bridges = bridgeFinder.find_bridges(graph, n);

    std::cout << "Found " << bridges.size() << " bridge(s):" << std::endl;
    for (const auto& [u, v] : bridges) {
        std::cout << u << " - " << v << std::endl;
    }

    std::cout << "=== Тестирование алгоритма поиска мостов ===\n";

    // Тест 1: Линейный граф — все рёбра — мосты
    {
        std::vector<std::vector<int>> graph = {
            {1},         // 0
            {0, 2},      // 1
            {1}          // 2
        };
        BridgeFinder bf;
        auto bridges = bf.find_bridges(graph, graph.size());

        std::cout << "\nТест 1: Линейный граф\n";
        std::cout << "Ожидаемые мосты: (0,1), (1,2)\n";
        std::cout << "Найденные мосты:\n";
        for (const auto& [u, v] : bridges)
            std::cout << u << " - " << v << "\n";
    }

    // Тест 2: Цикл из 4 вершин — нет мостов
    {
        std::vector<std::vector<int>> graph = {
            {1, 3},   // 0
            {0, 2},   // 1
            {1, 3},   // 2
            {2, 0}    // 3
        };
        BridgeFinder bf;
        auto bridges = bf.find_bridges(graph, graph.size());

        std::cout << "\nТест 2: Цикл из 4 вершин\n";
        std::cout << "Ожидаемые мосты: Нет\n";
        std::cout << "Найденные мосты:\n";
        if (bridges.empty())
            std::cout << "Мостов не найдено (OK)\n";
        else
            for (const auto& [u, v] : bridges)
                std::cout << u << " - " << v << "\n";
    }

    // Тест 3: Дерево — все рёбра — мосты
    {
        std::vector<std::vector<int>> graph = {
            {1},         // 0
            {0, 2, 3},   // 1
            {1},         // 2
            {1, 4},      // 3
            {3}          // 4
        };
        BridgeFinder bf;
        auto bridges = bf.find_bridges(graph, graph.size());

        std::cout << "\nТест 3: Дерево\n";
        std::cout << "Ожидаемые мосты: (0,1), (1,2), (1,3), (3,4)\n";
        std::cout << "Найденные мосты:\n";
        for (const auto& [u, v] : bridges)
            std::cout << u << " - " << v << "\n";
    }

    // Тест 4: Граф с несколькими компонентами связности
    {
        std::vector<std::vector<int>> graph = {
            {1},         // 0
            {0},         // 1
            {3, 4},      // 2
            {2},         // 3
            {2},         // 4
            {7},         // 5
            {},          // 6
            {5}          // 7
        };
        BridgeFinder bf;
        auto bridges = bf.find_bridges(graph, graph.size());

        std::cout << "\nТест 4: Множество компонент\n";
        std::cout << "Ожидаемые мосты: (0,1), (2,3), (2,4), (5,7)\n";
        std::cout << "Найденные мосты:\n";
        for (const auto& [u, v] : bridges)
            std::cout << u << " - " << v << "\n";
    }

    std::cout << "\n=== Тестирование завершено ===\n";
    return 0;
}