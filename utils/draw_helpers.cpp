#include "draw_helpers.h"

sf::RectangleShape drawLineFromTwoPoints(sf::Vector2f pt1, sf::Vector2f pt2) {
	const float delta_x = pt2.x - pt1.x;
	const float delta_y = pt2.y - pt1.y;
	const float theta_angle = atan2(delta_y, delta_x) * 180 / PI;

	const float length = vectorDistance(pt1, pt2);

	sf::RectangleShape line(sf::Vector2f(length, 2));
	line.setPosition(pt1);
	line.setRotation(theta_angle);

	return line;
}