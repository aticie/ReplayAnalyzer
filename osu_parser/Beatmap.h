#pragma once
#ifndef osu_parser_beatmap_h
#define osu_parser_beatmap_h

#include "hitobject.h"
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

class Beatmap {
public:
	Beatmap(std::string osu_file_path, bool is_hardrock);
	bool is_initialized = false;
	std::vector<Hitobject> hitobjects;
	float circle_size;
	float overall_diff;
};

#endif // !osu_parser_beatmap_h
