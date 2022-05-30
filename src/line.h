#pragma once

#include <deque>
#include <vector>
#include <random>
#include <iostream>
#include <map>
#include <set>

struct Point {
    double x;
    double y;
    Point operator-(Point& rhs);
};

struct Coordinate {
    Point a;
    Point b;
};


std::vector<int> GenerateNumbers(std::mt19937& generator, int count, int max_value);

std::vector<Point> GenerateRandomPoints(std::mt19937& generator, int count);

std::vector<std::vector<int>> Combination(int n, int k);
//std::vector<Point> GenerateSameDistPoint(int count);

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

std::map<double, int>  GetDistanceFromLineToPoints(Line line, std::vector<Point> points);
std::map<const Line*, double>  GetDistanceFromPointToLines(Point res_point, std::vector<Line>& lines);
std::vector<std::array<double, 2>> GetAllIntersction(const std::deque<Line>& lines);
std::pair<Point, std::map<const Line*, double>>  GetIntersections(std::set<const Line *> lines);
