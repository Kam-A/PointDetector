//
//  line.cpp
//  point_detector
//
//  Created by user on 28.05.2022.
//

#include "line.h"

#include <array>
#include <optional>
#include <set>


#include <xtensor/xarray.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xview.hpp>
#include <xtensor/xtensor.hpp>
#include "xtensor-blas/xlinalg.hpp"

#define MAX_AXYIS_VALUE 1000
#define MIN_AXYIS_VALUE -1000

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

std::vector<Point> GenerateRandomPoints(std::mt19937& generator, int count) {
    std::vector<Point> points;
    std::uniform_real_distribution<> distrib(MIN_AXYIS_VALUE, MAX_AXYIS_VALUE);
    while(points.size() < count) {
        points.push_back({distrib(generator), distrib(generator)});
    }
    return points;
}


void CombinationUtil(std::vector<std::vector<int>>& ans, std::vector<int>& tmp, int n, int left, int k) {
    if (k == 0) {
        ans.push_back(tmp);
        return;
    }
    for (int i = left; i <= n; ++i)
    {
        tmp.push_back(i);
        CombinationUtil(ans, tmp, n, i + 1, k - 1);
        tmp.pop_back();
    }
}
 
std::vector<std::vector<int>> Combination(int n, int k) {
    std::vector<std::vector<int>> ans;
    std::vector<int> tmp;
    CombinationUtil(ans, tmp, n, 1, k);
    return ans;
}


std::pair<Point, std::map<const Line*, double>>  GetIntersections(std::set<const Line *> lines) {
    std::vector<Point> a_coord(lines.size());
    std::vector<Point> b_coord(lines.size());
    std::vector<Point> a_b_diff(lines.size());
    std::vector<std::pair<Point, Point>> proj(lines.size());
    std::vector<std::pair<double, double>> R(2);
    std::pair<double, double> q{0, 0};
    int idx = 0;
    for(const Line* a : lines) {
        Coordinate line_coord = a->GetCoordinate();
        a_coord[idx] = line_coord.a;
        b_coord[idx] = line_coord.b;
        ++idx;
    }
    for (int i = 0; i < lines.size(); ++i) {
        a_b_diff[i] = b_coord[i] - a_coord[i];
    }
    for (int i = 0; i < lines.size(); ++i) {
        double norm = std::pow(std::pow(a_b_diff[i].x, 2) + std::pow(a_b_diff[i].y, 2), 0.5);
        a_b_diff[i].x /= norm;
        a_b_diff[i].y /= norm;
    }
    
    for (int i = 0; i < lines.size(); ++i) {
        proj[i].first.x = 1 - a_b_diff[i].x * a_b_diff[i].x;
        proj[i].first.y =  - a_b_diff[i].x * a_b_diff[i].y;
        proj[i].second.x =  - a_b_diff[i].x * a_b_diff[i].y;
        proj[i].second.y = 1 - a_b_diff[i].y * a_b_diff[i].y;
    }
    for (int i = 0; i < lines.size(); ++i) {
        R[0].first += proj[i].first.x;
        R[0].second += proj[i].first.y;
        R[1].first +=proj[i].second.x;
        R[1].second +=proj[i].second.y;
    }
    for (int i = 0; i < lines.size(); ++i) {
        q.first += proj[i].first.x * a_coord[i].x + proj[i].first.y * a_coord[i].y;
        q.second += proj[i].second.x * a_coord[i].x + proj[i].second.y * a_coord[i].y;
    }
    
    xt::xarray<double>::shape_type R_shape = {2, 2};
    xt::xarray<double>::shape_type q_shape = {2, 1};
    xt::xarray<double> a0(R_shape);
    xt::xarray<double> a1(q_shape);
    a0(0, 0) = R[0].first;
    a0(0, 1) = R[0].second;
    a0(1, 0) = R[1].first;
    a0(1, 1) = R[1].second;
    a1(0, 0) = q.first;
    a1(1, 0) = q.second;
    auto res = xt::linalg::lstsq(a0, a1);
    Point res_point{std::get<0>(res)(0,0), std::get<0>(res)(1,0)};
    std::vector<std::pair<double, double>> A(lines.size());
    for (int i = 0; i < lines.size(); ++i) {
        A[i].first = a_coord[i].x - res_point.x;
        A[i].second = a_coord[i].y - res_point.y;
    }
    std::vector<std::pair<double, double>> A_dot_proj(lines.size());
    for (int i = 0; i < lines.size(); ++i) {
        A_dot_proj[i].first = A[i].first * proj[i].first.x + A[i].second * proj[i].second.x;
        A_dot_proj[i].second = A[i].first * proj[i].first.y + A[i].second * proj[i].second.y;
    }
    std::map<const Line*, double> result;
    idx = 0;
    for(const Line* a : lines) {
        result[a] = A_dot_proj[idx].first * A[idx].first + A_dot_proj[idx].second * A[idx].second;
        ++idx;
    }
    return {res_point, result};
}

