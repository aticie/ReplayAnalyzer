#include "hitobject.h"

#include "../utils/string_utils.h"

hitobject::hitobject(std::string& beatmap_line, const bool is_hardrock): count_(0), slides_(0)
{
	const std::string delim = ",";
	auto parsed_objects = splitString(beatmap_line, delim);
	x_ = stoi(parsed_objects[0]); // 0 - x 

	if (!is_hardrock)
		y_ = stoi(parsed_objects[1]); // 1 - y
	else
		y_ = 384 - stoi(parsed_objects[1]);

	time_ = static_cast<int64_t>(stoi(parsed_objects[2])); // 2 - time
	type_ = stoi(parsed_objects[3]); // 3 - type
	if ((type_ & 1) == 1)
	{
		// Hit circle
		return;
	}
	if ((type_ & 2) == 2)
	{
		// Slider
		const auto slider_string = parsed_objects[5]; // 5 - Curve points
		slides_ = stoi(parsed_objects[6]); // 6 - slides
		length_ = stof(parsed_objects[7]); // 7 - slides
		const std::string slider_delim = "|";
		auto slider_objects = splitString(slider_string, slider_delim);
		curve_type_ = slider_objects[0];
		slider_objects.erase(slider_objects.begin());
		curve_points.append(sf::Vertex({static_cast<float>(x_), static_cast<float>(y_)}));
		for (const auto& slider_pt : slider_objects)
		{
			std::string slider_pt_delim = ":";
			auto slider_points = splitString(slider_pt, slider_pt_delim);
			if (!is_hardrock)
				curve_points.append(sf::Vertex({stof(slider_points[0]), stof(slider_points[1])}));
			else
				curve_points.append(sf::Vertex({stof(slider_points[0]), 384 - stof(slider_points[1])}));
		}
	}
	else if ((type_ & 8) == 8)
	{
		// Spinner
		return;
	}
}

float hitobject::get_length() const
{
	return length_;
}

auto hitobject::get_color() const -> sf::Color
{
	return color_;
}

void hitobject::set_count(const int c)
{
	count_ = c;
}

auto hitobject::set_color_by_hit_time(const hit_event event, const float od) -> void
{
	const auto hit_diff = abs(this->get_time() - event.hit_time);
	if (hit_diff < (160 - 12 * od) / 2.f)
		color_ = sf::Color(255, 255, 255, 155);
	else if(hit_diff < (280 - 16 * od) / 2.f)
		color_ = sf::Color(3, 255, 58, 155);
	else if(hit_diff < (400 - 20 * od) / 2.f)
		color_ = sf::Color(183, 212, 13, 155);
	else
		color_ = sf::Color(255, 0, 0, 155);
	
}

int hitobject::get_count() const
{
	return count_;
}

hitobject::~hitobject()
= default;

int64_t hitobject::get_time() const
{
	return time_;
}

int hitobject::get_type() const
{
	return type_;
}

sf::Vector2f hitobject::get_pos() const
{
	const sf::Vector2f pos(x_, y_);
	return pos;
}
