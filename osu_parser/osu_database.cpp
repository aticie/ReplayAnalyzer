#include "osu_database.h"

beatmap_info::beatmap_info()
= default;

std::string beatmap_info::get_hash() const
{
	return md5;
}

osu_database::osu_database(const std::wstring& db_path)
{
	if (!std::filesystem::exists(db_path))
	{
		is_initialized = false;
		return;
	}
	is_initialized = true;
	std::ifstream db_file(db_path, std::ios::binary);
	db_file.read(reinterpret_cast<char*>(&osu_version_), sizeof(osu_version_));
	db_file.read(reinterpret_cast<char*>(&folder_count_), sizeof(folder_count_));
	db_file.read(reinterpret_cast<char*>(&account_unlocked_), sizeof(account_unlocked_));
	db_file.read(reinterpret_cast<char*>(&date_to_unlock_), sizeof(date_to_unlock_));
	player_name_ = replay::read_osu_string(db_file);
	db_file.read(reinterpret_cast<char*>(&nr_beatmaps_), sizeof(nr_beatmaps_));

	for (auto i = 0; i < nr_beatmaps_; i++)
	{
		auto bmap = parse_beatmap(db_file);
		beatmap_mapping.insert({bmap.get_hash(), bmap});
	}
}


beatmap_info parse_beatmap(std::ifstream& db_file)
{
	beatmap_info bmap;
	for (auto i = 0; i < 7; i++)
	{
		replay::read_osu_string(db_file); // Read artist name, song title, creator, difficulty, audio file name
	}
	bmap.md5 = replay::read_osu_string(db_file);
	bmap.filename = replay::read_osu_string(db_file);
	db_file.seekg(39, std::ios::cur);
	auto nr_of_diff = 0;
	for (auto i = 0; i < 4; i++)
	{
		db_file.read(reinterpret_cast<char*>(&nr_of_diff), sizeof(nr_of_diff));
		auto const skip_this = 14 * nr_of_diff;
		db_file.seekg(skip_this, std::ios::cur);
	}
	db_file.seekg(12, std::ios::cur);
	auto nr_of_timing = 0;
	db_file.read(reinterpret_cast<char*>(&nr_of_timing), sizeof(nr_of_timing));
	auto const nr_of_bytes_to_read = 17 * nr_of_timing;
	db_file.seekg(nr_of_bytes_to_read, std::ios::cur);
	db_file.seekg(23, std::ios::cur);
	replay::read_osu_string(db_file); // Source
	replay::read_osu_string(db_file); // Tags
	db_file.seekg(2, std::ios::cur); // Online offset
	replay::read_osu_string(db_file); // Font used
	db_file.seekg(10, std::ios::cur); // unplayed, last time played, osz2?

	bmap.foldername = replay::read_osu_string(db_file); // Folder name
	db_file.seekg(18, std::ios::cur); // Garbage
	return bmap;
}
