#include "osu_database.h"

BeatmapInfo::BeatmapInfo() {
}

std::string BeatmapInfo::get_hash() {
	return md5;
}

OsuDatabase::OsuDatabase(std::wstring db_path) {
	if (!std::filesystem::exists(db_path)) {
		is_initialized = false;
		return;
	}
	is_initialized = true;
	std::ifstream db_file(db_path, std::ios::binary);
	db_file.read((char*)&osu_version, sizeof(osu_version));
	db_file.read((char*)&folder_count, sizeof(folder_count));
	db_file.read((char*)&account_unlocked, sizeof(account_unlocked));
	db_file.read((char*)&date_to_unlock, sizeof(date_to_unlock));
	player_name = Replay::read_osu_string(db_file);
	db_file.read((char*)&nr_beatmaps, sizeof(nr_beatmaps));

	for (int i = 0; i < nr_beatmaps; i++) {
		BeatmapInfo bmap = parse_beatmap(db_file);
		beatmap_mapping.insert({ bmap.get_hash(), bmap });
	}
	/*int folder_count;
	bool account_unlocked;
	long long date_to_unlock;
	std::string player_name;
	int nr_beatmaps;
	std::vector<BeatmapInfo> beatmaps;
	int user_permissions;*/
}


BeatmapInfo parse_beatmap(std::ifstream& db_file) {
	BeatmapInfo bmap;
	for (int i = 0; i < 7; i++) {
		Replay::read_osu_string(db_file); // Read artist name, song title, creator, difficulty, audio file name
	}
	bmap.md5 = Replay::read_osu_string(db_file);
	bmap.filename = Replay::read_osu_string(db_file);
	db_file.seekg(39, std::ios::cur);
	int nr_of_diff = 0;
	for (int i = 0; i < 4; i++) {
		db_file.read((char*)&nr_of_diff, sizeof(nr_of_diff));
		int skip_this = 14 * nr_of_diff;
		db_file.seekg(skip_this, std::ios::cur);
	}
	db_file.seekg(12, std::ios::cur);
	int nr_of_timing = 0;
	db_file.read((char*)&nr_of_timing, sizeof(nr_of_timing));
	int nr_of_bytes_to_read = 17 * nr_of_timing;
	db_file.seekg(nr_of_bytes_to_read, std::ios::cur);
	db_file.seekg(23, std::ios::cur);
	Replay::read_osu_string(db_file); // Source
	Replay::read_osu_string(db_file); // Tags
	db_file.seekg(2, std::ios::cur); // Online offset
	Replay::read_osu_string(db_file); // Font used
	db_file.seekg(10, std::ios::cur); // unplayed, last time played, osz2?

	bmap.foldername = Replay::read_osu_string(db_file); // Folder name
	db_file.seekg(18, std::ios::cur); // Garbage
	return bmap;
}