#include "imgui-SFML.h"
#include "imgui.h"
#include "osu_parser/Beatmap.h"
#include "osu_parser/replay_utils.h"
#include "utils/draw_helpers.h"
#include "utils/string_utils.h"
#include <SFML/Graphics.hpp>

#include <imfilebrowser.h>
#include <string>
#include <algorithm>

#include <Windows.h>
#include <cinttypes>
#include <utility>
#include "osu_parser/replay.h"
#include "osu_parser/osu_path_finder.h"
#include "osu_parser/osu_database.h"

void draw_frames_subarray(sf::RenderWindow& window, std::vector<replay_frame>& frames_subarray, int selected_subframe_idx,
	int cursor_subframe_idx, bool draw_points, sf::CircleShape& frame_point,
	sf::Color& frame_point_color);
void calculate_pulled_frame_positions(sf::RenderWindow& window, std::vector<replay_frame>& frames, int current_frame_index,
	int pull_distance, float pull_power);
void show_point_statistics(replay_frame frame);
void zoom_in_or_out(sf::Event event, sf::Vector2f mid_point, sf::View& view);
void draw_beatmap_objects(sf::RenderWindow& window, beatmap* beatmap, int64_t current_time, sf::CircleShape hitobject,
                          const float* hitobject_trails, sf::Text text, sf::View view, replay);
void change_replay(const std::string& selected_replay, replay& replay_obj, beatmap& bmap,
                   std::unordered_map<std::string, beatmap_info> bmap_dict, sf::CircleShape& hitobject_circle,
                   std::wstring osu_path);
