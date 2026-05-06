#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm> // Для max_element и min_element
#include <cmath> // atan2

#define M_PI 3.14159265358979323846
#define EPS 0.0000000001
using namespace std;

// Структура для хранения точки
struct Point {
    double x, y;
    // Конструктор с параметрами
    Point(double x = 0, double y = 0) : x(x), y(y) {}
    // Перегрузка оператора << для вывода в поток
    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        os << "Point(" << p.x << ", " << p.y << ")";
        return os;
    }

    friend std::ostream& operator<<(std::ostream& os, const std::vector<Point>& points) {
        for (const auto& p : points) {
            os << p << "\n";
        }
        return os;
    }
};

struct Pare {
    int x, y;
    // Конструктор с параметрами
    Pare(int x = 0, int y = 0) : x(x), y(y) {}
    // Перегрузка оператора << для вывода в поток
    friend std::ostream& operator<<(std::ostream& os, const Pare& p) {
        os << "Pare(" << p.x << ", " << p.y << ")";
        return os;
    }
};

// Функция для парсинга точек из строки
std::vector<Point> parsePoints(const std::string& data) ;

// Функция для вычисления ориентированной площади треугольника
double crossProduct(const Point& a, const Point& o, const Point& b);

// Функция для сравнения точек по углу относительно центроида
bool compareClockwise(const Point& a, const Point& b, const Point& centroid);

// Функция для сортировки точек по часовой стрелке
std::vector<Point> sortPointsClockwise(std::vector<Point>& points) ;

// Функция для проверки, отсортированы ли точки по часовой стрелке
bool validateClockwiseOrder(const std::vector<Point>& points) ;

// Перечисление для типов мостов
enum BridgeType { UPPER, LOWER };


// Функция для нахождения касательной прямой
int& supportingLine(std::vector<Point>& P, int type, Point& v, int& vp) ;

// Функция для нахождения моста между двумя полигонами
const Pare& bridge(std::vector<Point>& L, std::vector<Point>& R, int type, Pare& ind) ;

// Функция для слияния двух выпуклых оболочек
std::vector<Point> merge(std::vector<Point>& L, std::vector<Point>& R) ;


