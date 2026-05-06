#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <winsock2.h>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include "..\dir\ALg1.h"
#include "..\dir\ALg2.h"
#include <fstream>


/**
 * @file server.cpp
 * @brief Главный файл сервера, обрабатывающий клиентские запросы
 *
 * Сервер принимает графы и полигоны от клиентов,
 * выполняет алгоритмы поиска мостов и объединения полигонов,
 * отправляет результат обратно клиенту.
 */




std::ofstream logFile; // Лог-файл
std::mutex fileLogMutex;



/**
 * @enum LogLevel
 * @brief Уровни логирования
 */
enum class LogLevel {
    INFO,
    WARNING,
    ERR,
    DEBUG
};



/**
 * @brief Логирует сообщения в консоль и файл
 * @param message Сообщение для вывода
 * @param level Уровень логирования (INFO, WARNING, ERROR, DEBUG)
 *
 * Поддерживает цветной вывод в консоли и запись в файл server.log
 */

void logMessage(const std::string& message, LogLevel level = LogLevel::INFO) {
    std::lock_guard<std::mutex> lock(fileLogMutex);
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_time_t);

    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &now_tm);

    std::ostringstream oss;
    std::string levelStr;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    switch (level) {
        case LogLevel::INFO:    levelStr = "INFO";    SetConsoleTextAttribute(hConsole, 7); break;   // Белый
        case LogLevel::WARNING: levelStr = "WARNING";  SetConsoleTextAttribute(hConsole, 14); break; // Жёлтый
        case LogLevel::ERR:   levelStr = "ERR";   SetConsoleTextAttribute(hConsole, 12); break; // Красный
        case LogLevel::DEBUG:   levelStr = "DEBUG";   SetConsoleTextAttribute(hConsole, 8); break;  // Серый
    }

    oss << "[" << time_str << "] [" << levelStr << "] " << message;

    // Вывод в консоль
    std::cout << oss.str() << std::endl;

    // Восстанавливаем цвет
    SetConsoleTextAttribute(hConsole, 7);

    // Пишем в файл всегда белым
    if (logFile.is_open()) {
        logFile << oss.str() << std::endl;
    }
}

#define PORT 5000



/**
 * @enum Packet
 * @brief Типы сообщений, которые могут быть отправлены клиентом
 */
enum Packet {
    P_Alg1 = 1,       ///< Алгоритм 1: Объединение полигонов
    P_Alg2 = 2,       ///< Алгоритм 2: Поиск мостов в графе
    P_End = 3,        ///< Завершение текущей операции
    P_Disconnect = 4  ///< Клиент завершает соединение
};


SOCKET Connections[100]; // Массив подключений клиентов
std::mutex slotMutexes[100]; // Мьютексы для каждого слота
std::atomic<bool> running(true);
std::atomic<int> nextClientID(1); // Уникальный ID для каждого клиента
std::mutex logMutex; // Для защиты вывода в консоль
std::mutex queueMutex;
std::condition_variable taskAvailable;
std::queue<int> clientQueue;

// --- Слоты клиентов ---
std::vector<bool> clientSlots(100, false); // обычный вектор
std::mutex slotsMutex;                     // защита доступа к слотам
std::condition_variable_any slotAvailable;
// -----------------------

// Функция-обработчик сигнала завершения
BOOL WINAPI HandleCtrlEvent(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_CLOSE_EVENT) {
        running = false;
        taskAvailable.notify_all();
        logMessage("[INFO] Server shutdown initiated by user.");
        return TRUE;
    }
    return FALSE;
}


void sendToClient(SOCKET clientSocket, const std::string& message) {
    send(clientSocket, message.c_str(), message.size(), 0);
}



