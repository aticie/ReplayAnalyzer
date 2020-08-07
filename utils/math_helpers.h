#pragma once
#ifndef UTILS_MATH_HELPERS_H
#define UTILS_MATH_HELPERS_H

#include <SFML/System/Vector2.hpp>
#include <vector>
#include "../osu_parser/replay_frame.h"

float vector_distance(sf::Vector2f pt1, sf::Vector2f pt2);
int binary_search(std::vector<replay_frame> arr, int l, int r, int x);

#endif // !utils_math_helpers_h