void key_state_to_array(int keys, bool* key_states);
void draw_bezier_curve(const sf::VertexArray& curve_points, sf::Color color, sf::RenderWindow& window);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR szCmdLine, _In_ int iCmdShow)
{
	// --- Initialize variables --- //

	// Get osu!.db path from Regedit
	auto osu_path = get_osu_path();
	auto osu_db_path = osu_path + L"osu!.db";
	osu_database osu_db(osu_db_path); // Parse and initialize osu!.db

	// Get an initial replay
	std::string selected_replay = "C:\\replay.osr";
	replay replay(selected_replay);
	auto* frames = replay.get_replay_frames();
	std::vector<replay_frame> frames_subarray;
	int64_t current_time;

	sf::Font font;
	font.loadFromFile("OpenSans-Regular.ttf");
	sf::Text text;
	text.setFont(font);
	text.setFillColor(sf::Color::White);

	auto const beatmap_info = osu_db.beatmap_mapping[replay.get_beatmap_md5()];
	auto const beatmap_path = get_beatmap_path(beatmap_info, osu_path);
	beatmap beatmap(beatmap_path, replay.is_hardrock());

	if (replay.is_hardrock())
	{
		beatmap.circle_size = beatmap.circle_size * 1.3f;
	}

	auto circle_radius = 54.4f - 4.48f * beatmap.circle_size;

	sf::CircleShape hitobject_circle(circle_radius);
	hitobject_circle.setOrigin(hitobject_circle.getRadius(), hitobject_circle.getRadius());
	text.setOrigin(static_cast<float>(text.getCharacterSize()) / 2.f, static_cast<float>(text.getCharacterSize()) / 2.f);
	float hitobj_trails[2] = { -250, 650 };

	hitobject_circle.setFillColor({ 0, 0, 0, 0 });
	hitobject_circle.setOutlineColor({ 255, 255, 255, 150 });
	hitobject_circle.setOutlineThickness(2);

	sf::Vector2f replay_middle(512.f / 2, 384.f / 2);

	// Initialize Window
	sf::ContextSettings settings;
	settings.antialiasingLevel = 4; // Set anti-aliasing level 4
	sf::RenderWindow window(sf::VideoMode(1200, 600), "", sf::Style::Default, settings);
	sf::View play_area_view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
	play_area_view.setViewport(sf::FloatRect(0.f, 0.f, 0.66f, 1.f));
	play_area_view.setCenter(400, 300);
	//main_transform.translate(main_transform.transformPoint(sf::Vector2f(144, 108)));
	play_area_view.move(-144, -108);

	sf::RectangleShape play_area_rectangle(sf::Vector2f(790, 590)); // Draw a 800x600 rectangle indicating play area
	window.setFramerateLimit(60);
	play_area_rectangle.setPosition(5 - 144, 5 - 108);
	float play_area_fill_color[3] = { 0.f, 0.f, 0.f };
	float play_area_outline_color[3] = { 0.15f, 0.15f, 0.15f };
	sf::Color pa_color(0, 0, 0, 255);
	sf::Color pa_outline_color(38, 38, 38, 255);
	play_area_rectangle.setFillColor(pa_color);
	play_area_rectangle.setOutlineColor(pa_outline_color);
	play_area_rectangle.setOutlineThickness(5);

	window.setVerticalSyncEnabled(true);
	ImGui::SFML::Init(window);

	// Initialize frame points and lines
	auto frame_point_size = 3.f;
	sf::CircleShape frame_point(frame_point_size, 3);
	frame_point.setOrigin(frame_point_size, frame_point_size);
	auto frame_point_color = sf::Color(42, 157, 244, 255);
	frame_point.setFillColor(frame_point_color);

	sf::Vector2f mouse_local_position(0, 0);
	sf::Vector2f mouse_window_pos;
	sf::Vector2f mouse_click_position;

	auto show_point_stats = false;
	auto pull_power = .85f; // Pull power between frames when editing the replay
	auto pull_distance = 35; // How far we will pull frames

	// Boolean flags for main loop
	auto stop_playing = false;
	auto space_pressed = false;
	auto first_click = true;
	auto click_in_play_area = false;

	// Starting parameters
	auto selected_frame_idx = -1;
	auto selected_subframe_idx = -1;
	auto cursor_subframe_idx = -1;
	auto current_frame_index = 0;
	auto trail_length = 20;
	
	char window_title[255] = "Poor Man's Replay Editor";

	ImGui::FileBrowser file_dialog;
	file_dialog.SetTitle("Find Replay File");
	file_dialog.SetTypeFilters({ ".osr" });
	char replay_save_path[255] = "C:\\replay.osr";

	window.setTitle(window_title);

	sf::Clock delta_clock;

	auto& io = ImGui::GetIO();

	while (window.isOpen())
	{
		sf::Event event{};
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
		}

		if (current_frame_index >= static_cast<int>((*frames).size()))
			current_frame_index = 0;

		if (!stop_playing)
			current_frame_index++;

		current_time = (*frames)[current_frame_index].get_abs_time();
		ImGui::SFML::Update(window, delta_clock.restart());

		//ImGui::ShowDemoWindow();
		ImGui::Begin("Settings");
		ImGui::SetWindowSize(ImVec2(380, 500));
		ImGui::SetWindowPos(ImVec2(810, 10));

		if (!replay.is_initialized)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "ERROR WITH REPLAY FILE! (Doesn't exist?)");
		}
		else
		{
			frames_subarray = get_frames_subarray(*frames, current_frame_index, trail_length);
			cursor_subframe_idx = trail_length;
			//auto inverse_transform = main_transform.getInverse();
			//mouse_local_position = inverse_transform.transformPoint((sf::Vector2f)sf::Mouse::getPosition(window));
			mouse_local_position = window.mapPixelToCoords(sf::Mouse::getPosition(window));
			mouse_window_pos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
			selected_subframe_idx = find_nearest_frame(frames_subarray, mouse_local_position);
		}
		if (!osu_db.is_initialized)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "COULDN'T FIND OSU! PATH");
		}
		if (!beatmap.is_initialized)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "COULDN'T FIND BEATMAP FILE");
		}

		ImGui::SliderInt("Frame No", &current_frame_index, 0, static_cast<int>((*frames).size()) - 1);
		ImGui::SliderInt("Trail Length", &trail_length, 10, 100);
		ImGui::SliderFloat("Pull Power", &pull_power, .01f, .99f);
		ImGui::SliderInt("Pull Distance", &pull_distance, 1, 60);
		ImGui::SliderFloat2("Hitobject Fade Times", hitobj_trails, -2000, 2000);

		ImGui::Text("Osu path: %ws", osu_path.c_str());
		ImGui::Text("Current Game time: %" PRId64 "", current_time);

		if (ImGui::ColorEdit3("Play Area Fill color", play_area_fill_color))
		{
			pa_color.r = static_cast<sf::Uint8>(play_area_fill_color[0] * 255.f);
			pa_color.g = static_cast<sf::Uint8>(play_area_fill_color[1] * 255.f);
			pa_color.b = static_cast<sf::Uint8>(play_area_fill_color[2] * 255.f);
		}

		if (ImGui::ColorEdit3("Play Area Outline color", play_area_outline_color))
		{
			pa_outline_color.r = static_cast<sf::Uint8>(play_area_outline_color[0] * 255.f);
			pa_outline_color.g = static_cast<sf::Uint8>(play_area_outline_color[1] * 255.f);
			pa_outline_color.b = static_cast<sf::Uint8>(play_area_outline_color[2] * 255.f);
		}

		if (ImGui::InputText("Window title", window_title, 255))
		{
			window.setTitle(window_title);
		}
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
			1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Mouse location: (%.0f, %.0f)", mouse_local_position.x,
			mouse_local_position.y);

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) and
			first_click) // first mouse click
		{
			first_click = false;
			auto play_area_global_bounds = play_area_rectangle.getGlobalBounds();
			click_in_play_area = play_area_global_bounds.contains(mouse_window_pos);
			if (selected_subframe_idx != -1)
			{
				stop_playing = true;
				current_frame_index = std::clamp(selected_subframe_idx + current_frame_index - trail_length, 0,
					static_cast<int>((*frames).size() - 1));
				selected_frame_idx = current_frame_index;
			}
			else
			{
				mouse_click_position = static_cast<sf::Vector2f>(sf::Mouse::getPosition());
			}
		}
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) and !first_click) // hold
		{
			if (click_in_play_area)
			{
				if (selected_frame_idx == -1)
				{
					// Move frame
					sf::Vector2f new_mouse_pos(static_cast<sf::Vector2f>(sf::Mouse::getPosition()));
					auto mouse_pos_change = mouse_click_position - new_mouse_pos;
					play_area_view.move(mouse_pos_change);
					//main_transform.translate(-mouse_pos_change);
					mouse_click_position = new_mouse_pos;
				}
				else
				{
					// Move frame points
					calculate_pulled_frame_positions(window, *frames, current_frame_index, pull_distance, pull_power);
					//frames[current_frame_index].setMousePos((sf::Vector2f)sf::Mouse::getPosition(window));
					show_point_stats = true;
				}
			}
		}
		if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			first_click = true;
			show_point_stats = false;
			selected_frame_idx = -1;
		}

		if (event.type == sf::Event::MouseWheelScrolled)
		{
			if (play_area_rectangle.getGlobalBounds().contains(mouse_window_pos))
			{
				zoom_in_or_out(event, replay_middle, play_area_view);
				event.type = sf::Event::MouseEntered;
			}
		}

		if (ImGui::Button(selected_replay.c_str()))
			file_dialog.Open();

		if (io.KeysDown[0x39] and !space_pressed)
		{
			space_pressed = true;
			stop_playing = !stop_playing;
		}
		if (!io.KeysDown[0x39] and space_pressed)
			space_pressed = false;

		if (io.KeysDown[0x11])
		{
			current_frame_index = 0;
		}
		ImGui::InputText("Replay save location", replay_save_path, 255);
		if (ImGui::Button("Save to file"))
		{
			replay.save_to_file(replay_save_path);
		}

		if (show_point_stats)
			show_point_statistics((*frames)[current_frame_index]);
		file_dialog.Display();

		if (file_dialog.HasSelected())
		{
			selected_replay = file_dialog.GetSelected().string();
			if (endsWith(selected_replay, ".osr"))
			{
				change_replay(selected_replay, replay, beatmap, osu_db.beatmap_mapping, hitobject_circle,
					osu_path);
			}
			file_dialog.ClearSelected();
		}

		window.clear(sf::Color(0, 0, 0, 255)); // fill background with color
		window.draw(play_area_rectangle);
		draw_frames_subarray(window, frames_subarray, selected_subframe_idx, cursor_subframe_idx, true, frame_point,
			frame_point_color);
		if (beatmap.is_initialized)
		{
			draw_beatmap_objects(window, &beatmap, current_time, hitobject_circle, hitobj_trails, text, play_area_view,
				replay);
		}
		ImGui::End(); // end window
		window.setView(play_area_view);
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
}

