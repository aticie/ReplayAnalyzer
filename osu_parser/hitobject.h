#pragma once
#ifndef OSU_PARSER_HITOBJECT_H
#define OSU_PARSER_HITOBJECT_H

#include <SFML/Graphics.hpp>
#include "replay.h"

class hitobject
{
public:
	hitobject(std::string& beatmap_line, bool is_hardrock);

	[[nodiscard]] auto get_time() const -> int64_t;
	[[nodiscard]] auto get_count() const -> int;
	[[nodiscard]] auto get_type() const -> int;
	[[nodiscard]] auto get_length() const -> float;
	[[nodiscard]] auto get_color() const -> sf::Color;
	[[nodiscard]] sf::Vector2f get_pos() const;

	auto set_count(int) -> void;
	auto set_color_by_hit_time(hit_event, float) -> void;
	
	sf::VertexArray curve_points;
	~hitobject();

private:
	int x_;
	int y_;
	int64_t time_;
	int type_; // 0: Hit circle, 1: Slider, 3 : Spinner
	int count_;
	std::string curve_type_;
	int slides_;
	float length_ = 0;
	sf::Color color_;
};

#endif // !osu_parser_hitobject_h