/**
 * @brief Обрабатывает входящий пакет от клиента
 * @param index Индекс слота подключения
 * @param packettype Тип пакета (P_Alg1, P_Alg2 и т.д.)
 * @param clientID Уникальный ID клиента
 * @return true если пакет обработан успешно, false при ошибке
 *
 * Эта функция определяет тип команды от клиента и вызывает соответствующую логику:
 * - P_Alg1: объединяет полигоны
 * - P_Alg2: ищет мосты в графе
 * - P_Disconnect: закрывает соединение
 */

bool ProcessPacket(int index, Packet packettype, int clientID) {
    std::lock_guard<std::mutex> slotLock(slotMutexes[index]); // Защита уровня слота

    switch(packettype) {
        case P_Alg1: {
            logMessage("[Client ID=" + std::to_string(clientID) + "][Slot=" + std::to_string(index) + "] Processing P_Alg1 packet", LogLevel::INFO);

            int L_size;
            if(recv(Connections[index], (char*)&L_size, sizeof(int), 0) <= 0 || L_size <= 0) {
                logMessage("[Client ID=" + std::to_string(clientID) + "][Slot=" + std::to_string(index) + "] Invalid data size for left polygon", LogLevel::ERR);
                return false;
            }

            char* L = new char[L_size + 1];
            L[L_size] = '\0';
            if(recv(Connections[index], L, L_size, 0) <= 0) {
                delete[] L;
                logMessage("[Client ID=" + std::to_string(clientID) + "][Slot=" + std::to_string(index) + "] Failed to receive left polygon data", LogLevel::ERR);
                return false;
            }

            int R_size;
            if(recv(Connections[index], (char*)&R_size, sizeof(int), 0) <= 0 || R_size <= 0) {
                delete[] L;
                logMessage("[Client ID=" + std::to_string(clientID) + "][Slot=" + std::to_string(index) + "] Invalid data size for right polygon", LogLevel::ERR);
                return false;
            }

            char* R = new char[R_size + 1];
            R[R_size] = '\0';
            if(recv(Connections[index], R, R_size, 0) <= 0) {
                delete[] L;
                delete[] R;
                logMessage("[Client ID=" + std::to_string(clientID) + "][Slot=" + std::to_string(index) + "] Failed to receive right polygon data", LogLevel::ERR);
                return false;
            }

            std::string leftPolygonData(L);
            std::string rightPolygonData(R);
            delete[] L;
            delete[] R;

            std::vector<Point> leftPoints = parsePoints(leftPolygonData);
            std::vector<Point> rightPoints = parsePoints(rightPolygonData);
            sortPointsClockwise(leftPoints);
            sortPointsClockwise(rightPoints);
            std::vector<Point> result = merge(leftPoints, rightPoints);

            std::ostringstream intermediateStream;
            for(const auto& point : result) {
                intermediateStream << point.x << " " << point.y << "\n";
            }

            sendToClient(Connections[index], "INTERMEDIATE\n" + intermediateStream.str());
            sendToClient(Connections[index], "RESULT\n" + intermediateStream.str());

            logMessage("[Client ID=" + std::to_string(clientID) + "][Slot=" + std::to_string(index) + "] Sent results", LogLevel::INFO);

            Packet endPacket;
            if(recv(Connections[index], (char*)&endPacket, sizeof(Packet), 0) <= 0 || endPacket != P_End) {
                logMessage("[Client ID=" + std::to_string(clientID) + "][Slot=" + std::to_string(index) + "] Expected P_End but not received", LogLevel::ERR);
                return false;
            }
            break;
        }

        case P_Disconnect: {
            logMessage("[Client ID=" + std::to_string(clientID) + "][Slot=" + std::to_string(index) + "] Client sent disconnect signal", LogLevel::INFO);
            closesocket(Connections[index]);
            Connections[index] = INVALID_SOCKET;
            return false;
        }

        case P_Alg2: {
            logMessage("[Client ID=" + std::to_string(clientID) + "][Slot=" + std::to_string(index) + "] Processing P_Alg2 packet", LogLevel::INFO);

            int G_size;
            if(recv(Connections[index], (char*)&G_size, sizeof(int), 0) <= 0 || G_size <= 0) {
                logMessage("[Client ID=" + std::to_string(clientID) + "][Slot=" + std::to_string(index) + "] Invalid data size for graph", LogLevel::ERR);
                return false;
            }

            char* G_data = new char[G_size + 1];
            G_data[G_size] = '\0';
            if(recv(Connections[index], G_data, G_size, 0) <= 0) {
                delete[] G_data;
                logMessage("[Client ID=" + std::to_string(clientID) + "][Slot=" + std::to_string(index) + "] Failed to receive graph data", LogLevel::ERR);
                return false;
            }

            std::istringstream iss(G_data);
            delete[] G_data;

            std::vector<std::pair<int, int>> edges;
            std::string line;
            while(std::getline(iss, line)) {
                int u, v;
                if(sscanf(line.c_str(), "%d %d", &u, &v) == 2) {
                    edges.emplace_back(u, v);
                }
            }

            std::set<int> all_nodes;
            for (auto [u, v] : edges) {
                all_nodes.insert(u);
                all_nodes.insert(v);
            }

            if (all_nodes.empty()) {
                sendToClient(Connections[index], "BRIDGES\n");
                return true;
            }

            std::map<int, int> renumber;
            int idx = 0;
            for (int node : all_nodes)
                renumber[node] = idx++;

            std::vector<std::vector<int>> adj_list(idx);
            for (auto [u, v] : edges) {
                int u_new = renumber[u];
                int v_new = renumber[v];
                adj_list[u_new].push_back(v_new);
                adj_list[v_new].push_back(u_new);
            }

            BridgeFinder bridgeFinder;
            auto result_bridges = bridgeFinder.find_bridges(adj_list, idx);

            std::ostringstream oss;
            for(auto [a, b] : result_bridges) {
                for (const auto& [u, v] : edges) {
                    int u_ren = renumber[u];
                    int v_ren = renumber[v];
                    if ((u_ren == a && v_ren == b) || (u_ren == b && v_ren == a)) {
                        oss << u << " " << v << "\n";
                        break;
                    }
                }
            }

            std::string bridge_result = oss.str();
            sendToClient(Connections[index], "BRIDGES\n" + bridge_result);

            logMessage("[Client ID=" + std::to_string(clientID) + "][Slot=" + std::to_string(index) + "] Sent bridge results", LogLevel::INFO);

            Packet endPacket;
            if(recv(Connections[index], (char*)&endPacket, sizeof(Packet), 0) <= 0 || endPacket != P_End) {
                logMessage("[Client ID=" + std::to_string(clientID) + "][Slot=" + std::to_string(index) + "] Expected P_End but not received", LogLevel::ERR);
                return false;
            }
            break;
        }

        default:
            logMessage("[Client ID=" + std::to_string(clientID) + "][Slot=" + std::to_string(index) + "] Unrecognized packet: " + std::to_string((int)packettype), LogLevel::ERR);
            return false;
    }

    return true;
}


