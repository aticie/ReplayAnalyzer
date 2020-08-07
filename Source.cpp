#include "imgui-SFML.h"
#include "imgui.h"
#include "osu_parser/Beatmap.h"
#include "osu_parser/replay_utils.h"
#include "utils/draw_helpers.h"
#include "utils/string_utils.h"
#include <SFML/Graphics.hpp>

#include <fstream>
#include <imfilebrowser.h>
#include <iostream>
#include <string>
#include <algorithm>

#include <Windows.h>
#include <inttypes.h>
#include "osu_parser/replay.h"
#include "osu_parser/osu_path_finder.h"
#include "osu_parser/osu_database.h"

void drawFramesSubarray(sf::RenderWindow& window, std::vector<ReplayFrame>& frames_subarray, int selected_subframe_idx, int cursor_subframe_idx, bool draw_points, sf::CircleShape& frame_point, sf::Color& frame_point_color);
void calculatePulledFramePositions(sf::RenderWindow& window, std::vector<ReplayFrame>& frames, int current_frame_index, int pull_distance, float pull_power);
void showPointStatistics(ReplayFrame frame);
void zoomInOrOut(sf::Event event, sf::Vector2f mid_point, sf::View &view);
void draw_beatmap_objects(sf::RenderWindow& window, Beatmap* beatmap, int64_t current_time, sf::CircleShape hitobject, float* hitobject_trails, sf::Text text, sf::View view, Replay);
void change_replay(std::string selected_replay, Replay& replay, std::vector<ReplayFrame> *frames, Beatmap &bmap, std::unordered_map<std::string, BeatmapInfo> bmap_dict, sf::CircleShape &hitobject_circle, std::wstring osu_path);
void key_state_to_array(int keys, bool* key_states);
void drawBezierCurve(sf::VertexArray curve_points, sf::Color color, sf::RenderWindow& window);


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR szCmdLine, _In_ int iCmdShow) {

	// --- Initialize variables --- //

	// Get osu!.db path from Regedit
	std::wstring osu_path = GetOsuPath();
	std::wstring osu_db_path = osu_path + L"osu!.db";
	OsuDatabase osu_db(osu_db_path);  // Parse and initialize osu!.db

	// Get an initial replay
	std::string selected_replay = "C:\\replay.osr";
	Replay replay(selected_replay);
	std::vector<ReplayFrame> *frames = replay.get_replay_frames();
	std::vector<ReplayFrame> frames_subarray;
	int64_t current_time;

	sf::Font font;
	font.loadFromFile("OpenSans-Regular.ttf");
	sf::Text text;
	text.setFont(font);
	text.setFillColor(sf::Color::White);

	BeatmapInfo beatmap_info = osu_db.beatmap_mapping[replay.get_beatmap_md5()];
	std::string beatmap_path = get_beatmap_path(beatmap_info, osu_path);
	Beatmap beatmap(beatmap_path, replay.is_hardrock());

	if (replay.is_hardrock()) {
		beatmap.circle_size = beatmap.circle_size * 1.3;
	}

	float circle_radius = 54.4 - 4.48 * beatmap.circle_size;

	sf::CircleShape hitobject_circle(circle_radius);
	hitobject_circle.setOrigin(hitobject_circle.getRadius(), hitobject_circle.getRadius());
	text.setOrigin(text.getCharacterSize()/2, text.getCharacterSize()/2);
	float hitobj_trails[2] = { -250, 650 };
	
	hitobject_circle.setFillColor({ 0,0,0,0 });
	hitobject_circle.setOutlineColor({ 255, 255, 255, 150 });
	hitobject_circle.setOutlineThickness(2);

	sf::Vector2f replay_middle(512.f / 2, 384.f / 2);
	int translate_amount[2] = { 0, 0 };
	float scale_amount = 1.f;

	// Initialize Window
	sf::ContextSettings settings;
	settings.antialiasingLevel = 4;  // Set anti-aliasing level 4
	sf::RenderWindow window(sf::VideoMode(1200, 600), "", sf::Style::Default, settings);
	sf::View play_area_view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
	play_area_view.setViewport(sf::FloatRect(0.f, 0.f, 0.66f, 1.f));
	play_area_view.setCenter(400, 300);	//main_transform.translate(main_transform.transformPoint(sf::Vector2f(144, 108)));
	play_area_view.move(-144, -108);

	sf::RectangleShape play_area_rectangle(sf::Vector2f(790, 590));// Draw a 800x600 rectangle indicating play area
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
	float frame_point_size = 3.f;
	sf::CircleShape frame_point(frame_point_size, 3);
	frame_point.setOrigin(frame_point_size, frame_point_size);
	sf::Color frame_point_color = sf::Color(42, 157, 244, 255);
	float fp_color[3] = { 0.165f, 0.615f, .956f };  // Frame point color
	frame_point.setFillColor(frame_point_color);

	sf::Vector2f mouse_local_position(0, 0);
	sf::Vector2f mouse_window_pos;
	sf::Vector2f mouse_click_position;

	bool show_point_stats = false;
	float pull_power = .85f;  // Pull power between frames when editing the replay
	int pull_distance = 35;  // How far we will pull frames

	// Boolean flags for main loop
	bool stop_playing = false;
	bool space_pressed = false;
	bool first_click = true;
	bool click_in_play_area = false;

	// Starting parameters
	int selected_frame_idx = -1;
	int selected_subframe_idx = -1;
	int cursor_subframe_idx = -1;
	int current_frame_index = 0;
	int trail_length = 20;

	int scroll_frames = 0;

	char windowTitle[255] = "Poor Man's Replay Editor";

	ImGui::FileBrowser fileDialog;
	fileDialog.SetTitle("Find Replay File");
	fileDialog.SetTypeFilters({ ".osr", ".osrp" });
	char replay_save_path[255] = "C:\\replay.osr";

	window.setTitle(windowTitle);

	sf::Clock deltaClock;

	ImGuiIO& io = ImGui::GetIO();

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		if (current_frame_index >= (int)((*frames).size()))
			current_frame_index = 0;

		if (!stop_playing)
			current_frame_index++;

		current_time = (*frames)[current_frame_index].getAbsTime();
		ImGui::SFML::Update(window, deltaClock.restart());

		//ImGui::ShowDemoWindow();
		ImGui::Begin("Settings");
		ImGui::SetWindowSize(ImVec2(380, 500));
		ImGui::SetWindowPos(ImVec2(810, 10));

		if (!replay.is_initialized) {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "ERROR WITH REPLAY FILE! (Doesn't exist?)");
		}
		else {
			frames_subarray = getFramesSubarray(*frames, current_frame_index, trail_length);
			cursor_subframe_idx = trail_length;
			//auto inverse_transform = main_transform.getInverse();
			//mouse_local_position = inverse_transform.transformPoint((sf::Vector2f)sf::Mouse::getPosition(window));
			mouse_local_position = window.mapPixelToCoords(sf::Mouse::getPosition(window));
			mouse_window_pos = (sf::Vector2f)sf::Mouse::getPosition(window);
			selected_subframe_idx = findNearestFrame(frames_subarray, mouse_local_position);
		}
		if (!osu_db.is_initialized) {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "COULDN'T FIND OSU! PATH");
		}
		if (!beatmap.is_initialized) {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "COULDN'T FIND BEATMAP FILE");
		}
		

		ImGui::SliderInt("Frame No", &current_frame_index, 0, (*frames).size() - 1);
		ImGui::SliderInt("Trail Length", &trail_length, 10, 100);
		ImGui::SliderFloat("Pull Power", &pull_power, .01f, .99f);
		ImGui::SliderInt("Pull Distance", &pull_distance, 1, 60);
		ImGui::SliderFloat2("Hitobject Fade Times", hitobj_trails, -2000, 2000);

		ImGui::Text("Osu path: %ws", osu_path.c_str());
		ImGui::Text("Current Game time: %" PRId64 "", current_time);

		if (ImGui::ColorEdit3("Play Area Fill color", play_area_fill_color)) {
			pa_color.r = static_cast<sf::Uint8>(play_area_fill_color[0] * 255.f);
			pa_color.g = static_cast<sf::Uint8>(play_area_fill_color[1] * 255.f);
			pa_color.b = static_cast<sf::Uint8>(play_area_fill_color[2] * 255.f);
		}

		if (ImGui::ColorEdit3("Play Area Outline color", play_area_outline_color)) {
			pa_outline_color.r = static_cast<sf::Uint8>(play_area_outline_color[0] * 255.f);
			pa_outline_color.g = static_cast<sf::Uint8>(play_area_outline_color[1] * 255.f);
			pa_outline_color.b = static_cast<sf::Uint8>(play_area_outline_color[2] * 255.f);
		}

		if (ImGui::InputText("Window title", windowTitle, 255)) {
			window.setTitle(windowTitle);
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
			if (selected_subframe_idx != -1) {
				stop_playing = true;
				current_frame_index = std::clamp(selected_subframe_idx + current_frame_index - trail_length, 0, (int)((*frames).size() - 1));
				selected_frame_idx = current_frame_index;
			}
			else {
				mouse_click_position = (sf::Vector2f)sf::Mouse::getPosition();
			}
		}
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) and !first_click) // hold
		{
			if (click_in_play_area) {
				if (selected_frame_idx == -1) {
					// Move frame
					sf::Vector2f new_mouse_pos((sf::Vector2f)sf::Mouse::getPosition());
					auto mouse_pos_change = mouse_click_position - new_mouse_pos;
					play_area_view.move(mouse_pos_change);
					//main_transform.translate(-mouse_pos_change);
					mouse_click_position = new_mouse_pos;
				}
				else {
					// Move frame points
					calculatePulledFramePositions(window, *frames, current_frame_index, pull_distance, pull_power);
					//frames[current_frame_index].setMousePos((sf::Vector2f)sf::Mouse::getPosition(window));
					show_point_stats = true;
				}
			}
		}
		if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			first_click = true;
			show_point_stats = false;
			selected_frame_idx = -1;
		}

		if (event.type == sf::Event::MouseWheelScrolled) {
			if (play_area_rectangle.getGlobalBounds().contains(mouse_window_pos)) {
				zoomInOrOut(event, replay_middle, play_area_view);
				event.type = sf::Event::MouseEntered;
			}
		}

		if (ImGui::Button(selected_replay.c_str()))
			fileDialog.Open();

		if (io.KeysDown[0x39] and !space_pressed) {
			space_pressed = true;
			stop_playing = !stop_playing;
		}
		if (!io.KeysDown[0x39] and space_pressed)
			space_pressed = false;

		if (io.KeysDown[0x11]) {
			current_frame_index = 0;
		}
		ImGui::InputText("Replay save location", replay_save_path, 255);
		if (ImGui::Button("Save to file")) {
			replay.save_to_file(replay_save_path);
		}

		if (show_point_stats)
			showPointStatistics((*frames)[current_frame_index]);
		fileDialog.Display();

		if (fileDialog.HasSelected()) {
			selected_replay = fileDialog.GetSelected().string();
			if (endsWith(selected_replay, ".osr")) {
				change_replay(selected_replay, replay, frames, beatmap, osu_db.beatmap_mapping, hitobject_circle, osu_path);
				
			}
			fileDialog.ClearSelected();
		}

		window.clear(sf::Color(0,0,0,255)); // fill background with color
		window.draw(play_area_rectangle);
		drawFramesSubarray(window, frames_subarray, selected_subframe_idx, cursor_subframe_idx, true, frame_point, frame_point_color);
		if (beatmap.is_initialized) {
			draw_beatmap_objects(window, &beatmap, current_time, hitobject_circle, hitobj_trails, text, play_area_view, replay);
		}
		ImGui::End(); // end window
		window.setView(play_area_view);
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
}

