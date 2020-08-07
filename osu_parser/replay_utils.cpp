#include "replay_utils.h"
#include "replay.h"
#include <sstream>

auto get_raw_replay_data_from_osrp(const std::string& file_path) -> std::string
{
	std::ifstream replay_file(file_path);
	replay replay{};
	if (replay_file)
	{
		std::string contents;
		replay_file.seekg(0, std::ios::end);
		contents.resize(replay_file.tellg());
		replay_file.seekg(0, std::ios::beg);
		replay_file.read(&contents[0], contents.size());
		replay_file.close();
		return contents;
	}
	return "0|256|192|0";
}

std::string serialize_replay_frames_to_raw_text(std::vector<replay_frame> frames)
{
	std::stringstream replay_contents_ss;
	replay_contents_ss << frames[0].get_rel_time() << "|256|-500|0";
	frames.erase(frames.begin());
	for (auto frame : frames)
	{
		replay_contents_ss << "," << frame.get_rel_time() << "|" << frame.get_mouse_pos().x << "|" << frame.get_mouse_pos().y
			<< "|" << frame.get_key_state();
	}
	return replay_contents_ss.str();
}

std::vector<replay_frame> parse_replay_raw_content(const std::string& replay_contents)
{
	std::vector<replay_frame> replay_frames;
	std::stringstream replay_contents_ss(replay_contents);
	std::string str;
	const std::string delimiter = "|";

	replay_frame rng_frame;
	int64_t abs_time = 0;

	while (std::getline(replay_contents_ss, str, ','))
	{
		auto str_vec = splitString(str, delimiter);

		if (str_vec.size() < 4) break; // If we're reading garbage, break

		if (stod(str_vec[0]) == -12345)
		{
			rng_frame = replay_frame(stoi(str_vec[0]), -12345, sf::Vector2f(stof(str_vec[1]), stof(str_vec[2])),
			                             stoi(str_vec[3])); // Rng frame, save it at the end
			replay_frames.emplace_back(stoi(str_vec[0]), abs_time,
			                           sf::Vector2f(stof(str_vec[1]), stof(str_vec[2])),
			                           stoi(str_vec[3]));
		}
		else
			abs_time += stoi(str_vec[0]);

		replay_frames.emplace_back(stoi(str_vec[0]), abs_time,
		                           sf::Vector2f(stof(str_vec[1]), stof(str_vec[2])),
		                           stoi(str_vec[3]));
	}

	replay_frames.push_back(rng_frame);

	return replay_frames;
}


//  Finds the nearest frame to the mouse position on screeen
int find_nearest_frame(const std::vector<replay_frame>& frames_subarray,
                       const sf::Vector2f mouse_local_position)
{
	auto min_dist = 8.f;
	auto selected_frame_idx = -1;
	for (auto frame : frames_subarray)
	{
		const auto dist = vector_distance(frame.get_mouse_pos(), mouse_local_position);
		if (dist < min_dist)
		{
			min_dist = dist;
			selected_frame_idx =
				get_frame_index(frames_subarray, frame);
		}
	}
	return selected_frame_idx;
}

std::vector<replay_frame> get_frames_subarray(std::vector<replay_frame> frames,
                                             const int frames_index, const int trail_length)
{
	const auto frames_subbegin =
		std::clamp(frames_index - trail_length, 0, static_cast<int>(frames.size()) - 1);
	const auto frames_subend =
		std::clamp(frames_index + trail_length, 0, static_cast<int>(frames.size()) - 1);

	std::vector<replay_frame> frames_subarray(&frames[frames_subbegin],
	                                         &frames[frames_subend]);
	return frames_subarray;
}

auto get_frames_subarray_btw_time(std::vector<replay_frame> frames,
                              const int time_start, const int time_finish) -> std::vector<replay_frame>
{
	const auto time_begin =
		std::clamp(time_start, 0, static_cast<int>(frames[frames.size() - 1].get_abs_time()));
	const auto time_end =
		std::clamp(time_finish, 0, static_cast<int>(frames[frames.size() - 1].get_abs_time()));

	const auto begin_frame_idx = binary_search(frames, frames[0].get_abs_time(), frames[frames.size() - 1].get_abs_time(),
	                                           time_begin);
	const auto end_frame_idx = binary_search(frames, frames[0].get_abs_time(), frames[frames.size() - 1].get_abs_time(),
	                                         time_end);
	std::vector<replay_frame> frames_subarray(&frames[begin_frame_idx],
	                                         &frames[end_frame_idx]);
	return frames_subarray;
}


int get_frame_index(std::vector<replay_frame> v, replay_frame k)
{
	const auto it = find_if(v.begin(), v.end(), [&k](replay_frame& obj)
	{
		return obj.get_mouse_pos() == k.get_mouse_pos();
	});

	// If element was found
	if (it != v.end())
	{
		// calculating the index
		// of K
		const auto index = distance(v.begin(), it);
		return index;
	}
	return 0;
}
