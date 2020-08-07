#pragma once
#ifndef osu_parser_replay_h
#define osu_parser_replay_h

#include <fstream>
#include <vector>
#include <filesystem>
#include "replay_frame.h"

class Replay {
public:
	bool is_initialized = false;
	Replay(std::string replay_path);
	static std::string read_osu_string(std::ifstream& replay_file);
	std::string get_beatmap_md5();
	void write_osu_string(std::ofstream& replay_file, std::string);
	void set_replay_frames(std::vector<ReplayFrame> frames);
	void save_to_file(std::string);
	bool is_hardrock();
	std::vector<ReplayFrame>* get_replay_frames();
	~Replay();
private:
	std::vector<ReplayFrame> frames;
	uint8_t game_mode;
	int game_version;
	std::string beatmap_md5_hash;
	std::string	player_name;
	std::string	replay_md5_hash;
	short count_300;
	short count_100;
	short count_50;
	short count_geki;
	short count_katu;
	short count_miss;
	int total_score;
	short greatest_combo;
	uint8_t perfect;
	int mods;
	std::string life_bar_graph;
	long long time_stamp;
	int compressed_data_length;
	char* compressed_data;
	long long online_score_id;
};


#endif // !define osu_parser_replay_h
