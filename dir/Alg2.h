#pragma once

#include <vector>
#include <utility>

class BridgeFinder {
public:
    // Конструктор
    BridgeFinder() = default;

    // Основной метод: поиск мостов
    std::vector<std::pair<int, int>> find_bridges(const std::vector<std::vector<int>>& graph, int n);

private:
    // Вспомогательный DFS
    void dfs(int v, int p, const std::vector<std::vector<int>>& graph);

    // Данные, используемые при поиске
    std::vector<int> tin;
    std::vector<int> fup;
    std::vector<bool> visited;
    std::vector<std::pair<int, int>> bridges;
    int timer = 0;
};