#include "basket.h"

#include <cmath>
#include <vector>
#include <xtensor/xarray.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xview.hpp>
#include <xtensor/xtensor.hpp>
#include "xtensor-blas/xlinalg.hpp"

std::pair<Point, std::map<const Line*, double>>  Basket::GetIntersections() const {
    std::vector<Point> a_coord(lines_.size());
    std::vector<Point> b_coord(lines_.size());
    std::vector<Point> a_b_diff(lines_.size());
    std::vector<std::pair<Point, Point>> proj(lines_.size());
    std::vector<std::pair<double, double>> R(2);
    std::pair<double, double> q{0, 0};
    int idx = 0;
    for(const Line* a : lines_) {
        Coordinate line_coord = a->GetCoordinate();
        a_coord[idx] = line_coord.a;
        b_coord[idx] = line_coord.b;
        ++idx;
    }
    for (int i = 0; i < lines_.size(); ++i) {
        a_b_diff[i] = b_coord[i] - a_coord[i];
    }
    for (int i = 0; i < lines_.size(); ++i) {
        double norm = std::pow(std::pow(a_b_diff[i].x, 2) + std::pow(a_b_diff[i].y, 2), 0.5);
        a_b_diff[i].x /= norm;
        a_b_diff[i].y /= norm;
    }
    
    for (int i = 0; i < lines_.size(); ++i) {
        proj[i].first.x = 1 - a_b_diff[i].x * a_b_diff[i].x;
        proj[i].first.y =  - a_b_diff[i].x * a_b_diff[i].y;
        proj[i].second.x =  - a_b_diff[i].x * a_b_diff[i].y;
        proj[i].second.y = 1 - a_b_diff[i].y * a_b_diff[i].y;
    }
    for (int i = 0; i < lines_.size(); ++i) {
        R[0].first += proj[i].first.x;
        R[0].second += proj[i].first.y;
        R[1].first +=proj[i].second.x;
        R[1].second +=proj[i].second.y;
    }
    for (int i = 0; i < lines_.size(); ++i) {
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
    std::vector<std::pair<double, double>> A(lines_.size());
    for (int i = 0; i < lines_.size(); ++i) {
        A[i].first = a_coord[i].x - res_point.x;
        A[i].second = a_coord[i].y - res_point.y;
    }
    std::vector<std::pair<double, double>> A_dot_proj(lines_.size());
    for (int i = 0; i < lines_.size(); ++i) {
        A_dot_proj[i].first = A[i].first * proj[i].first.x + A[i].second * proj[i].second.x;
        A_dot_proj[i].second = A[i].first * proj[i].first.y + A[i].second * proj[i].second.y;
    }
    std::map<const Line*, double> result;
    idx = 0;
    for(const Line* a : lines_) {
        result[a] = A_dot_proj[idx].first * A[idx].first + A_dot_proj[idx].second * A[idx].second;
        ++idx;
    }
    return {res_point, result};
}

Basket::Basket() : center_({0,0}) {
}

void Basket::AddLine(const Line* line) {
    lines_.insert(line);
}

void Basket::DeleteLine(const Line* line) {
    lines_.erase(line);
}

const Line* Basket::DeleteFurthestLine() {
    std::pair<Point, std::map<const Line*, double>> point_dist = GetIntersections();
    center_ = point_dist.first;
    auto max_it = std::max_element(point_dist.second.begin(),
                                   point_dist.second.end(),
                                   [] (const auto& a, auto& b)->bool{ return a.second < b.second; } );
    const Line* deleted_line = max_it->first;
    lines_.erase(max_it->first);
    return deleted_line;
    
}

const Line* Basket::DeleteClosestLine() {
    std::pair<Point, std::map<const Line*, double>> point_dist = GetIntersections();
    center_ = point_dist.first;
    auto min_it = std::min_element(point_dist.second.begin(),
                                   point_dist.second.end(),
                                   [] (const auto& a, auto& b)->bool{ return a.second < b.second; } );
    const Line* deleted_line = min_it->first;
    lines_.erase(min_it->first);
    return deleted_line;
}

std::pair<const Line*,const Line*> Basket::SwapFurthestLine(Basket& rhs) {
    const Line* lhs_furthest = DeleteFurthestLine();
    const Line* rhs_furthest = rhs.DeleteFurthestLine();
    AddLine(rhs_furthest);
    rhs.AddLine(lhs_furthest);
    return {lhs_furthest, rhs_furthest};
}

bool Basket::SwapFurthestLineDecrease(Basket& rhs) {
    double init_dist_sum = GetDistancesSum() + rhs.GetDistancesSum();
    std::pair<const Line*,const Line*> changed_ptr = SwapFurthestLine(rhs);
    if (GetDistancesSum() + rhs.GetDistancesSum() > init_dist_sum) {
        DeleteLine(changed_ptr.second);
        AddLine(changed_ptr.first);
        rhs.DeleteLine(changed_ptr.first);
        rhs.AddLine(changed_ptr.second);
        return false;
    } else {
        return true;
    }
}

std::pair<const Line*,const Line*> Basket::SwapClosestLine(Basket& rhs) {
    const Line* lhs_closest = DeleteClosestLine();
    const Line* rhs_closest = rhs.DeleteClosestLine();
    AddLine(rhs_closest);
    rhs.AddLine(lhs_closest);
    return {lhs_closest, rhs_closest};
}

bool Basket::SwapClosestLineDecrease(Basket& rhs) {
    double init_dist_sum = GetDistancesSum() + rhs.GetDistancesSum();
    std::pair<const Line*,const Line*> changed_ptr = SwapClosestLine(rhs);
    if (GetDistancesSum() + rhs.GetDistancesSum() > init_dist_sum) {
        DeleteLine(changed_ptr.second);
        AddLine(changed_ptr.first);
        rhs.DeleteLine(changed_ptr.first);
        rhs.AddLine(changed_ptr.second);
        GetDistancesSum();
        rhs.GetDistancesSum();
        return false;
    } else {
        return true;
    }
}

std::pair<const Line*,const Line*> Basket::ChangeFurthestToClosest(Basket& rhs) {
    const Line* lhs_furthest = DeleteFurthestLine();
    const Line* rhs_closest = rhs.DeleteClosestLine();
    AddLine(rhs_closest);
    rhs.AddLine(lhs_furthest);
    return {lhs_furthest, rhs_closest};
}

bool Basket::ChangeFurthestToClosestDecrease(Basket& rhs) {
    double init_dist_sum = GetDistancesSum() + rhs.GetDistancesSum();
    std::pair<const Line*,const Line*> changed_ptr = ChangeFurthestToClosest(rhs);
    if (GetDistancesSum() + rhs.GetDistancesSum() > init_dist_sum) {
        DeleteLine(changed_ptr.second);
        AddLine(changed_ptr.first);
        rhs.DeleteLine(changed_ptr.first);
        rhs.AddLine(changed_ptr.second);
        GetDistancesSum();
        rhs.GetDistancesSum();
        return false;
    } else {
        return true;
    }
}

std::vector<std::pair<const Line*,const Line*>> Basket::SwapRandom(Basket& rhs, int swap_number) {
    if (rhs.GetLineNumber() < swap_number || GetLineNumber() < swap_number) {
        return {};
    }
    std::vector<std::pair<const Line*,const Line*>> swaped_pointers;
    std::vector<int> lhs_idx = GenerateNumbers(generator_, swap_number, GetLineNumber() - 1);
    std::vector<int> rhs_idx = GenerateNumbers(generator_, swap_number, rhs.GetLineNumber() - 1);
    std::vector<const Line*> lhs_ptr;
    std::vector<const Line*> rhs_ptr;
    for(int i = 0; i < swap_number; ++i) {
        auto lhs_it = lines_.begin();
        std::advance(lhs_it, lhs_idx[i]);
        lhs_ptr.push_back(*lhs_it);
        auto rhs_it = rhs.lines_.begin();
        std::advance(rhs_it, rhs_idx[i]);
        rhs_ptr.push_back(*rhs_it);
        
    }
    for(int i = 0; i < swap_number; ++i) {
        DeleteLine(lhs_ptr[i]);
        AddLine(rhs_ptr[i]);
        rhs.DeleteLine(rhs_ptr[i]);
        rhs.AddLine(lhs_ptr[i]);
        swaped_pointers.push_back({lhs_ptr[i], rhs_ptr[i]});
    }
    return swaped_pointers;
}

bool Basket::SwapRandomDecrease(Basket& rhs, int swap_number) {
    double init_dist_sum = GetDistancesSum() + rhs.GetDistancesSum();
    std::vector<std::pair<const Line*,const Line*>> swapped_ptr = SwapRandom(rhs, swap_number);
    if (GetDistancesSum() + rhs.GetDistancesSum() > init_dist_sum) {
        for (const auto changed_ptr : swapped_ptr) {
            DeleteLine(changed_ptr.second);
            AddLine(changed_ptr.first);
            rhs.DeleteLine(changed_ptr.first);
            rhs.AddLine(changed_ptr.second);
        }
        return false;
    } else {
        return true;
    }
}

bool Basket::SwapRandomSmallInc(Basket& rhs, int swap_number) {
    double init_dist_sum = GetDistancesSum() + rhs.GetDistancesSum();
    std::vector<std::pair<const Line*,const Line*>> swapped_ptr = SwapRandom(rhs, swap_number);
    if (GetDistancesSum() + rhs.GetDistancesSum() > 10 * init_dist_sum) {
        for (const auto changed_ptr : swapped_ptr) {
            DeleteLine(changed_ptr.second);
            AddLine(changed_ptr.first);
            rhs.DeleteLine(changed_ptr.first);
            rhs.AddLine(changed_ptr.second);
        }
        return false;
    } else {
        return true;
    }
}

double Basket::GetDistancesSum() {
    std::pair<Point, std::map<const Line*, double>> point_dist = GetIntersections();
    double total_sum = std::accumulate(point_dist.second.begin(), point_dist.second.end(), 0.0,
                                       [] (auto prev_sum, auto &entry) {
                                           return prev_sum + entry.second;
                                       });
    return total_sum;
}

int Basket::GetLineNumber() const {
    return lines_.size();
}

Point Basket::GetCenter() {
    std::pair<Point, std::map<const Line*, double>> point_dist = GetIntersections();
    center_ = point_dist.first;
    return center_;
}

std::set<const Line*> Basket::GetLines() const {
    return lines_;
}
