#pragma once
#ifndef OSU_PARSER_REPLAY_UTILS_H
#define OSU_PARSER_REPLAY_UTILS_H

#include <vector>
#include "replay.h"
#include "replay_frame.h"
#include "../utils/math_helpers.h"

void parse_replay_raw_content(const std::string& replay_contents, replay& replay);
std::string get_raw_replay_data_from_osrp(const std::string& file_path);
int get_frame_index(std::vector<replay_frame> v, replay_frame k);
std::vector<replay_frame> get_frames_subarray(std::vector<replay_frame> frames,
                                           int frames_index, int trail_length);
int find_nearest_frame(const std::vector<replay_frame>& frames_subarray,
                     sf::Vector2f mouse_local_position);
std::string serialize_replay_frames_to_raw_text(std::vector<replay_frame> frames);
std::vector<replay_frame> get_frames_subarray_btw_time(std::vector<replay_frame> frames,
                                                  int time_start, int time_finish);
//std::vector<ReplayFrame> parseReplayFromOsr(std::string selected_replay);
#endif