void change_replay(std::string selected_replay, 
	Replay& replay, 
	std::vector<ReplayFrame> *frames, 
	Beatmap &bmap,	
	std::unordered_map<std::string, BeatmapInfo> bmap_dict, 
	sf::CircleShape &hitobject_circle,
	std::wstring osu_path)
{
	replay = Replay(selected_replay);
	frames = replay.get_replay_frames();
	BeatmapInfo beatmap_info = bmap_dict[replay.get_beatmap_md5()];
	std::string beatmap_path = get_beatmap_path(beatmap_info, osu_path);
	bmap = Beatmap(beatmap_path, replay.is_hardrock());
	if (replay.is_hardrock()) {
		bmap.circle_size *= 1.3;
	}
	float circle_radius = 54.4 - 4.48 * bmap.circle_size;

	hitobject_circle.setRadius(circle_radius);
	hitobject_circle.setOrigin(hitobject_circle.getRadius(), hitobject_circle.getRadius());
}

sf::Vertex lerp_sfml_vectors(float t, sf::Vector2f a, sf::Vector2f b) {
	float x = a.x * (1 - t) + b.x * t;
	float y = a.y * (1 - t) + b.y * t;
	return sf::Vertex({ x, y });
}

void drawBezierCurve(sf::VertexArray curve_points, sf::Color color, sf::RenderWindow& window) {
	const float precision = 0.01;

	sf::VertexArray bezierPoints;

	//window.draw(curve_points);
	for (float t = 0; t < 1; t = t + precision) {
		sf::VertexArray temp1 = curve_points;

		while (temp1.getVertexCount() > 1)
		{
			sf::VertexArray temp2;

			for (int i = 0; i < temp1.getVertexCount() - 1; i++)
			{
				sf::Vertex pt1 = temp1[i];
				sf::Vertex pt2 = temp1[i + 1];

				temp2.append(lerp_sfml_vectors(t, pt1.position, pt2.position));
			}

			temp1 = temp2;
		}
		bezierPoints.append(temp1[0]);
	}
	bezierPoints.setPrimitiveType(sf::PrimitiveType::LinesStrip);
	window.draw(bezierPoints);
}