/**
 * @brief Обработчик клиента — основной поток работы с каждым подключением
 * @param index Индекс слота, где находится сокет клиента
 *
 * Получает данные от клиента, обрабатывает пакеты,
 * вызывает соответствующие алгоритмы и отправляет ответы.
 */

void ClientHandler(int index) {
    int clientID = nextClientID++; // Получаем уникальный ID
    logMessage("[Client ID=" + std::to_string(clientID) + "][Slot=" + std::to_string(index) + "] Client handler started", LogLevel::INFO);

    Packet packetType;
    while(running) {
        int bytesReceived = recv(Connections[index], (char*)&packetType, sizeof(Packet), 0);
        if(bytesReceived <= 0) {
            logMessage("[Client ID=" + std::to_string(clientID) + "][Slot=" + std::to_string(index) + "] No data received from client", LogLevel::ERR);
            closesocket(Connections[index]);
            break;
        }

        if (!ProcessPacket(index, packetType, clientID)) {
            break;
        }
    }

    if (index >= 0 && static_cast<size_t>(index) < clientSlots.size()) {
        std::lock_guard<std::mutex> lock(slotsMutex);
        clientSlots[index] = false;
        slotAvailable.notify_one();
    }

    logMessage("[Client ID=" + std::to_string(clientID) + "][Slot=" + std::to_string(index) + "] ClientHandler finished", LogLevel::INFO);
}



