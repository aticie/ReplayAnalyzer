#include "replay_frame.h"

replay_frame::replay_frame()
{
	abs_time_ = NULL;
	rel_time_ = NULL;
	mouse_pos_ = sf::Vector2f(0.f, 0.f);
	key_state_ = 0;
}

replay_frame::replay_frame(const int64_t rel_time, const int64_t abs_time, const sf::Vector2f mouse_pos, const int key_state)
{
	abs_time_ = abs_time;
	rel_time_ = rel_time;
	mouse_pos_ = mouse_pos;
	key_state_ = key_state;
}

void replay_frame::set_mouse_pos(const sf::Vector2f mouse_pos)
{
	mouse_pos_ = mouse_pos;
}

auto replay_frame::get_mouse_pos() const -> sf::Vector2f
{
	return mouse_pos_;
}

void replay_frame::set_key_state(const int key_state)
{
	key_state_ = key_state;
}

int replay_frame::get_key_state() const
{
	return key_state_;
}

int64_t replay_frame::get_rel_time() const
{
	return rel_time_;
}

void replay_frame::set_rel_time(const int64_t time)
{
	rel_time_ = time;
}

int64_t replay_frame::get_abs_time() const
{
	return abs_time_;
}

void replay_frame::set_abs_time(const int64_t time)
{
	abs_time_ = time;
}


