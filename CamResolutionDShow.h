#pragma once
#include <vector>
#include <map>


#define BLUE    0x0001
#define GREEN   0x0002
#define RED     0x0004
#define GRAY    0x0007

void setcolor(unsigned int color);
std::map<std::string, std::vector<std::pair<int, int>>> enum_devices();