std::optional<Point> GetIntersection(Line left, Line right) {
    Coordinate a_coordinate = left.GetCoordinate();
    Coordinate b_coordinate = right.GetCoordinate();
    double a = a_coordinate.b.y - a_coordinate.a.y;
    double b = a_coordinate.a.x - a_coordinate.b.x;
    double c = a * a_coordinate.a.x + b * a_coordinate.a.y;
    double a1 = b_coordinate.b.y - b_coordinate.a.y;
    double b1 = b_coordinate.a.x - b_coordinate.b.x;
    double c1 = a1 * b_coordinate.a.x + b1 * b_coordinate.a.y;
    double det = a*b1 - a1*b;
    if (det == 0) {
        return {};
    } else {
       double x = (b1*c - b*c1)/det;
       double y = (a*c1 - a1*c)/det;
        if (x > 1000 || x < -1000 || y > 1000 || y < -1000) {
            return {};
        }
        return Point{x, y};
    }
}

std::vector<std::array<double, 2>> GetAllIntersction(const std::deque<Line>& lines) {
    std::vector<std::array<double, 2>> res;
    for (const auto& idxs : Combination(lines.size(), 2)) {
        std::optional<Point> intersect_point = GetIntersection(lines[idxs[0] - 1], lines[idxs[1] - 1]);
        if (intersect_point) {
            res.push_back({intersect_point.value().x , intersect_point.value().y});
        }
    }
    return  res;
}

std::map<double, int> GetDistanceFromLineToPoints(Line line, std::vector<Point> points) {
    Coordinate line_coord = line.GetCoordinate();
    Point a_b_diff  = line_coord.b - line_coord.a;
    double norm = std::pow(std::pow(a_b_diff.x, 2) + std::pow(a_b_diff.y, 2), 0.5);
    a_b_diff.x /= norm;
    a_b_diff.y /= norm;
    std::pair<Point, Point> proj;
    proj.first.x = 1 - a_b_diff.x * a_b_diff.x;
    proj.first.y =  - a_b_diff.x * a_b_diff.y;
    proj.second.x =  - a_b_diff.x * a_b_diff.y;
    proj.second.y = 1 - a_b_diff.y * a_b_diff.y;
    std::vector<std::pair<double, double>> A(points.size());
    for (int i = 0; i < points.size(); ++i) {
        A[i].first = line_coord.a.x - points[i].x;
        A[i].second = line_coord.a.y - points[i].y;
    }
    std::vector<std::pair<double, double>> A_dot_proj(points.size());
    for (int i = 0; i < points.size(); ++i) {
        A_dot_proj[i].first = A[i].first * proj.first.x + A[i].second * proj.second.x;
        A_dot_proj[i].second = A[i].first * proj.first.y + A[i].second * proj.second.y;
    }
    std::map<double, int> result;
    for (int i = 0; i < points.size(); ++i) {
        result[A_dot_proj[i].first * A[i].first + A_dot_proj[i].second * A[i].second] = i;
    }
    return result;
}

std::map<const Line*, double>  GetDistanceFromPointToLines(Point res_point, std::vector<Line>& lines) {
    std::vector<Point> a_coord(lines.size());
    std::vector<Point> b_coord(lines.size());
    std::vector<Point> a_b_diff(lines.size());
    std::vector<std::pair<Point, Point>> proj(lines.size());
    int idx = 0;
    for(Line a : lines) {
        Coordinate line_coord = a.GetCoordinate();
        a_coord[idx] = line_coord.a;
        b_coord[idx] = line_coord.b;
        ++idx;
    }
    for (int i = 0; i < lines.size(); ++i) {
        a_b_diff[i] = b_coord[i] - a_coord[i];
    }
    for (int i = 0; i < lines.size(); ++i) {
        double norm = std::pow(std::pow(a_b_diff[i].x, 2) + std::pow(a_b_diff[i].y, 2), 0.5);
        a_b_diff[i].x /= norm;
        a_b_diff[i].y /= norm;
    }
    for (int i = 0; i < lines.size(); ++i) {
        proj[i].first.x = 1 - a_b_diff[i].x * a_b_diff[i].x;
        proj[i].first.y =  - a_b_diff[i].x * a_b_diff[i].y;
        proj[i].second.x =  - a_b_diff[i].x * a_b_diff[i].y;
        proj[i].second.y = 1 - a_b_diff[i].y * a_b_diff[i].y;
    }
    std::vector<std::pair<double, double>> A(lines.size());
    for (int i = 0; i < lines.size(); ++i) {
        A[i].first = a_coord[i].x - res_point.x;
        A[i].second = a_coord[i].y - res_point.y;
    }
    std::vector<std::pair<double, double>> A_dot_proj(lines.size());
    for (int i = 0; i < lines.size(); ++i) {
        A_dot_proj[i].first = A[i].first * proj[i].first.x + A[i].second * proj[i].second.x;
        A_dot_proj[i].second = A[i].first * proj[i].first.y + A[i].second * proj[i].second.y;
    }
    std::map<const Line*, double> result;
    idx = 0;
    for(Line a : lines) {
        result[&a] = A_dot_proj[idx].first * A[idx].first + A_dot_proj[idx].second * A[idx].second;
        ++idx;
    }
    return result;
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
