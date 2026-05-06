#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <winsock2.h>
#include <sstream>
#include <algorithm>
#include <cmath>
#include "dir1.h"

#define PORT 5000

// Типы пакетов
enum Packet {
    P_Alg1=1,
    P_Alg2=2,
    P_End=3,
    P_Disconnect=4,
};

SOCKET Connections[100]; // Массив подключений клиентов
std::atomic<bool> running(true);

std::mutex mtx; // Для синхронизации вывода в консоль
std::mutex queueMutex;
std::condition_variable taskAvailable;
std::queue<int> clientQueue;

// --- Слоты клиентов ---
std::vector<bool> clientSlots(100, false); // обычный вектор
std::mutex slotsMutex;                     // защита доступа к слотам
std::condition_variable_any slotAvailable;
// -----------------------


void sendToClient(SOCKET clientSocket, const std::string& message) {
    send(clientSocket, message.c_str(), message.size(), 0);
}

bool ProcessPacket(int index, Packet packettype) {
    std::lock_guard<std::mutex> lock(mtx);
    switch(packettype) {
        case P_Alg1: {
            int L_size;
            if(recv(Connections[index], (char*)&L_size, sizeof(int), 0) <= 0 || L_size <= 0) {
                std::cerr << "Invalid data size for left polygon from client " << index << std::endl;
                return false;
            }

            char* L = new char[L_size + 1];
            L[L_size] = '\0';
            if(recv(Connections[index], L, L_size, 0) <= 0) {
                std::cerr << "Failed to receive left polygon data from client " << index << std::endl;
                delete[] L;
                return false;
            }

            int R_size;
            if(recv(Connections[index], (char*)&R_size, sizeof(int), 0) <= 0 || R_size <= 0) {
                std::cerr << "Invalid data size for right polygon from client " << index << std::endl;
                delete[] L;
                return false;
            }

            char* R = new char[R_size + 1];
            R[R_size] = '\0';
            if(recv(Connections[index], R, R_size, 0) <= 0) {
                std::cerr << "Failed to receive right polygon data from client " << index << std::endl;
                delete[] L;
                delete[] R;
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

            // Ждём сигнал P_End
            Packet endPacket;
            if(recv(Connections[index], (char*)&endPacket, sizeof(Packet), 0) <= 0 || endPacket != P_End) {
                std::cerr << "[Error] Expected P_End from client " << index << std::endl;
                return false;
            }
            break;
        }
        case P_Disconnect: {
            std::cout << "Client " << index << " sent disconnect signal.\n";
            closesocket(Connections[index]);
            Connections[index] = INVALID_SOCKET;
            return false;
        }
        default:
            std::cout << "Unrecognized packet: " << packettype << " from client " << index << std::endl;
            return false;
    }
    return true;
}

void ClientHandler(int index) {
    Packet packetType;
    while(running) {
        int bytesReceived = recv(Connections[index], (char*)&packetType, sizeof(Packet), 0);
        if(bytesReceived <= 0) {
            std::cout << "No data received from client " << index << std::endl;
            closesocket(Connections[index]);
            break;
        }
        if (!ProcessPacket(index, packetType)) {
            std::cout << "Processing packet failed for client " << index << std::endl;
            break;
        }
    }

    if (index >= 0 && static_cast<size_t>(index) < clientSlots.size()) {
        std::lock_guard<std::mutex> lock(slotsMutex);
        clientSlots[index] = false;
        slotAvailable.notify_one(); // Уведомляем очередь
    }

    std::cout << "ClientHandler finished for client " << index << std::endl;
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

void AcceptClients(SOCKET sListen) {
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    while(running) {
        SOCKET newConnection = accept(sListen, (SOCKADDR*)&addr, nullptr);
        if(newConnection == INVALID_SOCKET) {
            std::cerr << "Accept failed\n";
            continue;
        } else {
            std::cout << "Client connected!\n";

            int index = GetAvailableSlot();
            if (index == -1) {
                closesocket(newConnection);
                continue;
            }

            std::lock_guard<std::mutex> queueLock(queueMutex);
            Connections[index] = newConnection;
            clientQueue.push(index);
            taskAvailable.notify_one();
        }
    }
}

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

int main() {
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if(WSAStartup(DLLVersion, &wsaData) != 0) {
        std::cout << "Error initializing Winsock\n";
        return 1;
    }

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
    listen(sListen, SOMAXCONN);
    std::cout << "Server is listening on port " << PORT << "... Ctrl+C to stop.\n";

    // Запуск рабочих потоков
    const int THREAD_COUNT = 10;
    std::vector<std::thread> workers;
    for(int i = 0; i < THREAD_COUNT; ++i) {
        workers.emplace_back(WorkerThread);
    }

    // Запуск потока для принятия новых соединений
    std::thread acceptThread(AcceptClients, sListen);
    acceptThread.detach();

    system("pause");
    running = false;
    taskAvailable.notify_all();
    for(auto& worker : workers) {
        if(worker.joinable()) worker.join();
    }
    closesocket(sListen);
    WSACleanup();
    return 0;
}