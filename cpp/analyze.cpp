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
#include <array>

static_assert(UINT32_MAX > 1000000000, "UINT32 is not sufficient as a count");
constexpr size_t max_rows = 1000000000;
constexpr size_t max_temperature_accumulate = max_rows * 100;
static_assert(INT64_MAX > max_temperature_accumulate, "INT64 is not sufficient for the accumulate");
struct Temperature{
    int16_t min{1000};
    int16_t max{-1000};
    uint32_t cnt{0};
    int64_t acc{0};
};

static inline int16_t numFromChar(char c ){
    return (c-'0');
}

constexpr static std::array<int16_t, 4> power10{1, 10, 100, 1000};
static int16_t parseInt(std::string_view view){

    if(view.size() == 1){
        return numFromChar(view[0]);
    }

    size_t size = view.size();
    size_t index = 0;
    int16_t val = 0;
    int16_t sign = 1;
    if(view[0] == '-'){
        sign *= -1;
        ++index;
    }

    for(; index < size - 2; ++index){
        val += numFromChar(view[index]) * power10[size-index - 2];
    }

    return sign * (val  + numFromChar(view[size-1]));
}

void parseLine(std::string_view line, std::unordered_map<std::string, Temperature>& weather_stations){
    std::string_view name;
    std::string_view value_str;
    for(size_t i = 1; i < line.size(); ++i){
        if(line[i] == ';'){
            name = line.substr(0, i);
            value_str = line.substr(i+1, line.size());
            break;
        }
    }

    int16_t value = parseInt(value_str);

    auto it = weather_stations.find(std::string(name));
    if(it == weather_stations.end()){
        weather_stations.emplace(name, Temperature{value, value, 1, value});
    }
    else{
        Temperature & tmps = it->second;
        if(value < tmps.min){
            tmps.min = value;
        }
        if(value > tmps.max){
            tmps.max = value;
        }
        tmps.acc += value;
        tmps.cnt += 1;
    }
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

        double mean = std::ceil(std::abs(val.acc)/double(val.cnt));
        std::string_view sign = val.acc < 0 ? "-" : "";

        std::cout << key << "=" << double(val.min)/10 << "/"  <<  sign << mean/10 << "/" << double( val.max)/10;

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
