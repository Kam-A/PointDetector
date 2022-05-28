#pragma once

#include <vector>
#include <random>
#include <iostream>

std::vector<int> GenerateNumbers(std::mt19937& generator, int count, int max_value);

struct Point {
    double x;
    double y;
    Point operator-(Point& rhs);
};

struct Coordinate {
    Point a;
    Point b;
};


std::ostream& operator<<(std::ostream& os, const Point& point);
std::ostream& operator<<(std::ostream& os, const Coordinate& coordinate);

class Line {
public:
    Line(Point a, Point b);
    
    double GetSlope() const;
    
    Coordinate GetCoordinate() const;
private:
    Point a_;
    Point b_;
    double slope_;
};
