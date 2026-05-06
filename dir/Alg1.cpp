#include "Alg1.h"
using namespace std;



// Функция для парсинга точек из строки
std::vector<Point> parsePoints(const std::string& data) {
    std::vector<Point> points;
    std::istringstream stream(data);
    std::string line;
    while (std::getline(stream, line)) {
        double x, y;
        std::istringstream lineStream(line);
        lineStream >> x >> y;
        points.emplace_back(x, y);
    }
    return points;
}

// Функция для вычисления ориентированной площади треугольника
double crossProduct(const Point& a, const Point& o, const Point& b) {
    return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

// Функция для сравнения точек по углу относительно центроида
bool compareClockwise(const Point& a, const Point& b, const Point& centroid) {
    double angleA = std::atan2(a.y - centroid.y, a.x - centroid.x);
    double angleB = std::atan2(b.y - centroid.y, b.x - centroid.x);
    return angleA > angleB;
}

// Функция для сортировки точек по часовой стрелке
std::vector<Point> sortPointsClockwise(std::vector<Point>& points) {
    if (points.size() < 3) {
        return points;
    }

    // Вычисляем центроид
    Point centroid = {0, 0};
    for (const auto& p : points) {
        centroid.x += p.x;
        centroid.y += p.y;
    }
    centroid.x /= points.size();
    centroid.y /= points.size();

    // Выводим углы перед сортировкой
    // std::cout << "Centroid: (" << centroid.x << ", " << centroid.y << ")\n";
    // std::cout << "Angles before sorting:\n";
    // for (const auto& p : points) {
    //     double angle = std::atan2(p.y - centroid.y, p.x - centroid.x);
    //     // std::cout << "Point (" << p.x << ", " << p.y << ") -> Angle: " << angle << "\n";
    // }

    // Сортируем точки по углу относительно центроида
    std::sort(points.begin(), points.end(), [&centroid](const Point& a, const Point& b) {
        return compareClockwise(a, b, centroid);
    });

    // Выводим углы после сортировки
    // std::cout << "\nAngles after sorting:\n";
    // for (const auto& p : points) {
    //     double angle = std::atan2(p.y - centroid.y, p.x - centroid.x);
        // std::cout << "Point (" << p.x << ", " << p.y << ") -> Angle: " << angle << "\n";
    // }

    return points;
}

// Функция для проверки, отсортированы ли точки по часовой стрелке
bool validateClockwiseOrder(const std::vector<Point>& points) {
    if (points.size() < 3) {
        std::cerr << "Error: Polygon must have at least 3 points.\n";
        return false;
    }

    // Вычисляем центроид
    Point centroid = {0, 0};
    for (const auto& p : points) {
        centroid.x += p.x;
        centroid.y += p.y;
    }
    centroid.x /= points.size();
    centroid.y /= points.size();

    bool isClockwise = true;
    for (size_t i = 0; i < points.size(); ++i) {
        const Point& a = points[i];
        const Point& b = points[(i + 1) % points.size()];
        double cp = crossProduct(centroid, a, b);
        if (cp > 0) {
            isClockwise = false;
            break;
        }
    }

    if (!isClockwise) {
        std::cerr << "Error: Points are not sorted in clockwise order.\n";
        for (const auto& p : points) {
            std::cerr << "(" << p.x << ", " << p.y << ")\n";
        }
    }

    return isClockwise;
}

// Функция для нахождения касательной прямой
int& supportingLine(std::vector<Point>& P, int type, Point& v, int& vp) {
    int d = (type == UPPER) ? 1 : -1;
    int nP = P.size();
    int maxIterations = 1000;
    int iterations = 0;
    vp = (vp + nP) % nP;
    int nextP = (vp + d + nP) % nP;

    while ((iterations < maxIterations)&&(d * crossProduct(P[vp], v, P[nextP]) > EPS)){
        vp = nextP;
        nextP = (vp + d + nP) % nP;
        // cout<<"nextP: "<<nextP<<" ";
        iterations++;
    }
    // cout<<endl;

    if (iterations >= maxIterations) {
        std::cerr << "Warning: Maximum iterations reached in supportingLine.\n";
    }

    // std::cout << "Supporting line found between v = " << v <<" and vp = "
    //           << P[vp]<<"iterations:" << iterations <<"\n";
    return vp;
}

// Функция для нахождения моста между двумя полигонами
const Pare& bridge(std::vector<Point>& L, std::vector<Point>& R, int type, Pare& ind) {
    int maxIterations = 100;
    int iterations = 0;
    int vl = ind.x;
    int vr = ind.y;
    int cur_vl = vl;
    int cur_vr = vr;

    do {
        cur_vr = vr;
        supportingLine(R, type, L[vl], vr);

        cur_vl = vl;
        supportingLine(L, 1 - type, R[vr], vl);
        iterations++;
    } while (((vl != cur_vl) && (vr != cur_vr)) && iterations < maxIterations);

    if (iterations >= maxIterations) {
        std::cerr << "Warning: Maximum iterations reached in bridge.\n";
    }

    ind = {vl, vr};
    // std::cout << "Bridge between " << L[vl] << " and " << R[vr] << "\n";
    return ind;
}

// Функция для слияния двух выпуклых оболочек
std::vector<Point> merge(std::vector<Point>& L, std::vector<Point>& R) {
    // std::cout << "Begin merge\n";

    // Найти верхнюю и нижнюю вершины
    int vl = std::max_element(L.begin(), L.end(), [](const Point& a, const Point& b) {
        return a.x < b.x;
    }) - L.begin();
    int vr = std::min_element(R.begin(), R.end(), [](const Point& a, const Point& b) {
        return a.x < b.x;
    }) - R.begin();


    Pare up(vl, vr);
    Pare down(vl, vr);

    // cout<<"vl,vr = "<< up<<endl;

    // Найти верхний мост
    bridge(L, R, UPPER, up);

    // Найти нижний мост
    bridge(L, R, LOWER, down);

    int nL = L.size();
    int nR = R.size();

    // Объединить результаты
    std::vector<Point> result;
    int it =  down.x;
    while (it != up.x){
        result.push_back(L[it]);
        it = (it + 1) % nL;
    }
    result.push_back(L[it]);

    it = up.y;
    while (it != down.y){
        result.push_back(R[it]);
        it = (it + 1) % nR;
    }
    result.push_back(R[it]);

    // std::cout << "End merge\n";
    return result;
}




