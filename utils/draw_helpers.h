#pragma once
#ifndef UTILS_DRAW_HELPERS_H
#define UTILS_DRAW_HELPERS_H

const auto pi = 3.141592653589793238462;

#include "math_helpers.h"
#include <SFML/Graphics.hpp>
sf::RectangleShape draw_line_from_two_points(sf::Vector2f pt1, sf::Vector2f pt2);

#endif // !define utils_draw_helpers_h
