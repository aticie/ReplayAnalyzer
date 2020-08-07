#include "hitobject.h"

Hitobject::Hitobject(std::string& beatmap_line, bool is_hardrock) {
	std::string delim = ",";
	std::vector<std::string> parsed_objects = splitString(beatmap_line, delim);
	x = stoi(parsed_objects[0]);  // 0 - x 

	if (!is_hardrock) 
		y = stoi(parsed_objects[1]);  // 1 - y
	else 
		y = 384 - stoi(parsed_objects[1]);

	time = stoi(parsed_objects[2]);  // 2 - time
	type = stoi(parsed_objects[3]);  // 3 - type
	if ((type & 1) == 1) { // Hit circle
		return;
	}
	else if ((type & 2) == 2) { // Slider
		std::string slider_string = parsed_objects[5]; // 5 - Curve points
		slides = stoi(parsed_objects[6]); // 6 - slides
		length = stof(parsed_objects[7]); // 7 - slides
		std::string slider_delim = "|";
		std::vector<std::string> slider_objects = splitString(slider_string, slider_delim);
		curve_type = slider_objects[0];
		slider_objects.erase(slider_objects.begin());
		curve_points.append(sf::Vertex({ (float)x, (float)y }));
		for (auto slider_pt : slider_objects) {
			std::string slider_pt_delim = ":";
			std::vector<std::string> slider_points = splitString(slider_pt, slider_pt_delim);
			if (!is_hardrock)
				curve_points.append(sf::Vertex({ stof(slider_points[0]), stof(slider_points[1]) }));
			else
				curve_points.append(sf::Vertex({ stof(slider_points[0]), 384 - stof(slider_points[1]) }));
		}
	}
	else if ((type & 8) == 8) { // Spinner
		return;
	}
}

float Hitobject::getLength() {
	return length;
}

void Hitobject::setCount(int c) {
	count = c;
}

int Hitobject::getCount() {
	return count;
}

Hitobject::~Hitobject() {

}

int Hitobject::getTime() {
	return time;
}

int Hitobject::getType() {
	return type;
}

sf::Vector2f Hitobject::getPos() {
	sf::Vector2f pos(x, y);
	return pos;
}