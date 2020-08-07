#include "replay_frame.h"

ReplayFrame::ReplayFrame() {
	abs_time = NULL;
	rel_time = NULL;
	mouse_pos = sf::Vector2f(0.f, 0.f);
	key_state = 0;
}

ReplayFrame::ReplayFrame(int64_t rel_t, int64_t abs_t, sf::Vector2f mp, int ks) {
	abs_time = abs_t;
	rel_time = rel_t;
	mouse_pos = mp;
	key_state = ks;
}

void ReplayFrame::setMousePos(sf::Vector2f mp) {
	mouse_pos = mp;
}
sf::Vector2f ReplayFrame::getMousePos() {
	return mouse_pos;
}

void ReplayFrame::setKeyState(int keys) {
	key_state = keys;
}
int ReplayFrame::getKeyState() {
	return key_state;
}

int64_t ReplayFrame::getRelTime() {
	return rel_time;
}
void ReplayFrame::setRelTime(int64_t t) {
	rel_time = t;
}

int64_t ReplayFrame::getAbsTime() {
	return abs_time;
}
void ReplayFrame::setAbsTime(int64_t t) {
	abs_time = t;
}

ReplayFrame::~ReplayFrame() {

}