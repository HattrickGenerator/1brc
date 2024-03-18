#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <sstream>
#include <map>

struct Temperature{
    double min{100};
    double max{-100};
    double acc{0};
    size_t cnt{0};
};

void parseLine(const std::string& line, std::map<std::string, Temperature>& weather_stations){
    std::string segment;
    std::vector<std::string> segments;
    std::stringstream test(line);

    while(std::getline(test, segment, ';'))
    {
        segments.push_back(segment);
    }

    double value = strtod(segments[1].c_str(), NULL);

    Temperature& tmps = weather_stations[segments[0]];
    if(value < tmps.min){
        tmps.min = value;
    }
    if(value > tmps.max){
        tmps.max = value;
    }
    tmps.acc += value;
    tmps.cnt += 1;
}

std::ifstream getOpenedFile(){
    std::ifstream myfile;
    myfile.open("measurements.txt");

    if(!myfile.is_open()) {
        perror("Error open");
        exit(EXIT_FAILURE);
    }
    return myfile;
}

template<typename WeatherStations> 
void print_formatted(const WeatherStations & weather_stations){

    std::cout << std::fixed;
    std::cout << std::setprecision(1);

    std::cout << "{";
    
    for( auto it = weather_stations.begin(); it != weather_stations.end(); ++it){
        const auto & key = it->first;
        const auto & val = it->second;


        double cnt = val.cnt;
        double mean = std::ceil(std::abs(val.acc * 10)/cnt);
        std::string_view sign = val.acc < 0 ? "-" : "";

        std::cout << key << "=" << val.min << "/"  <<  sign << mean/10 << "/" << val.max;

        if(it != --weather_stations.end()){
            std::cout << ", ";
        
        }
    }
    std::cout << "}";
}

int main(){

    std::map<std::string, Temperature> weather_stations;
    std::ifstream myfile = getOpenedFile();

    std::string line;
    while(getline(myfile, line)) {
        parseLine(line, weather_stations);
    }


    print_formatted(weather_stations);
}
