#pragma once
#ifndef OSU_PARSER_REPLAY_H
#define OSU_PARSER_REPLAY_H

#include <fstream>
#include <vector>
#include <filesystem>
#include "replay_frame.h"

struct hit_event
{
	int64_t hit_time;
	int key_no;
	hit_event(const int64_t abs_time, const int key_num)
		: hit_time(abs_time), key_no(key_num){}
};

class replay
{
public:
	std::vector<replay_frame> frames;
	std::vector<hit_event> hit_events;
	
	explicit replay(const std::string& replay_path);
	explicit replay();

	replay(const replay&);
	replay& operator=(const replay&);
	
	bool is_initialized = false;
	static auto read_osu_string(std::ifstream& replay_file) -> std::string;
	[[nodiscard]] auto get_beatmap_md5() const -> std::string;
	static auto write_osu_string(std::ofstream& replay_file, const std::string&) -> void;
	auto save_to_file(const std::string&) -> void;
	[[nodiscard]] auto is_hardrock() const -> bool;
	auto get_replay_frames() -> std::vector<replay_frame>*;
	
	~replay();
private:
	uint8_t game_mode_{};
	int game_version_{};
	std::string beatmap_md5_hash_;
	std::string player_name_;
	std::string replay_md5_hash_;
	short count_300_{};
	short count_100_{};
	short count_50_{};
	short count_geki_{};
	short count_katu_{};
	short count_miss_{};
	int total_score_{};
	short greatest_combo_{};
	uint8_t perfect_{};
	int mods_{};
	std::string life_bar_graph_;
	long long time_stamp_{};
	int compressed_data_length_{};
	char* compressed_data_{};
	long long online_score_id_{};
};

#endif // !define osu_parser_replay_h
