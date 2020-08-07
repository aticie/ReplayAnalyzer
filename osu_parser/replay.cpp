#include "replay.h"
#include "replay_utils.h"
#include "../utils/LEB128.h"
#include "LZMA.h"
#include <cassert>

bool Replay::is_hardrock() {
	if ((mods & 16) == 16) {
		return true;
	}
	return false;
}

Replay::Replay(std::string replay_path) {
	std::ifstream replay_file(replay_path, std::ios::binary);
	if (!replay_file.is_open())
	{
		is_initialized = false;
		ReplayFrame dummy(0, 0, sf::Vector2f(256, 256), 0);
		frames.push_back(dummy);
		return;
	}
	is_initialized = true;
	replay_file.read(reinterpret_cast<char*>(&this->game_mode), sizeof(unsigned char));
	replay_file.read(reinterpret_cast<char*>(&this->game_version), sizeof(int));
	this->beatmap_md5_hash = read_osu_string(replay_file);
	this->player_name = read_osu_string(replay_file);
	this->replay_md5_hash = read_osu_string(replay_file);
	replay_file.read(reinterpret_cast<char*>(&this->count_300), sizeof(short));
	replay_file.read(reinterpret_cast<char*>(&this->count_100), sizeof(short));
	replay_file.read(reinterpret_cast<char*>(&this->count_50), sizeof(short));
	replay_file.read(reinterpret_cast<char*>(&this->count_geki), sizeof(short));
	replay_file.read(reinterpret_cast<char*>(&this->count_katu), sizeof(short));
	replay_file.read(reinterpret_cast<char*>(&this->count_miss), sizeof(short));
	replay_file.read(reinterpret_cast<char*>(&this->total_score), sizeof(int));
	replay_file.read(reinterpret_cast<char*>(&this->greatest_combo), sizeof(short));
	replay_file.read(reinterpret_cast<char*>(&this->perfect), sizeof(unsigned char));
	replay_file.read(reinterpret_cast<char*>(&this->mods), sizeof(int));
	this->life_bar_graph = read_osu_string(replay_file);
	replay_file.read(reinterpret_cast<char*>(&this->time_stamp), sizeof(long long));
	replay_file.read(reinterpret_cast<char*>(&this->compressed_data_length), sizeof(int));

	this->compressed_data = new char[compressed_data_length];
	replay_file.read(compressed_data, compressed_data_length);
	replay_file.read(reinterpret_cast<char*>(&this->online_score_id), sizeof(long long));

	// Read LZMA Stream and decompress
	LZMA::DataStream stream;
	stream.inData = (const unsigned char*)compressed_data;
	stream.inLen = compressed_data_length;
	stream.outData = nullptr;
	stream.outLen = 0;
	LZMA::DecompressStreamData(&stream);
	//delete[] compressed_data;
	// Parse frames
	frames = parseReplayRawContent(reinterpret_cast<char*>(stream.outData));
}

void Replay::save_to_file(std::string save_file_path) {
	std::ofstream replay_file(save_file_path, std::ios::out | std::ios::binary);
	replay_file.write((char*)&this->game_mode, sizeof(this->game_mode));
	replay_file.write(reinterpret_cast<char*>(&this->game_version), sizeof(int));
	write_osu_string(replay_file, this->beatmap_md5_hash);
	write_osu_string(replay_file, this->player_name);
	write_osu_string(replay_file, this->replay_md5_hash);
	replay_file.write(reinterpret_cast<char*>(&this->count_300), sizeof(short));
	replay_file.write(reinterpret_cast<char*>(&this->count_100), sizeof(short));
	replay_file.write(reinterpret_cast<char*>(&this->count_50), sizeof(short));
	replay_file.write(reinterpret_cast<char*>(&this->count_geki), sizeof(short));
	replay_file.write(reinterpret_cast<char*>(&this->count_katu), sizeof(short));
	replay_file.write(reinterpret_cast<char*>(&this->count_miss), sizeof(short));
	replay_file.write(reinterpret_cast<char*>(&this->total_score), sizeof(int));
	replay_file.write(reinterpret_cast<char*>(&this->greatest_combo), sizeof(short));
	replay_file.write(reinterpret_cast<char*>(&this->perfect), sizeof(uint8_t));
	replay_file.write(reinterpret_cast<char*>(&this->mods), sizeof(int));
	write_osu_string(replay_file, this->life_bar_graph);
	replay_file.write(reinterpret_cast<char*>(&this->time_stamp), sizeof(long long));

	std::string uncompressed_data = serializeReplayFramesToRawText(frames);
	LZMA::DataStream stream;
	stream.inData = (const unsigned char*)uncompressed_data.c_str();
	stream.inLen = uncompressed_data.size();
	stream.outData = nullptr;
	stream.outLen = 0;

	LZMA::CompressStreamData(&stream);
	compressed_data_length = stream.outLen;
	compressed_data = new char[compressed_data_length];
	compressed_data = (char*)stream.outData;

	replay_file.write(reinterpret_cast<char*>(&this->compressed_data_length), sizeof(int));
	replay_file.write(compressed_data, this->compressed_data_length);
	replay_file.write(reinterpret_cast<char*>(&this->online_score_id), sizeof(long long));
}

void Replay::set_replay_frames(std::vector<ReplayFrame> fr) {
	frames = fr;
}

std::vector<ReplayFrame>* Replay::get_replay_frames() {
	return &frames;
}

Replay::~Replay() {
}

std::string Replay::get_beatmap_md5() {
	return beatmap_md5_hash;
}

std::string Replay::read_osu_string(std::ifstream& _replayFile)
{
	char chr;
	std::string str;

	// check if there is something to read first
	_replayFile.read(&chr, sizeof(chr));
	if (chr != '\0')
	{
		// If it begins with this, play by peppy string rules
		if (chr == 0x0b)
		{
			// get the first part of the peppy string; size of the string
			std::string size_str;
			do
			{
				_replayFile.read(&chr, sizeof(chr));
				size_str += chr;
			} while (chr < 0);

			// read the peppy string
			int size = decodeULEB128((const uint8_t*)size_str.data());
			char* tmp = new char[size + 1];
			_replayFile.read(tmp, size);
			tmp[size] = '\0';
			str.assign(tmp);
			delete[] tmp;
		}
		else // otherwise, take it as a normal null terminated string
		{
			str += chr;
			_replayFile.read(&chr, sizeof(chr));

			while (chr != '\0')
			{
				str += chr;
				_replayFile.read(&chr, sizeof(chr));
			}
		}
	}

	return str;
}

void Replay::write_osu_string(std::ofstream& _replayFile, std::string string)
{
	uint8_t string_start = 0x0b;
	if (string.size() > 0) {
		//_replayFile << 0x0b;
		_replayFile.write(reinterpret_cast<char*>(&string_start), sizeof(uint8_t));
		encodeULEB128(string.size(), _replayFile);
		//_replayFile << string;
		_replayFile.write(string.c_str(), string.size());
	}
	else {
		uint8_t string_length = 0x00;
		_replayFile.write(reinterpret_cast<char*>(&string_length), sizeof(uint8_t));
	}
}