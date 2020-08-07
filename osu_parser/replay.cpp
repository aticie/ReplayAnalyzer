#include "replay.h"
#include "replay_utils.h"
#include "../utils/LEB128.h"
#include "LZMA.h"
#include <cassert>
#include <utility>

auto replay::is_hardrock() const -> bool
{
	if ((mods_ & 16) == 16)
	{
		return true;
	}
	return false;
}

replay::replay(const std::string& replay_path)
{
	std::ifstream replay_file(replay_path, std::ios::binary);
	if (!replay_file.is_open())
	{
		is_initialized = false;
		replay_frame dummy(0, 0, sf::Vector2f(256, 256), 0);
		frames_.push_back(dummy);
		return;
	}
	is_initialized = true;
	replay_file.read(reinterpret_cast<char*>(&this->game_mode_), sizeof(unsigned char));
	replay_file.read(reinterpret_cast<char*>(&this->game_version_), sizeof(int));
	this->beatmap_md5_hash_ = read_osu_string(replay_file);
	this->player_name_ = read_osu_string(replay_file);
	this->replay_md5_hash_ = read_osu_string(replay_file);
	replay_file.read(reinterpret_cast<char*>(&this->count_300_), sizeof(short));
	replay_file.read(reinterpret_cast<char*>(&this->count_100_), sizeof(short));
	replay_file.read(reinterpret_cast<char*>(&this->count_50_), sizeof(short));
	replay_file.read(reinterpret_cast<char*>(&this->count_geki_), sizeof(short));
	replay_file.read(reinterpret_cast<char*>(&this->count_katu_), sizeof(short));
	replay_file.read(reinterpret_cast<char*>(&this->count_miss_), sizeof(short));
	replay_file.read(reinterpret_cast<char*>(&this->total_score_), sizeof(int));
	replay_file.read(reinterpret_cast<char*>(&this->greatest_combo_), sizeof(short));
	replay_file.read(reinterpret_cast<char*>(&this->perfect_), sizeof(unsigned char));
	replay_file.read(reinterpret_cast<char*>(&this->mods_), sizeof(int));
	this->life_bar_graph_ = read_osu_string(replay_file);
	replay_file.read(reinterpret_cast<char*>(&this->time_stamp_), sizeof(long long));
	replay_file.read(reinterpret_cast<char*>(&this->compressed_data_length_), sizeof(int));

	this->compressed_data_ = new char[compressed_data_length_];
	replay_file.read(compressed_data_, compressed_data_length_);
	replay_file.read(reinterpret_cast<char*>(&this->online_score_id_), sizeof(long long));

	// Read LZMA Stream and decompress
	lzma::data_stream stream{};
	stream.in_data = reinterpret_cast<const unsigned char*>(compressed_data_);
	stream.in_len = compressed_data_length_;
	stream.out_data = nullptr;
	stream.out_len = 0;
	lzma::decompress_stream_data(&stream);

	// Parse frames
	frames_ = parse_replay_raw_content(reinterpret_cast<char*>(stream.out_data));
}

replay::replay()
{
	is_initialized = false;
}

void replay::save_to_file(const std::string& save_file_path)
{
	std::ofstream replay_file(save_file_path, std::ios::out | std::ios::binary);
	replay_file.write(reinterpret_cast<char*>(&this->game_mode_), sizeof(this->game_mode_));
	replay_file.write(reinterpret_cast<char*>(&this->game_version_), sizeof(int));
	write_osu_string(replay_file, this->beatmap_md5_hash_);
	write_osu_string(replay_file, this->player_name_);
	write_osu_string(replay_file, this->replay_md5_hash_);
	replay_file.write(reinterpret_cast<char*>(&this->count_300_), sizeof(short));
	replay_file.write(reinterpret_cast<char*>(&this->count_100_), sizeof(short));
	replay_file.write(reinterpret_cast<char*>(&this->count_50_), sizeof(short));
	replay_file.write(reinterpret_cast<char*>(&this->count_geki_), sizeof(short));
	replay_file.write(reinterpret_cast<char*>(&this->count_katu_), sizeof(short));
	replay_file.write(reinterpret_cast<char*>(&this->count_miss_), sizeof(short));
	replay_file.write(reinterpret_cast<char*>(&this->total_score_), sizeof(int));
	replay_file.write(reinterpret_cast<char*>(&this->greatest_combo_), sizeof(short));
	replay_file.write(reinterpret_cast<char*>(&this->perfect_), sizeof(uint8_t));
	replay_file.write(reinterpret_cast<char*>(&this->mods_), sizeof(int));
	write_osu_string(replay_file, this->life_bar_graph_);
	replay_file.write(reinterpret_cast<char*>(&this->time_stamp_), sizeof(long long));

	const auto uncompressed_data = serialize_replay_frames_to_raw_text(frames_);
	lzma::data_stream stream;
	stream.in_data = reinterpret_cast<const unsigned char*>(uncompressed_data.c_str());
	stream.in_len = uncompressed_data.size();
	stream.out_data = nullptr;
	stream.out_len = 0;

	lzma::compress_stream_data(&stream);
	compressed_data_length_ = stream.out_len;
	compressed_data_ = new char[compressed_data_length_];
	compressed_data_ = reinterpret_cast<char*>(stream.out_data);

	replay_file.write(reinterpret_cast<char*>(&this->compressed_data_length_), sizeof(int));
	replay_file.write(compressed_data_, this->compressed_data_length_);
	replay_file.write(reinterpret_cast<char*>(&this->online_score_id_), sizeof(long long));
}

void replay::set_replay_frames(std::vector<replay_frame> frames)
{
	frames_ = std::move(frames);
}

std::vector<replay_frame>* replay::get_replay_frames()
{
	return &frames_;
}

replay::~replay() = default;

auto replay::get_beatmap_md5() const -> std::string
{
	return beatmap_md5_hash_;
}

std::string replay::read_osu_string(std::ifstream& replay_file)
{
	char chr;
	std::string str;

	// check if there is something to read first
	replay_file.read(&chr, sizeof(chr));
	if (chr != '\0')
	{
		// If it begins with this, play by peppy string rules
		if (chr == 0x0b)
		{
			// get the first part of the peppy string; size of the string
			std::string size_str;
			do
			{
				replay_file.read(&chr, sizeof(chr));
				size_str += chr;
			}
			while (chr < 0);

			// read the peppy string
			const int size = decode_uleb128(reinterpret_cast<const uint8_t*>(size_str.data()));
			auto* const tmp = new char[size + 1];
			replay_file.read(tmp, size);
			tmp[size] = '\0';
			str.assign(tmp);
			delete[] tmp;
		}
		else // otherwise, take it as a normal null terminated string
		{
			str += chr;
			replay_file.read(&chr, sizeof(chr));

			while (chr != '\0')
			{
				str += chr;
				replay_file.read(&chr, sizeof(chr));
			}
		}
	}

	return str;
}

void replay::write_osu_string(std::ofstream& replay_file, const std::string& string)
{
	uint8_t string_start = 0x0b;
	if (!string.empty())
	{
		//replay_file << 0x0b;
		replay_file.write(reinterpret_cast<char*>(&string_start), sizeof(uint8_t));
		encode_uleb128(string.size(), replay_file);
		//replay_file << string;
		replay_file.write(string.c_str(), string.size());
	}
	else
	{
		uint8_t string_length = 0x00;
		replay_file.write(reinterpret_cast<char*>(&string_length), sizeof(uint8_t));
	}
}
