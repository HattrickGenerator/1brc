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

constexpr size_t BUFFER_SIZE = 1024*1024;
constexpr size_t MAX_LINE_SIZE = 150;
static_assert(BUFFER_SIZE > MAX_LINE_SIZE, "Buffer has to be bigger than line length. Otherwise behavior is undefined");

using WeatherStations = std::unordered_map<std::string, Temperature>;

//Trims files with more than one \n in the end.
//Takes the original buffer lenght as an input
template<typename Buffer>
static size_t getTrimmedBufferLen(size_t len, const Buffer & buffer ){
    size_t s = len;
    len += 1;
    if(len > 0 && buffer[len - 1] != '\n'){ //No line break
        return s;
    }

    for(int i = s - 1; i >= 0; --i){
        if(buffer[i] == '\n'){
            len = s-1;
        }
        else{
            break;
        }
    }

    return len;
}

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

void parseLine(std::string_view line, WeatherStations & weather_stations){
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


//compile time visitor pattern for lambda
using Buffer = std::array<char,BUFFER_SIZE + 1>;
using Line = std::array<char, MAX_LINE_SIZE>;


//Returns the index of the line
struct Positions{
    size_t buffer_index;
    size_t line_length;
};

static Positions copyPartialBeginningToBuffer(const Buffer & buffer, size_t buffer_length, Line & line, size_t line_length){

    //Nothing to copy
    if(line_length == 0){
        return {0,0};
    }

    std::string_view buffer_string(buffer.data(), BUFFER_SIZE);
    size_t i = 0;
    for(i = 0; i < buffer_length; ++i){
        if(buffer_string[i] == '\n'){
            auto begin = buffer.begin();
            auto end = buffer.begin() + i;
            std::copy(begin, end, line.begin() + line_length);
            line_length += std::distance(begin, end);
            break;
        }
    }
    return {i + 1,line_length};
}

static void parseBuffer(const Buffer & buffer, size_t len, WeatherStations& weather_stations){

    static Line line{'\0'}; // This buffer is only used if chunks are not aligned with line breaks
    static size_t line_length = 0;
    std::string_view buffer_string(buffer.data(), BUFFER_SIZE);

    //Buffer is so big that this only happens once in the beginning.
    auto positions = copyPartialBeginningToBuffer(buffer, len, line, line_length);
    if(positions.line_length > 0){
        parseLine(std::string_view(line.data(), positions.line_length), weather_stations);
        line_length = 0;
        line[0] = '\0';
    }

    size_t i = positions.buffer_index;
    size_t buffer_offset = i;
    for(; i < len; ++i){
        if(buffer_string[i] == '\n'){
            parseLine(buffer_string.substr(buffer_offset, i - buffer_offset), weather_stations);
            buffer_offset = i + 1;
        }
    }

    if(buffer_offset != i){
        //copy the end into the buffer
        std::copy(buffer.begin() + buffer_offset, buffer.end(), line.begin());
        line_length += std::distance(buffer.begin() + buffer_offset, buffer.end()) - 1;
    }
}

void parseBufferWise(std::ifstream & ifile, WeatherStations & weather_stations){

    Buffer buffer; //reads only the first ${bufferSize} bytes

    while(1)
    {
        ifile.read(buffer.data(), BUFFER_SIZE);
        std::streamsize s = ((ifile) ? BUFFER_SIZE : ifile.gcount());

        buffer[s] = 0;
        size_t len = s;
        if(!ifile){ //Trim line breaks in the end of the file so exactly one is left
            len = getTrimmedBufferLen(len, buffer);
        }

        parseBuffer(buffer, len, weather_stations);

        if(!ifile) break;
    }
    ifile.close();
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

    WeatherStations weather_stations;
    std::ifstream myfile = getOpenedFile();

    parseBufferWise(myfile, weather_stations);

    std::map<std::string, Temperature> weather_stations_sorted;
    for(auto & [key, val] : weather_stations){
        weather_stations_sorted.emplace(key, val);
    }

    print_formatted(weather_stations_sorted);
}
