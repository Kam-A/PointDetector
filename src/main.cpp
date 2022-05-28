
#include <cmath>
#include <iostream>
#include <fstream>

#include "detector.h"
#include "input_reader.h"

int main(int argc, char* argv[])
{
    Detector detector;
    std::ifstream input_file;
    input_file.open("1-lines.txt");
    std::vector<Point> points = RequestPoint(detector, input_file);
    for (const Point p : points) {
        std::cout << p.x << " " << p.y << std::endl;
    }
    return 0;
}

