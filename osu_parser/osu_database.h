#pragma once
#ifndef osu_parser_osu_database_h
#define osu_parser_osu_database_h

#include <unordered_map>
#include "replay.h"
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

class BeatmapInfo {
public:
	BeatmapInfo();
	std::string get_hash();
	std::string foldername;
	std::string filename;
	std::string md5;
};

class OsuDatabase {
public:
	OsuDatabase(std::wstring);
	std::unordered_map<std::string, BeatmapInfo> beatmap_mapping;
	bool is_initialized = false;
private:
	int osu_version;
	int folder_count;
	bool account_unlocked;
	long long date_to_unlock;
	std::string player_name;
	int nr_beatmaps;
	int user_permissions;
};

BeatmapInfo parse_beatmap(std::ifstream&);

#endif // !osu_parser_osu_db_parser_h
