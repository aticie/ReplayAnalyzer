#pragma once
#ifndef OSU_PARSER_OSU_DATABASE_H
#define OSU_PARSER_OSU_DATABASE_H

#include <unordered_map>
#include "replay.h"
#include <string>
#include <filesystem>

class beatmap_info
{
public:
	beatmap_info();
	[[nodiscard]] std::string get_hash() const;
	std::string foldername;
	std::string filename;
	std::string md5;
};

class osu_database
{
public:
	explicit osu_database(const std::wstring&);
	std::unordered_map<std::string, beatmap_info> beatmap_mapping;
	bool is_initialized = false;
private:
	int osu_version_{};
	int folder_count_{};
	bool account_unlocked_{};
	long long date_to_unlock_{};
	std::string player_name_;
	int nr_beatmaps_{};
};

beatmap_info parse_beatmap(std::ifstream&);

#endif // !osu_parser_osu_db_parser_h
