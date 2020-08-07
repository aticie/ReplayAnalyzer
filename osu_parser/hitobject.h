#pragma once
#ifndef OSU_PARSER_HITOBJECT_H
#define OSU_PARSER_HITOBJECT_H

#include <SFML/Graphics.hpp>

class hitobject
{
public:
	hitobject(std::string& beatmap_line, bool is_hardrock);

	[[nodiscard]] auto get_time() const -> int64_t;
	[[nodiscard]] int get_count() const;
	[[nodiscard]] int get_type() const;
	[[nodiscard]] auto get_length() const -> float;
	[[nodiscard]] sf::Vector2f get_pos() const;

	auto set_count(int) -> void;
	
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
};

#endif // !osu_parser_hitobject_h
