#pragma once
#ifndef osu_parser_replay_utils_h
#define osu_parser_replay_utils_h

#include <vector>
#include <fstream>
#include "LzmaLib/LzmaLib.h"
#include "replay_frame.h"
#include <algorithm>
#include <sstream>
#include "../utils/string_utils.h"
#include "../utils/math_helpers.h"

std::vector<ReplayFrame> parseReplayRawContent(std::string replay_contents);
std::string getRawReplayDataFromOSRP(std::string file_path);
int getFrameIndex(std::vector<ReplayFrame> v, ReplayFrame K);
std::vector<ReplayFrame> getFramesSubarray(std::vector<ReplayFrame> frames,
	int frames_index, int trail_length);
int findNearestFrame(std::vector<ReplayFrame> frames_subarray,
	sf::Vector2f mouseLocalPosition);
std::string serializeReplayFramesToRawText(std::vector<ReplayFrame> frames);
std::vector<ReplayFrame> getFramesSubarrayBtwTime(std::vector<ReplayFrame> frames,
	int time_start, int time_finish);
//std::vector<ReplayFrame> parseReplayFromOsr(std::string selected_replay);
#endif