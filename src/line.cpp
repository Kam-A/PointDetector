//
//  line.cpp
//  point_detector
//
//  Created by user on 28.05.2022.
//

#include "line.h"
#include <set>

std::vector<int> GenerateNumbers(std::mt19937& generator, int count, int max_value) {
    std::set<int> numbers;
    std::uniform_int_distribution<> distrib(0, max_value);
    while(numbers.size() < count) {
        numbers.insert(distrib(generator));
    }
    std::vector<int> idx_to_change(numbers.begin(), numbers.end());
    std::shuffle(idx_to_change.begin(), idx_to_change.end(), std::random_device());
    return idx_to_change;
}

Point Point::operator-(Point& rhs) {
    Point res;
    res.x = x - rhs.x;
    res.y = y - rhs.y;
    return res;
}

std::ostream& operator<<(std::ostream& os, const Point& point) {
    os << "[ " << point.x << ", " << point.y << " ]" << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Coordinate& coordinate) {
    os << coordinate.a << ", " << coordinate.b << std::endl;
    return os;
}

Line::Line(Point a, Point b) : a_(a), b_(b) {
    if (a.x == b.x) {
        slope_ = std::numeric_limits<double>::max();
    } else {
        slope_ = (b.y - a.y) / (b.x - a.x);
    }
}
    
double Line::GetSlope() const {
    return slope_;
}
    
Coordinate Line::GetCoordinate() const {
    return {a_, b_};
}