void draw_beatmap_objects(sf::RenderWindow& window, Beatmap* beatmap, int64_t current_time, sf::CircleShape hitobject, float *hitobject_trails,sf::Text text, sf::View view, Replay replay) {
	float length = 0;
	float od = beatmap->overall_diff;

	std::vector<ReplayFrame>* frames = replay.get_replay_frames();
	
	for (auto hitobj : beatmap->hitobjects) {
		if ((hitobj.getType() & 2) == 2) {
			length = hitobj.getLength();
		}
		else
			length = 0;
		if ((hitobj.getTime() > current_time + hitobject_trails[0]) && hitobj.getTime() < current_time + hitobject_trails[1] + length) {
			if ((hitobj.getType() & 2) == 2) {
				if (hitobj.curve_points.getVertexCount() > 3) {
					drawBezierCurve(hitobj.curve_points, sf::Color::White, window);
				}
			}
			hitobject.setPosition(hitobj.getPos());
			text.setPosition(hitobj.getPos());
			std::string hitobj_num = std::to_string(hitobj.getCount());
			text.setString(hitobj_num);
			window.draw(hitobject);
			window.draw(text);
		}
	}
}

void zoomInOrOut(sf::Event event, sf::Vector2f mid_point, sf::View &view) {
	float zoom_amount = ((-event.mouseWheelScroll.delta + 1) * 0.25 + 0.75);
	sf::Vector2f new_size = zoom_amount * view.getSize();
	view.setSize(new_size);
	//view.zoom(zoom_amount);
	//main_transform.scale(sf::Vector2f(zoom_amount, zoom_amount), mid_point);
}

