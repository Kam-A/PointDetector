#pragma once

#include <deque>
#include <vector>

#include "basket.h"

class Detector {
public:
    Detector() = default;
    void AddLine(Line line);
    void SetBasketNumber(int number);
    std::vector<Point> Process();
    double GetTotalDistance();
    std::vector<Point> GetPoints();
    
private:
    void InitStateDistributionBySlope();
    void InitStateDistributionByRandomPoints();
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
    void ClearAllBasket();
    int GetBasketIndexByMinDistance(Line line, std::vector<Point>& points);
    void InitStateByKmeans();
    std::vector<Point> FindCenterBySmallDistanceIntersection();
    std::vector<Basket> baskets_;
    std::vector<std::vector<int>> idx_to_change_;
    std::deque<Line> lines_;
    std::mt19937 generator_;
};
