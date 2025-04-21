#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <iomanip>

namespace fs = std::filesystem;

// Convert Kelvin to Fahrenheit
double kelvin_to_fahrenheit(double kelvin) {
    return (kelvin - 273.15) * 9.0 / 5.0 + 32.0;
}

// Check if point is within US bounding box
bool in_us_bounds(double lat, double lon) {
    return (lat >= 24.25 && lat <= 49.25) &&
           (lon >= -125.00 && lon <= -67.00);
}

int main() {
    double total = 0.0;
    int count = 0;

    for (const auto& entry : fs::directory_iterator(".")) {
        std::string filename = entry.path().filename().string();
        if (filename.rfind("master_", 0) == 0 && filename.size() >= 4 && filename.substr(filename.size() - 4) == ".csv") {
	    std::ifstream file(entry.path());
            std::string line;
            getline(file, line); // skip header

            while (getline(file, line)) {
                std::stringstream ss(line);
                double lat, lon, pop, tempK;
                char comma;

                ss >> lat >> comma >> lon >> comma >> pop >> comma >> tempK;

                if (!in_us_bounds(lat, lon)) continue;

                double tempF = kelvin_to_fahrenheit(tempK);
                double diff = tempF - 65.0;

                total += diff * pop;
                count++;
            }
        }
    }

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "15 Day, Population-Weighted Degree Day: " << total << "\n";
    return 0;
}

