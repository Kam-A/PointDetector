//
//  detector.cpp
//  point_detector
//
//  Created by user on 26.05.2022.
//
#include <algorithm>
#include <iostream>
#include <cmath>
#include <utility>

#include "detector.h"
#include "dkm.h"


#define LINE_NUMBER_TO_RANDOM_DECREASE_SWAP 3
#define LINE_NUMBER_TO_RANDOM_SWAP 3
#define LOCAL_MINIMA_LIMIT 10
#define CONVERGENCE_FOR_INIT 100

void Detector::AddLine(Line line) {
    lines_.push_back(line);
}

void Detector::SetBasketNumber(int number) {
    baskets_.resize(number);
    idx_to_change_ = Combination(baskets_.size(), 2);
}
void Detector::ClearAllBasket() {
    for (Basket& b : baskets_) {
        b.Clear();
    }
}
bool IsPointInSquare(Point point) {
    if (point.x <= 1000 && point.x >= -1000 && point.y <= 1000 && point.y >= -1000) {
        return true;
    } else {
        return false;
    }
}
std::vector<Point> Detector::FindCenterBySmallDistanceIntersection() {
    std::vector<std::vector<int>> line_idx_to_chg = Combination(lines_.size(), 3);
    std::shuffle(line_idx_to_chg.begin(), line_idx_to_chg.end(), std::random_device());
    std::cout << "combination ready" << std::endl;
    std::vector<std::array<double, 2>> all_precise_intersection;
    int count = 0;
    for (auto& line_idx : line_idx_to_chg) {
        if (all_precise_intersection.size() > 10'000) {
            std::cout << "full points container" << std::endl;
            break;
        }
        if (count % 1000 == 0) {
            std::cout << count << std::endl;
        }
        std::set<const Line*> line_to_check{&lines_[line_idx[0] - 1],
                                            &lines_[line_idx[1] - 1],
                                            &lines_[line_idx[2] - 1]};
        std::pair<Point, std::map<const Line*, double>> point_dist = GetIntersections(line_to_check);
        double three_sum_line = std::accumulate(point_dist.second.begin(), point_dist.second.end(),
                                                0.0,
                                                [](const std::size_t previous, auto& p)
                                                { return previous + p.second; });
        
        if (three_sum_line < 0.01 && IsPointInSquare(point_dist.first)) {
            all_precise_intersection.push_back({point_dist.first.x, point_dist.first.y});
        }
        ++count;
    }
    auto cluster_res = dkm::kmeans_lloyd(all_precise_intersection, baskets_.size());
    std::vector<Point> cluster_centers;
    for (auto p : std::get<0>(cluster_res)) {
        cluster_centers.push_back({p[0], p[1]});
    }
    return cluster_centers;
    
}

std::vector<Point> Detector::Process() {
    return FindCenterBySmallDistanceIntersection();
}

void Detector::InitStateByKmeans() {
    std::vector<std::array<double, 2>> intersections = GetAllIntersction(lines_);
    auto cluster_res = dkm::kmeans_lloyd(intersections, baskets_.size());
    std::vector<Point> cluster_centers;
    for (auto p : std::get<0>(cluster_res)) {
        cluster_centers.push_back({p[0], p[1]});
    }
        for(int i = 0; i < lines_.size(); ++i) {
        baskets_[GetBasketIndexByMinDistance(lines_[i], cluster_centers)].AddLine(&lines_[i]);
    }
}

void Detector::InitStateDistributionBySlope() {
    std::sort(lines_.begin(), lines_.end(),[](const auto& lhs, const auto& rhs) {
            return lhs.GetSlope() < rhs.GetSlope();
        });
    for(int i = 0; i < lines_.size(); ++i) {
        baskets_[i % baskets_.size()].AddLine(&lines_[i]);
    }
}

int Detector::GetBasketIndexByMinDistance(Line line, std::vector<Point>& points) {
    std::map<double, int> distance = GetDistanceFromLineToPoints(line, points);
    int line_per_basket = (lines_.size() / baskets_.size());
    for (auto dist : distance) {
        if (baskets_[dist.second].GetLineNumber() < line_per_basket) {
            return dist.second;
        }
    }
    return baskets_.size() - 1;
    
}

void Detector::InitStateDistributionByRandomPoints() {
    std::vector<Point> points = GenerateRandomPoints(generator_, baskets_.size());
    for(int i = 0; i < lines_.size(); ++i) {
        baskets_[GetBasketIndexByMinDistance(lines_[i], points)].AddLine(&lines_[i]);
    }
}

bool CheckLocalMinima(double prev_sum, double current_sum, int& same_val_count) {
    if (std::abs(prev_sum - current_sum) < 1) {
        ++same_val_count;
    } else {
        same_val_count = 0;
    }
    if (same_val_count == LOCAL_MINIMA_LIMIT) {
        same_val_count = 0;
        return true;
    } else {
        return false;
    }
}

void Detector::PermutateWithoutChangeSize() {
    double best_distance = GetTotalDistance();
    std::vector<Basket> best_composition = baskets_;
    double prev_sum = best_distance;
    int same_val_count = 0;
    int idx = 0;
    while (idx < 1000) {
        SwapFurthestToDecrease();
        RandomPermutationToDecrease();
        double current_sum = GetTotalDistance();
        if (idx % 50 == 0) {
            std::cout << "Current total sum: " << current_sum << std::endl;
        }
        if (current_sum < best_distance) {
            best_composition = baskets_;
            best_distance = current_sum;
        }
        if (current_sum < CONVERGENCE_FOR_INIT * baskets_.size()) {
            std::cout << "Convergence" << std::endl;
            std::cout << current_sum << std::endl;
            break;
        }
        if (CheckLocalMinima(prev_sum, current_sum, same_val_count)) {
            RandomPermutation();
        }
        prev_sum = current_sum;
        ++idx;
    }
    baskets_ = best_composition;
}

