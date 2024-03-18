#include <string>
#include <vector>
#include <fstream>

#include <iostream>
#include <iomanip>

#include <stdlib.h>
#include <cmath>
#include <sstream>

#include <unordered_map>
#include <map>

struct Temperature{
    double min{100};
    double max{-100};
    double acc{0};
    size_t cnt{0};
};

int parseInt(std::string_view view){
    int val = view[view.size() - 1];

    int comma = view.size() -2;

    //two digits
    if (view.size() == 5){
        val = -(view[1] *100 + view[2] * 10 + view[4]);
    } //one digit negative
    else if(view[0] == '-'){
        val = -(view[1] *10 + view[3]);
    }
    else if(view.size() == 4){
        val = (view[0] *100 + view[1] * 10 + view[3]);
    }
    else if(view.size() == 3){
        val = (view[0] *10 + view[2]);
    }
    return val;
}

void parseLine(const std::string& line, std::unordered_map<std::string, Temperature>& weather_stations){
    std::string segment;
    std::vector<std::string> segments;
    std::stringstream test(line);

    while(std::getline(test, segment, ';'))
    {
        segments.push_back(segment);
    }

    //int value = parseInt(segments[1]);
    double value = std::stod(segments[1]);

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

    std::unordered_map<std::string, Temperature> weather_stations;
    std::ifstream myfile = getOpenedFile();

    std::string line;
    while(getline(myfile, line)) {
        parseLine(line, weather_stations);
    }

    std::map<std::string, Temperature> weather_stations_sorted;
    for(auto & [key, val] : weather_stations){
        weather_stations_sorted.emplace(key, val);
    }

    print_formatted(weather_stations_sorted);
}
