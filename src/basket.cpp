#include "basket.h"

#include <cmath>
#include <vector>

Basket::Basket() : center_({0,0}) {
}

void Basket::AddLine(const Line* line) {
    lines_.insert(line);
}

void Basket::DeleteLine(const Line* line) {
    lines_.erase(line);
}

const Line* Basket::DeleteFurthestLine() {
    std::pair<Point, std::map<const Line*, double>> point_dist = GetIntersections(lines_);
    center_ = point_dist.first;
    auto max_it = std::max_element(point_dist.second.begin(),
                                   point_dist.second.end(),
                                   [] (const auto& a, auto& b)->bool{ return a.second < b.second; } );
    const Line* deleted_line = max_it->first;
    lines_.erase(max_it->first);
    return deleted_line;
    
}

const Line* Basket::DeleteClosestLine() {
    std::pair<Point, std::map<const Line*, double>> point_dist = GetIntersections(lines_);
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
    if (GetDistancesSum() + rhs.GetDistancesSum() > 1.1 * init_dist_sum) {
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
    std::pair<Point, std::map<const Line*, double>> point_dist = GetIntersections(lines_);
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
    std::pair<Point, std::map<const Line*, double>> point_dist = GetIntersections(lines_);
    center_ = point_dist.first;
    return center_;
}
void Basket::Clear() {
    lines_.clear();
}
const std::set<const Line*>& Basket::GetLines() const {
    return lines_;
}
