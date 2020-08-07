#include "draw_helpers.h"

sf::RectangleShape draw_line_from_two_points(const sf::Vector2f pt1, const sf::Vector2f pt2)
{
	const auto delta_x = pt2.x - pt1.x;
	const auto delta_y = pt2.y - pt1.y;
	const float theta_angle = atan2(delta_y, delta_x) * 180.f / pi;

	const auto length = vector_distance(pt1, pt2);

	sf::RectangleShape line(sf::Vector2f(length, 2));
	line.setPosition(pt1);
	line.setRotation(theta_angle);

	return line;
}
