
#include <cmath>
#include <iostream>
#include <fstream>

#include "detector.h"
#include "input_reader.h"

int main(int argc, char* argv[])
{
    Detector detector;
    std::ifstream input_file;
    std::ofstream output_file;
    input_file.open(argv[1]);
    output_file.open(argv[2]);
    int a = 5;
    std::vector<Point> points = RequestPoint(detector, input_file);
    for (const Point p : points) {
        std::cout << p.x << " " << p.y << std::endl;
        output_file << p.x << " " << p.y << std::endl;
    }
    return 0;
}

