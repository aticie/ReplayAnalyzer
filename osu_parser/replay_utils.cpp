#include "replay_utils.h"
#include "replay.h"

std::string getRawReplayDataFromOSRP(std::string file_path) {
	std::ifstream replayFile(file_path);
	Replay replay();
	if (replayFile) {
		std::string contents;
		replayFile.seekg(0, std::ios::end);
		contents.resize(replayFile.tellg());
		replayFile.seekg(0, std::ios::beg);
		replayFile.read(&contents[0], contents.size());
		replayFile.close();
		return contents;
	}
	return "0|256|192|0";
}

std::string serializeReplayFramesToRawText(std::vector<ReplayFrame> frames) {
	std::stringstream replay_contents_ss;
	replay_contents_ss << frames[0].getRelTime() << "|256|-500|0";
	frames.erase(frames.begin());
	for (auto frame : frames) {
		replay_contents_ss << "," << frame.getRelTime() << "|" << frame.getMousePos().x << "|" << frame.getMousePos().y << "|" << frame.getKeyState();
	}
	return replay_contents_ss.str();
}

std::vector<ReplayFrame> parseReplayRawContent(std::string replay_contents) {
	std::vector<ReplayFrame> replay_frames;
	std::stringstream replay_contents_ss(replay_contents);
	std::string str;
	std::string delimiter = "|";
	ReplayFrame rng_frame;

	int64_t abs_time = 0;

	int skip_first_two = 0;
	while (std::getline(replay_contents_ss, str, ',')) {
		std::vector<std::string> str_vec = splitString(str, delimiter);

		if (str_vec.size() < 4) break; // If we're reading garbage, break

		if (stod(str_vec[0]) == -12345) {
			rng_frame = ReplayFrame(stoi(str_vec[0]), -12345, sf::Vector2f(stof(str_vec[1]), stof(str_vec[2])), stoi(str_vec[3])); // Rng frame, save it at the end
			replay_frames.push_back(ReplayFrame(stoi(str_vec[0]), abs_time,
				sf::Vector2f(stof(str_vec[1]), stof(str_vec[2])),
				stoi(str_vec[3])));
		}
		else
			abs_time += stoi(str_vec[0]);

		replay_frames.push_back(ReplayFrame(stoi(str_vec[0]), abs_time,
			sf::Vector2f(stof(str_vec[1]), stof(str_vec[2])),
			stoi(str_vec[3])));
	}

	return replay_frames;
}


//  Finds the nearest frame to the mouse position on screeen
int findNearestFrame(std::vector<ReplayFrame> frames_subarray,
	sf::Vector2f mouseLocalPosition) {
	float min_dist = 8.f;
	int selected_frame_idx = -1;
	for (auto frame : frames_subarray) {
		float dist = vectorDistance(frame.getMousePos(), mouseLocalPosition);
		if (dist < min_dist) {
			min_dist = dist;
			selected_frame_idx =
				getFrameIndex(frames_subarray, frame);
		}
	}
	return selected_frame_idx;
}
std::vector<ReplayFrame> getFramesSubarray(std::vector<ReplayFrame> frames,
	int frames_index, int trail_length) {
	auto frames_subbegin =
		std::clamp(frames_index - trail_length, 0, (int)frames.size() - 1);
	auto frames_subend =
		std::clamp(frames_index + trail_length, 0, (int)frames.size() - 1);

	std::vector<ReplayFrame> frames_subarray(&frames[frames_subbegin],
		&frames[frames_subend]);
	return frames_subarray;
}

std::vector<ReplayFrame> getFramesSubarrayBtwTime(std::vector<ReplayFrame> frames,
	int time_start, int time_finish) {
	
	auto time_begin =
		std::clamp(time_start, 0, (int)frames[frames.size() - 1].getAbsTime());
	auto time_end =
		std::clamp(time_finish, 0, (int)frames[frames.size() - 1].getAbsTime());

	auto begin_frame_idx = binarySearch(frames, frames[0].getAbsTime(), frames[frames.size() - 1].getAbsTime(), time_begin);
	auto end_frame_idx = binarySearch(frames, frames[0].getAbsTime(), frames[frames.size() - 1].getAbsTime(), time_begin);
	std::vector<ReplayFrame> frames_subarray(&frames[begin_frame_idx],
		&frames[end_frame_idx]);
	return frames_subarray;
}


int getFrameIndex(std::vector<ReplayFrame> v, ReplayFrame K) {
	auto it = find_if(v.begin(), v.end(), [&K](ReplayFrame& obj) {
		return obj.getMousePos() == K.getMousePos();
	});

	// If element was found
	if (it != v.end()) {
		// calculating the index
		// of K
		int index = distance(v.begin(), it);
		return index;
	}
	else return 0;
}