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




void Detector::AddLine(Line line) {
    lines_.push_back(line);
}

void Detector::SetBasketNumber(int number) {
    baskets_.resize(number);
}

void Detector::Process() {
    std::cout << "Init state" << std::endl;
    InitStateDistribution();
    std::cout << "Shuffle with cont basket size" << std::endl;
    PermutateWithoutChangeSize();
    std::cout << "Shuffle with change basket size" << std::endl;
    PermutateWithChangeSize();
}

bool CheckLocalMinima(double prev_sum, double current_sum, int& same_val_count) {
    if (std::abs(prev_sum - current_sum) < 1) {
        ++same_val_count;
    } else {
        same_val_count = 0;
    }
    if (same_val_count == 50) {
        same_val_count = 0;
        return true;
    } else {
        return false;
    }
}

void Detector::PermutateWithoutChangeSize() {
    double start_sum = GetTotalDistance();
    double prev_sum = start_sum;
    int same_val_count = 0;
    for(int i = 0; i < 4000; ++i) {
        SwapFurthestToDecrease();
        RandomPermutationToDecrease();
        double current_sum = GetTotalDistance();
        if (i % 50 == 0) {
            std::cout << "Current total sum: " << current_sum << std::endl;
        }
        if (current_sum * 15 * baskets_.size() < start_sum) {
            std::cout << "Convergence" << std::endl;
            std::cout << current_sum << std::endl;
            break;
        }
        if (CheckLocalMinima(prev_sum, current_sum, same_val_count)) {
            RandomPermutation();
        }
        if (i > 1000 && IsPointClose()) {
            RandomPermutation();
        }
        prev_sum = current_sum;
    }
}

void Detector::PermutateWithChangeSize() {
    double start_sum = GetTotalDistance();
    double prev_sum = start_sum;
    int same_val_count = 0;
    for(int i = 0; i < 10000; ++i) {
        ChangeSizeBasketAndShuffle();
        double current_sum = GetTotalDistance();
        if (i % 50 == 0) {
            std::cout << "Current total sum: " << current_sum << std::endl;
        }
        if (current_sum < (baskets_.size() * 5)) {
            std::cout << "Convergence" << std::endl;
            std::cout << current_sum << std::endl;
            break;
        }
        prev_sum = current_sum;
    }
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

void Detector::InitStateDistribution() {
    std::sort(lines_.begin(), lines_.end(),[](const auto& lhs, const auto& rhs) {
            return lhs.GetSlope() < rhs.GetSlope();
        });
    for(int i = 0; i < lines_.size(); ++i) {
        baskets_[i % baskets_.size()].AddLine(&lines_[i]);
    }
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

void Detector::RandomPermutation() {
    std::vector<std::vector<int>> idx_to_change = Combination(baskets_.size(), 2);
    for (const auto& idxs : idx_to_change) {
        baskets_[idxs[0] - 1].SwapRandom(baskets_[idxs[1]- 1], 3);
    }
}

void Detector::RandomPermutationToDecrease() {
    std::vector<std::vector<int>> idx_to_change = Combination(baskets_.size(), 2);
    for (const auto& idxs : idx_to_change) {
        baskets_[idxs[0]- 1].SwapRandomDecrease(baskets_[idxs[1]- 1], 3);
    }
}

void Detector::SwapFurthest() {
    std::vector<std::vector<int>> idx_to_change = Combination(baskets_.size(), 2);
    for (const auto& idxs : idx_to_change) {
        baskets_[idxs[0]- 1].SwapFurthestLine(baskets_[idxs[1]- 1]);
    }
}

void Detector::SwapFurthestToDecrease() {
    std::vector<std::vector<int>> idx_to_change = Combination(baskets_.size(), 2);
    for (const auto& idxs : idx_to_change) {
        baskets_[idxs[0]- 1].SwapFurthestLineDecrease(baskets_[idxs[1]- 1]);
    }
}

void Detector::SwapClosest() {
    std::vector<std::vector<int>> idx_to_change = Combination(baskets_.size(), 2);
    for (const auto& idxs : idx_to_change) {
        baskets_[idxs[0] - 1].SwapClosestLine(baskets_[idxs[1] - 1]);
    }
}

void Detector::SwapClosestToDecrease() {
    std::vector<std::vector<int>> idx_to_change = Combination(baskets_.size(), 2);
    for (const auto& idxs : idx_to_change) {
        baskets_[idxs[0] - 1].SwapClosestLineDecrease(baskets_[idxs[1] - 1]);
    }
}

void Detector::SwapClosestForFurthest() {
    std::vector<std::vector<int>> idx_to_change = Combination(baskets_.size(), 2);
    for (const auto& idxs : idx_to_change) {
        baskets_[idxs[0] - 1].ChangeFurthestToClosest(baskets_[idxs[1] - 1]);
    }
}

void Detector::SwapClosestForFurthestToDecrease() {
    std::vector<std::vector<int>> idx_to_change = Combination(baskets_.size(), 2);
    for (const auto& idxs : idx_to_change) {
        baskets_[idxs[0]- 1].ChangeFurthestToClosestDecrease(baskets_[idxs[1]- 1]);
    }
}

bool Detector::CheckSize(int l_idx, int r_idx) {
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
//    std::vector<std::vector<int>> idx_to_change = Combination(baskets_.size(), 2);
//    std::shuffle(idx_to_change.begin(), idx_to_change.end(), std::random_device());
//    for (const auto& idxs : idx_to_change) {
//        if (CheckSize(idxs[0] - 1, idxs[1] - 1)) {
//            double init_dist_sum = baskets_[idxs[0] - 1].GetDistancesSum() + baskets_[idxs[1] - 1].GetDistancesSum();
//            const Line* l = baskets_[idxs[0] - 1].DeleteFurthestLine();
//            baskets_[idxs[1] - 1].AddLine(l);
//            double dist_after_change = baskets_[idxs[0] - 1].GetDistancesSum() + baskets_[idxs[1] - 1].GetDistancesSum();
//            if (init_dist_sum < dist_after_change) {
//                baskets_[idxs[1] - 1].DeleteLine(l);
//                baskets_[idxs[0] - 1].AddLine(l);
//            }
//            break;
//        }
//    }
}

bool Detector::IsPointClose() {
    std::vector<std::vector<int>> idx_to_check = Combination(baskets_.size(), 2);
    for (const auto& idxs : idx_to_check) {
        Point center_l = baskets_[idxs[0] - 1].GetCenter();
        Point center_r = baskets_[idxs[1] - 1].GetCenter();
        if (sqrt(pow(center_l.x - center_r.x, 2) + pow(center_l.y - center_r.y, 2) * 1.0) < 100) {
            return true;
        }
    }
    return false;
}