void change_replay(const std::string& selected_replay,
                   replay& replay_obj,
                   beatmap& bmap,
                   std::unordered_map<std::string, beatmap_info> bmap_dict,
                   sf::CircleShape& hitobject_circle,
                   std::wstring osu_path)
{
	replay_obj = replay(selected_replay);
	const auto beatmap_info = bmap_dict[replay_obj.get_beatmap_md5()];
	const auto beatmap_path = get_beatmap_path(beatmap_info, std::move(osu_path));
	bmap = beatmap(beatmap_path, replay_obj.is_hardrock());
	if (replay_obj.is_hardrock())
	{
		bmap.circle_size *= 1.3f;
	}
	const auto circle_radius = 54.4f - 4.48f * bmap.circle_size;

	hitobject_circle.setRadius(circle_radius);
	hitobject_circle.setOrigin(hitobject_circle.getRadius(), hitobject_circle.getRadius());
}

sf::Vertex lerp_sfml_vectors(const float t, const sf::Vector2f a, const sf::Vector2f b)
{
	const auto x = a.x * (1 - t) + b.x * t;
	const auto y = a.y * (1 - t) + b.y * t;
	return sf::Vertex({ x, y });
}

auto draw_bezier_curve(const sf::VertexArray& curve_points, sf::Color color, sf::RenderWindow& window) -> void
{
	const float precision = 0.01;

	sf::VertexArray bezier_points;

	for (auto j = 0; j < 100; j+=1)
	{
		const auto t = precision * j;
		auto temp1 = curve_points;

		while (temp1.getVertexCount() > 1)
		{
			sf::VertexArray temp2;

			for (unsigned int i = 0; i < temp1.getVertexCount() - 1; i++)
			{
				const auto pt1 = temp1[i];
				const auto pt2 = temp1[i + 1];

				temp2.append(lerp_sfml_vectors(t, pt1.position, pt2.position));
			}

			temp1 = temp2;
		}
		bezier_points.append(temp1[0]);
	}
	bezier_points.setPrimitiveType(sf::PrimitiveType::LinesStrip);
	window.draw(bezier_points);
}

