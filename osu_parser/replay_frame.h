#pragma once
#ifndef OSU_UTILS_REPLAY_FRAME_H
#define OSU_UTILS_REPLAY_FRAME_H

#include <cinttypes>
#include <SFML/System/Vector2.hpp>

class replay_frame
{
	int64_t abs_time_;
	int64_t rel_time_;
	sf::Vector2f mouse_pos_;
	int key_state_;

public:
	// Getters and setters
	void set_rel_time(int64_t time);
	[[nodiscard]] int64_t get_rel_time() const;
	
	void set_abs_time(int64_t time);
	[[nodiscard]] int64_t get_abs_time() const;

	void set_mouse_pos(sf::Vector2f mouse_pos);
	[[nodiscard]] sf::Vector2f get_mouse_pos() const;

	void set_key_state(int key_state);
	[[nodiscard]] int get_key_state() const;

	// Constructor
	replay_frame();
	replay_frame(int64_t rel_time, int64_t abs_time, sf::Vector2f mouse_pos, int key_state);

	// Destructor
	~replay_frame() = default;
};

#endif // !ReplayParser.h