void Detector::PermutateWithChangeSize() {
    double best_distance = GetTotalDistance();
    double prev_sum = best_distance;
    std::vector<Basket> best_composition = baskets_;
    int same_val_count = 0;
    int idx = 0;
    while(idx < 50'000) {
        ChangeSizeBasketAndShuffle();
        double current_sum = GetTotalDistance();
        if (idx % 50 == 0) {
            std::cout << "Current total sum: " << current_sum << std::endl;
        }
        if (current_sum < best_distance) {
            best_composition = baskets_;
            best_distance = current_sum;
        }
        if (current_sum < (baskets_.size() * 5)) {
            std::cout << "Convergence" << std::endl;
            std::cout << current_sum << std::endl;
            break;
        }
        if (current_sum > 2.0e+08) {
            std::cout << "Big diff" << std::endl;
            baskets_ = best_composition;
            break;
        }
        if (CheckLocalMinima(prev_sum, current_sum, same_val_count)) {
            RandomPermutation();
        }
        prev_sum = current_sum;
        ++idx;
    }
    baskets_ = best_composition;
    std::cout << best_distance << std::endl;
}

double Detector::GetTotalDistance() {
    double total_sum = std::accumulate(baskets_.begin(), baskets_.end(), 0.0,
                                       [](auto prev_sum, auto &entry) {
                                           return prev_sum + entry.GetDistancesSum();
                                       });
    return total_sum;
}

std::vector<Point> Detector::GetPoints() {
    std::vector<Point> points;
    for(auto& basket : baskets_) {
        points.push_back(basket.GetCenter());
    }
    return points;
}

void Detector::RandomPermutation() {
    for (const auto& idxs : idx_to_change_) {
        baskets_[idxs[0] - 1].SwapRandom(baskets_[idxs[1]- 1], LINE_NUMBER_TO_RANDOM_SWAP);
    }
}

void Detector::RandomPermutationToDecrease() {
    for (const auto& idxs : idx_to_change_) {
        baskets_[idxs[0]- 1].SwapRandomDecrease(baskets_[idxs[1]- 1], LINE_NUMBER_TO_RANDOM_DECREASE_SWAP);
    }
}

void Detector::SwapFurthest() {
    for (const auto& idxs : idx_to_change_) {
        baskets_[idxs[0]- 1].SwapFurthestLine(baskets_[idxs[1]- 1]);
    }
}

void Detector::SwapFurthestToDecrease() {
    for (const auto& idxs : idx_to_change_) {
        baskets_[idxs[0]- 1].SwapFurthestLineDecrease(baskets_[idxs[1]- 1]);
    }
}

void Detector::SwapClosest() {
    for (const auto& idxs : idx_to_change_) {
        baskets_[idxs[0] - 1].SwapClosestLine(baskets_[idxs[1] - 1]);
    }
}

void Detector::SwapClosestToDecrease() {
    for (const auto& idxs : idx_to_change_) {
        baskets_[idxs[0] - 1].SwapClosestLineDecrease(baskets_[idxs[1] - 1]);
    }
}

void Detector::SwapClosestForFurthest() {
    for (const auto& idxs : idx_to_change_) {
        baskets_[idxs[0] - 1].ChangeFurthestToClosest(baskets_[idxs[1] - 1]);
    }
}

void Detector::SwapClosestForFurthestToDecrease() {
    for (const auto& idxs : idx_to_change_) {
        baskets_[idxs[0]- 1].ChangeFurthestToClosestDecrease(baskets_[idxs[1]- 1]);
    }
}

bool Detector::CheckSize(int l_idx, int r_idx) {
    int max_line = 0;
    int min_line = 200;
    for(int i = 0; i < baskets_.size(); ++i) {
        int check_val = baskets_[i].GetLineNumber();
        if (i == l_idx) {
            ++check_val;
        } else if(i == r_idx) {
            --check_val;
        }
        if (check_val > 200 || check_val < baskets_.size()) {
            return false;
        }
        if (check_val > max_line) {
            max_line = check_val;
        } else if (check_val < min_line) {
            min_line = check_val;
        }
    }
    if (max_line > 2 * min_line) {
        return false;
    }
    return true;
}

void Detector::ChangeSizeBasketAndShuffle() {
    int idx = 0;
    while(idx < 1000) {
        std::vector<int> idxs = GenerateNumbers(generator_, 2, baskets_.size() - 1);
        if (CheckSize(idxs[0], idxs[1])) {
            double init_dist_sum = baskets_[idxs[0]].GetDistancesSum() + baskets_[idxs[1]].GetDistancesSum();
            const Line* l = baskets_[idxs[0]].DeleteFurthestLine();
            baskets_[idxs[1]].AddLine(l);
            double end_dist_sum = baskets_[idxs[0]].GetDistancesSum() + baskets_[idxs[1]].GetDistancesSum();
            if (init_dist_sum > end_dist_sum) {
                break;
            }
            baskets_[idxs[1]].DeleteLine(l);
            baskets_[idxs[0]].AddLine(l);
        }
        ++idx;
    }
}

bool Detector::IsPointClose() {
    for (const auto& idxs : idx_to_change_) {
        Point center_l = baskets_[idxs[0] - 1].GetCenter();
        Point center_r = baskets_[idxs[1] - 1].GetCenter();
        if (sqrt(pow(center_l.x - center_r.x, 2) + pow(center_l.y - center_r.y, 2) * 1.0) < 300) {
            return true;
        }
    }
    return false;
}
