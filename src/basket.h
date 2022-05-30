#pragma once

#include <map>
#include <optional>
#include <set>

#include "line.h"

class Basket {
public:
    Basket();
    void AddLine(const Line* line);
    void DeleteLine(const Line* line);
    const Line* DeleteFurthestLine();
    const Line* DeleteClosestLine();
    std::pair<const Line*,const Line*> SwapFurthestLine(Basket& rhs);
    bool SwapFurthestLineDecrease(Basket& rhs);
    std::pair<const Line*,const Line*> SwapClosestLine(Basket& rhs);
    bool SwapClosestLineDecrease(Basket& rhs);
    std::pair<const Line*,const Line*> ChangeFurthestToClosest(Basket& rhs);
    bool ChangeFurthestToClosestDecrease(Basket& rhs);
    std::vector<std::pair<const Line*,const Line*>> SwapRandom(Basket& rhs, int swap_number);
    bool SwapRandomDecrease(Basket& rhs, int swap_number);
    bool SwapRandomSmallInc(Basket& rhs, int swap_number);
    double GetDistancesSum();
    int GetLineNumber() const;
    Point GetCenter();
    const std::set<const Line*>& GetLines() const;
    void Clear();
private:
    std::set<const Line*> lines_;
    Point center_;
    std::mt19937 generator_;
};
