#include "Alg2.h"
#include <vector>

/**
 * @class BridgeFinder
 * @brief Реализация алгоритма поиска мостов графа
 *
 * Использует DFS для поиска рёбер, удаление которых приводит к разрыву связности графа.
 */

void BridgeFinder::dfs(int v, int p, const std::vector<std::vector<int>>& graph) {
    visited[v] = true;
    tin[v] = fup[v] = timer++;
    for (int to : graph[v]) {
        if (to == p) continue;
        if (visited[to])
            fup[v] = std::min(fup[v], tin[to]);
        else {
            dfs(to, v, graph);
            fup[v] = std::min(fup[v], fup[to]);
            if (fup[to] > tin[v])
                bridges.push_back({v, to});
        }
    }
}

std::vector<std::pair<int, int>> BridgeFinder::find_bridges(const std::vector<std::vector<int>>& graph, int n) {
    visited.assign(n, false);
    tin.assign(n, -1);
    fup.assign(n, -1);
    bridges.clear();
    timer = 0;

    for (int i = 0; i < n; ++i)
        if (!visited[i])
            dfs(i, -1, graph);

    return bridges;
}