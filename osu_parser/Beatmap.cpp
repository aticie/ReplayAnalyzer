#include "Beatmap.h"

Beatmap::Beatmap(std::string osu_file_path, bool is_hardrock) {
	if (!std::filesystem::exists(osu_file_path))
	{
		is_initialized = false;
		return;
	}
	is_initialized = true;
	std::ifstream osu_file(osu_file_path);

	for (std::string line; std::getline(osu_file, line); )
	{
		if (line == "[Difficulty]") {
			for (std::string inner_line; std::getline(osu_file, inner_line); )
			{
				if (inner_line == "") 
					break;

				if (inner_line.find("CircleSize") != std::string::npos) {
					circle_size = std::stof(inner_line.substr(11, inner_line.size()));
				}
				
				if (inner_line.find("OverallDifficulty") != std::string::npos) {
					overall_diff = std::stof(inner_line.substr(18, inner_line.size()));
				}

			}
		}
		if (line == "[HitObjects]")
			break;
	}
	int hitobj_count = 1;
	for (std::string line; std::getline(osu_file, line); )
	{
		if (line == "") {
			break;
		}
		Hitobject hit_obj(line, is_hardrock);
		hit_obj.setCount(hitobj_count);
		hitobjects.push_back(hit_obj);
		hitobj_count++;
	}
}