auto draw_beatmap_objects(sf::RenderWindow& window, beatmap* beatmap, int64_t current_time, sf::CircleShape hitobject,
                          const float* hitobject_trails, sf::Text text, sf::View view, replay replay) -> void
{
	float length = 0;
	auto const od = beatmap->overall_diff;

	auto* frames = replay.get_replay_frames();

	for (auto const& hitobj : beatmap->hitobjects)
	{
		if ((hitobj.get_type() & 2) == 2)
		{
			length = hitobj.get_length();
		}
		else
			length = 0;
		if (hitobj.get_time() > current_time + hitobject_trails[0] && hitobj.get_time() < current_time +
			hitobject_trails[1] + length)
		{
			if ((hitobj.get_type() & 2) == 2)
			{
				if (hitobj.curve_points.getVertexCount() > 3)
				{
					draw_bezier_curve(hitobj.curve_points, sf::Color::White, window);
				}
			}
			hitobject.setPosition(hitobj.get_pos());
			text.setPosition(hitobj.get_pos());
			auto hitobj_num = std::to_string(hitobj.get_count());
			text.setString(hitobj_num);
			window.draw(hitobject);
			window.draw(text);
		}
	}
}

void zoom_in_or_out(const sf::Event event, sf::Vector2f mid_point, sf::View& view)
{
	const auto zoom_amount = (-event.mouseWheelScroll.delta + 1.f) * 0.25f + 0.75f;
	const auto new_size = zoom_amount * view.getSize();
	view.setSize(new_size);
}

