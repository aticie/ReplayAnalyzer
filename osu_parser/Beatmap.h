#pragma once
#ifndef OSU_PARSER_BEATMAP_H
#define OSU_PARSER_BEATMAP_H

#include "hitobject.h"
#include <string>
#include <vector>
#include <filesystem>

class beatmap
{
public:
	beatmap(const std::string& osu_file_path, bool is_hardrock);
	bool is_initialized = false;
	std::vector<hitobject> hitobjects;
	float circle_size;
	float overall_diff;
};

#endif // !osu_parser_beatmap_h
