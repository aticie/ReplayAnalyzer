#pragma once
#ifndef osu_parser_hitobject_h
#define osu_parser_hitobject_h

#include <sstream>
#include <vector>
#include "../utils/string_utils.h"
#include <SFML/Graphics.hpp>

class Hitobject{
public:
	Hitobject(std::string &a, bool is_hardrock);
	int getTime();
	void setCount(int);
	int getCount();
	int getType();
	float getLength();
	sf::Vector2f getPos();
	sf::VertexArray curve_points;
	~Hitobject();

private:
	int x;
	int y;
	int time;
	int type; // 0: Hit circle, 1: Slider, 3 : Spinner
	int count;
	std::string curve_type;
	int slides;
	float length;
};
#endif // !osu_parser_hitobject_h