void calculate_pulled_frame_positions(sf::RenderWindow& window, std::vector<replay_frame>& frames, const int current_frame_index,
	const int pull_distance, const float pull_power)
{
	const auto mouse_position = window.mapPixelToCoords(sf::Mouse::getPosition(window));
	const auto pos_change = mouse_position - frames[current_frame_index].get_mouse_pos();
	frames[current_frame_index].set_mouse_pos(mouse_position);
	for (auto i = 1; i < pull_distance; i++)
	{
		const auto next_frame_idx = std::clamp(current_frame_index + i, 0, static_cast<int>(frames.size()) - 1);
		const auto prev_frame_idx = std::clamp(current_frame_index - i, 0, static_cast<int>(frames.size()) - 1);

		const float current_pull_power = std::pow(pull_power, i * 1.2);

		frames[next_frame_idx].set_mouse_pos(frames[next_frame_idx].get_mouse_pos() + pos_change * current_pull_power);
		frames[prev_frame_idx].set_mouse_pos(frames[prev_frame_idx].get_mouse_pos() + pos_change * current_pull_power);
	}
}

void show_point_statistics(replay_frame frame)
{
	ImGui::Begin("Point Information");
	ImGui::Text("Time: %lld", frame.get_abs_time());
	ImGui::Text("[X,Y]: [%.0f, %.0f]", frame.get_mouse_pos().x, frame.get_mouse_pos().y);
	ImGui::Text("KeyState: %d", frame.get_key_state());
	ImGui::End();
}

void key_state_to_array(const int keys, bool* key_states)
{
	for (auto i = 0; i < 5; i++)
	{
		const int key_num = std::pow(2, (i));
		key_states[i] = keys & key_num;
	}
}

void draw_frames_subarray(sf::RenderWindow& window, std::vector<replay_frame>& frames_subarray, int selected_subframe_idx,
	int cursor_subframe_idx, bool draw_points, sf::CircleShape& frame_point,
	sf::Color& frame_point_color)
{
	if (frames_subarray.empty())
		return;
	
	auto cur_mouse_pos = frames_subarray[0].get_mouse_pos();
	bool cur_key_states[5] = { false, false, false, false, false };
	bool next_key_states[5] = { false, false, false, false, false };

	for (auto j = 0; j < static_cast<int>(frames_subarray.size()) - 1; j++)
	{
		auto next_mouse_pos = frames_subarray[j + 1].get_mouse_pos();
		key_state_to_array(frames_subarray[j].get_key_state(), cur_key_states);
		key_state_to_array(frames_subarray[j + 1].get_key_state(), next_key_states);

		if (cur_key_states[2] && cur_key_states[3])
			frame_point_color = sf::Color(177.f, 0.f, 132.f, 255.f);
		else if (cur_key_states[2] && !cur_key_states[3])
			frame_point_color = sf::Color(199.f, 0.f, 57.f, 255.f);
		else if (!cur_key_states[2] && cur_key_states[3])
			frame_point_color = sf::Color(0.f, 191.f, 255.f, 255.f);
		else
			frame_point_color = sf::Color(25.f, 25.f, 25.f, 255.f);

		auto line = draw_line_from_two_points(cur_mouse_pos, next_mouse_pos);

		line.setFillColor(frame_point_color);
		frame_point.setFillColor(frame_point_color);

		frame_point.setPosition(next_mouse_pos - sf::Vector2f(0, 5));

		if (cur_key_states[2] != next_key_states[2])
		{
			frame_point.setFillColor(sf::Color(199.f, 0.f, 57.f, 255.f));
			if ((cur_key_states[2] == true) && next_key_states[2] == false)
			{
				frame_point.setRotation(0);
			}
			else frame_point.setRotation(180);
			window.draw(frame_point);
		}
		if (cur_key_states[3] != next_key_states[3])
		{
			frame_point.setFillColor(sf::Color(0.f, 191.f, 255.f, 255.f));
			if ((cur_key_states[3] == true) && next_key_states[3] == false)
			{
				frame_point.setRotation(0);
			}
			else frame_point.setRotation(180);
			window.draw(frame_point);
		}
		window.draw(line);

		cur_mouse_pos = next_mouse_pos;
	}
}