void calculatePulledFramePositions(sf::RenderWindow& window, std::vector<ReplayFrame>& frames, int current_frame_index, int pull_distance, float pull_power) {
	//auto inverse_transform = main_transform.getInverse();
	//sf::Vector2f mouse_position = inverse_transform.transformPoint((sf::Vector2f)sf::Mouse::getPosition(window));
	sf::Vector2f mouse_position = window.mapPixelToCoords(sf::Mouse::getPosition(window));
	sf::Vector2f pos_change = mouse_position - frames[current_frame_index].getMousePos();
	frames[current_frame_index].setMousePos(mouse_position);
	for (int i = 1; i < pull_distance; i++) {
		int next_frame_idx = std::clamp(current_frame_index + i, 0, (int)frames.size() - 1);
		int prev_frame_idx = std::clamp(current_frame_index - i, 0, (int)frames.size() - 1);

		float current_pull_power = std::pow(pull_power, i * 1.2);

		frames[next_frame_idx].setMousePos(frames[next_frame_idx].getMousePos() + pos_change * current_pull_power);
		frames[prev_frame_idx].setMousePos(frames[prev_frame_idx].getMousePos() + pos_change * current_pull_power);
	}
}

void showPointStatistics(ReplayFrame frame) {
	ImGui::Begin("Point Information");
	ImGui::Text("Time: %d", frame.getAbsTime());
	ImGui::Text("[X,Y]: [%.0f, %.0f]", frame.getMousePos().x, frame.getMousePos().y);
	ImGui::Text("KeyState: %d", frame.getKeyState());
	ImGui::End();
}

