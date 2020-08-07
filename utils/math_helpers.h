#pragma once
#ifndef utils_math_helpers_h
#define utils_math_helpers_h

#include <SFML/System/Vector2.hpp>
#include <vector>
#include "../osu_parser/replay_frame.h"

float vectorDistance(sf::Vector2f pt1, sf::Vector2f pt2);
int binarySearch(std::vector<ReplayFrame> arr, int l, int r, int x);

#endif // !utils_math_helpers_h
