#pragma once
#ifndef osu_utils_replay_frame_h
#define osu_utils_replay_frame_h

#include <inttypes.h>
#include <SFML/System/Vector2.hpp>

class ReplayFrame {

private:
	int64_t abs_time;
	int64_t rel_time;
	sf::Vector2f mouse_pos;
	int key_state;

public:

	void setRelTime(int64_t time);
	int64_t getRelTime();
	// Getters and setters
	void setAbsTime(int64_t time);
	int64_t getAbsTime();

	void setMousePos(sf::Vector2f mouse_pos);
	sf::Vector2f getMousePos();

	void setKeyState(int key_state);
	int getKeyState();

	// Constructor
	ReplayFrame();
	ReplayFrame(int64_t abs_time, int64_t rel_time, sf::Vector2f mouse_pos, int key_state);

	// Destructor
	~ReplayFrame();
};

#endif // !ReplayParser.h