void key_state_to_array(int keys, bool* key_states) {
	for (int i = 0; i < 5; i++) {
		int key_num = std::pow(2, (i));
		key_states[i] = keys & key_num;
	}
}

void drawFramesSubarray(sf::RenderWindow& window, std::vector<ReplayFrame>& frames_subarray, int selected_subframe_idx, int cursor_subframe_idx, bool draw_points, sf::CircleShape& frame_point, sf::Color& frame_point_color) {
	if (frames_subarray.size() == 0)
		return;
	//auto cur_mouse_pos = main_transform.transformPoint(frames_subarray[0].getMousePos());
	auto cur_mouse_pos = frames_subarray[0].getMousePos();
	bool cur_key_states[5] = {false, false, false, false, false};
	bool next_key_states[5] = { false, false, false, false, false };

	
	for (int j = 0; j < (int)frames_subarray.size() - 1; j++) {

		auto next_mouse_pos = frames_subarray[j + 1].getMousePos();
		key_state_to_array(frames_subarray[j].getKeyState(), cur_key_states);
		key_state_to_array(frames_subarray[j + 1].getKeyState(), next_key_states);

		if (cur_key_states[2] && cur_key_states[3])
			frame_point_color = sf::Color(177.f, 0.f, 132.f, 255.f);
		else if (cur_key_states[2] && !cur_key_states[3])
			frame_point_color = sf::Color(199.f, 0.f, 57.f, 255.f);
		else if (!cur_key_states[2] && cur_key_states[3])
			frame_point_color = sf::Color(0.f, 191.f, 255.f, 255.f);
		else
			frame_point_color = sf::Color(25.f, 25.f, 25.f, 255.f);
		
		sf::RectangleShape line = drawLineFromTwoPoints(cur_mouse_pos, next_mouse_pos);

		line.setFillColor(frame_point_color);
		frame_point.setFillColor(frame_point_color);

		frame_point.setPosition(next_mouse_pos - sf::Vector2f(0, 5));

		if (cur_key_states[2] != next_key_states[2]) {
			frame_point.setFillColor(sf::Color(199.f, 0.f, 57.f, 255.f));
			if ((cur_key_states[2] == True) && next_key_states[2] == False) {
				frame_point.setRotation(0);
			}
			else frame_point.setRotation(180);
			window.draw(frame_point);
		}
		if (cur_key_states[3] != next_key_states[3]) {
			frame_point.setFillColor(sf::Color(0.f, 191.f, 255.f, 255.f));
			if ((cur_key_states[3] == True) && next_key_states[3] == False) {
				frame_point.setRotation(0);
			}
			else frame_point.setRotation(180);
			window.draw(frame_point);
		}
		//frame_point.setR(sf::Vector2f(scale, scale));
		window.draw(line);

		cur_mouse_pos = next_mouse_pos;
	}
}