int GetAvailableSlot() {
    std::unique_lock<std::mutex> lock(slotsMutex);
    slotAvailable.wait(lock, []{
        return !running || std::any_of(clientSlots.begin(), clientSlots.end(),
            [](bool slot){ return !slot; });
    });

    if (!running) return -1;

    for (size_t i = 0; i < clientSlots.size(); ++i) {
        if (!clientSlots[i]) {
            clientSlots[i] = true;
            return static_cast<int>(i);
        }
    }

    return -1;
}


/**
 * @brief Принимает новые клиентские подключения
 * @param sListen Слушающий сокет
 *
 * При каждом новом подключении освобождает слот и добавляет клиент в очередь задач
 */
void AcceptClients(SOCKET sListen) {
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    while(running) {
        SOCKET newConnection = accept(sListen, (SOCKADDR*)&addr, nullptr);
        if(newConnection == INVALID_SOCKET) {
            logMessage("[Server] Accept failed", LogLevel::ERR);
            continue;
        }

        logMessage("[Server] New client connected!", LogLevel::INFO);
        int index = GetAvailableSlot();

        if (index == -1) {
            logMessage("[Server] No available slot for new connection", LogLevel::ERR);
            closesocket(newConnection);
            continue;
        }

        std::lock_guard<std::mutex> queueLock(queueMutex);
        Connections[index] = newConnection;
        clientQueue.push(index);
        taskAvailable.notify_one();
    }
}


/**
 * @brief Рабочий поток, обслуживающий клиентов
 *
 * Выполняет задачи из очереди, вызывая ClientHandler()
 */
void WorkerThread() {
    while(running) {
        std::unique_lock<std::mutex> lock(queueMutex);
        taskAvailable.wait(lock, []{ return !clientQueue.empty() || !running; });
        if(!running) return;
        int index = clientQueue.front();
        clientQueue.pop();
        lock.unlock();

        std::thread clientThread(ClientHandler, index);
        clientThread.detach();
    }
}



/**
 * @brief Рабочий поток, обслуживающий клиентов
 *
 * Выполняет задачи из очереди, вызывая ClientHandler()
 */
int main() {
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if(WSAStartup(DLLVersion, &wsaData) != 0) {
        logMessage("Error initializing Winsock", LogLevel::ERR);
        return 1;
    }

    // Регистрируем обработчик сигнала завершения
    SetConsoleCtrlHandler(HandleCtrlEvent, TRUE);

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
    listen(sListen, SOMAXCONN);
    logMessage("Server is listening on port " + std::to_string(PORT) + "... Ctrl+C to stop.", LogLevel::INFO);

    logFile.open("server.log", std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        logMessage("Failed to open log file!", LogLevel::WARNING);
    } else {
        logMessage("Server started.", LogLevel::INFO);
    }

    const int THREAD_COUNT = 10;
    std::vector<std::thread> workers;
    for(int i = 0; i < THREAD_COUNT; ++i) {
        workers.emplace_back(WorkerThread);
    }

    std::thread acceptThread(AcceptClients, sListen);
    acceptThread.detach();

    // Ожидаем завершения всех потоков
    while(running) {
        Sleep(100); // Проверяем каждые 100 мс
    }


    // Ждём завершение всех потоков
    running = false;
    taskAvailable.notify_all();
    for(auto& worker : workers) {
        if(worker.joinable()) worker.join();
    }

    closesocket(sListen);
    WSACleanup();

    if (logFile.is_open()) {
        logMessage("Server shutting down.", LogLevel::INFO);
        logFile.close();
    }

    return 0;
}