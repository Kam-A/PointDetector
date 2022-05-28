#pragma once

#include <deque>
#include <vector>

#include "basket.h"

class Detector {
public:
    Detector() = default;
    void AddLine(Line line);
    void SetBasketNumber(int number);
    void Process();
    double GetTotalDistance();
    std::vector<Point> GetPoints();
    
private:
    void InitStateDistribution();
    void PermutateWithoutChangeSize();
    void PermutateWithChangeSize();
    void RandomPermutation();
    void RandomPermutationToDecrease();
    void SwapFurthest();
    void SwapFurthestToDecrease();
    void SwapClosest();
    void SwapClosestToDecrease();
    void SwapClosestForFurthest();
    void SwapClosestForFurthestToDecrease();
    void ChangeSizeBasketAndShuffle();
    bool CheckSize(int l_idx, int r_idx);
    bool IsPointClose();
    
    std::vector<Basket> baskets_;
    std::deque<Line> lines_;
    std::mt19937 generator_;
};
