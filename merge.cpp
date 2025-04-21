#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <filesystem>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem;

struct LatLon {
    double lat, lon;
    bool operator==(const LatLon &other) const {
        return lat == other.lat && lon == other.lon;
    }
};

namespace std {
    template <>
    struct hash<LatLon> {
        size_t operator()(const LatLon& k) const {
            return hash<double>()(k.lat) ^ (hash<double>()(k.lon) << 1);
        }
    };
}

std::string get_today_date() {
    auto now = std::chrono::system_clock::now();
    std::time_t t_now = std::chrono::system_clock::to_time_t(now);
    std::tm *ptm = std::localtime(&t_now);
    char buffer[16];
    std::strftime(buffer, 16, "%m-%d-%y", ptm);
    return std::string(buffer);
}

int main(int argc, char* argv[]) {
    std::string date = (argc > 1) ? argv[1] : get_today_date();
    std::string folder = "./" + date;
    std::string popFilePath = "population_2020.csv";

    if (!fs::exists(folder)) {
        std::cerr << "❌ Directory does not exist: " << folder << "\n";
        return 1;
    }

    std::unordered_map<LatLon, double> populationMap;
    std::ifstream popFile(popFilePath);
    if (!popFile.is_open()) {
        std::cerr << "❌ Could not open population file: " << popFilePath << "\n";
        return 1;
    }

    std::string line;
    getline(popFile, line); // skip header
    while (getline(popFile, line)) {
        std::stringstream ss(line);
        double lat, lon, pop;
        char comma;
        ss >> lat >> comma >> lon >> comma >> pop;
        populationMap[{lat, lon}] = pop;
    }

    std::ofstream out("master_" + date + ".csv");
    out << "forecast_time,latitude,longitude,population,temp_2m\n";

    for (const auto& entry : fs::directory_iterator(folder)) {
        const std::string filename = entry.path().filename().string();
        if (filename.rfind(date.substr(0, 2) + "_" + date.substr(3, 2) + "_" + date.substr(6, 2), 0) == 0) {
            std::ifstream forecastFile(entry.path());
            std::string row;
            getline(forecastFile, row); // skip header
            while (getline(forecastFile, row)) {
                std::stringstream ss(row);
                std::string timestamp;
                double lat, lon, temp;
                char comma;

                getline(ss, timestamp, ',');
                ss >> lat >> comma >> lon >> comma >> temp;

                LatLon key{lat, lon};
                if (populationMap.count(key)) {
                    out << std::fixed << std::setprecision(6)
                        << timestamp << "," << lat << "," << lon << "," << static_cast<int>(populationMap[key]) << "," << temp << "\n";
                }
            }
        }
    }

    std::cout << "✅ Done. Output saved to master_" << date << ".csv\n";
    return 0;
}

