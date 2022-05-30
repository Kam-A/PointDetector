//
//  input_reader.cpp
//  point_detector
//
//  Created by user on 26.05.2022.
//

#include "input_reader.h"

std::vector<Point> RequestPoint(Detector& detector, std::istream& input) {
    int number_line = 0;
    int number_point = 0;
    input >> number_line;
    input >> number_point;
    detector.SetBasketNumber(number_point);
    for(int i = 0; i < number_line; ++i) {
        Point begin;
        Point end;
        input >> begin.x >> begin.y;
        input >> end.x >> end.y;
        detector.AddLine({begin, end});
    }
    return detector.Process();
}
