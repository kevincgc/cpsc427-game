#include "common.hpp"

#include <iostream>
#include <map>
#include <sstream>

// Note, we could also use the functions from GLM but we write the transformations here to show the uderlying math
void Transform::scale(vec2 scale)
{
	mat3 S = { { scale.x, 0.f, 0.f },{ 0.f, scale.y, 0.f },{ 0.f, 0.f, 1.f } };
	mat = mat * S;
}

void Transform::rotate(float radians)
{
	float c = cosf(radians);
	float s = sinf(radians);
	mat3 R = { { c, s, 0.f },{ -s, c, 0.f },{ 0.f, 0.f, 1.f } };
	mat = mat * R;
}

void Transform::translate(vec2 offset)
{
	mat3 T = { { 1.f, 0.f, 0.f },{ 0.f, 1.f, 0.f },{ offset.x, offset.y, 1.f } };
	mat = mat * T;
}

void Transform::reflect() {
	mat3 R = { { -1.f, 0.f, 0.f },{ 0.f, 1.f, 0.f },{ 0.f, 0.f, 1.f } };
	mat = mat * R;
}

bool gl_has_errors()
{
	GLenum error = glGetError();

	if (error == GL_NO_ERROR) return false;

	while (error != GL_NO_ERROR)
	{
		const char* error_str = "";
		switch (error)
		{
		case GL_INVALID_OPERATION:
			error_str = "INVALID_OPERATION";
			break;
		case GL_INVALID_ENUM:
			error_str = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error_str = "INVALID_VALUE";
			break;
		case GL_OUT_OF_MEMORY:
			error_str = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error_str = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}

		fprintf(stderr, "OpenGL: %s", error_str);
		error = glGetError();
		assert(false);
	}

	return true;
}

entt::registry registry;


// ============ Mouse Gestures ============

void Mouse_spell::reset_swipe_status(std::map<std::string,bool> &map, std::string except_button, std::string except_dir) {
	if (except_button == "RMB") {
		if (except_dir == "up") { map["gesture_LMB_down"] = false, map["gesture_LMB_right"] = false, map["gesture_LMB_left"] = false; }
		else if (except_dir == "down") { map["gesture_LMB_up"] = false, map["gesture_LMB_right"] = false, map["gesture_LMB_left"] = false; }
		else if (except_dir == "right") { map["gesture_LMB_down"] = false, map["gesture_LMB_up"] = false, map["gesture_LMB_left"] = false; }
		else if (except_dir == "left") { map["gesture_LMB_down"] = false, map["gesture_LMB_right"] = false, map["gesture_LMB_up"] = false; }
		else {map["gesture_LMB_down"] = false, map["gesture_LMB_right"] = false, map["gesture_LMB_left"] = false; map["gesture_LMB_up"] = false; }
	}
	else if (except_button == "LMB") {
		if (except_dir == "up") { map["gesture_RMB_down"] = false, map["gesture_RMB_right"] = false, map["gesture_RMB_left"] = false; }
		else if (except_dir == "down") { map["gesture_RMB_up"] = false, map["gesture_RMB_right"] = false, map["gesture_RMB_left"] = false; }
		else if (except_dir == "right") { map["gesture_RMB_down"] = false, map["gesture_RMB_up"] = false, map["gesture_RMB_left"] = false; }
		else if (except_dir == "left") { map["gesture_RMB_down"] = false, map["gesture_RMB_right"] = false, map["gesture_RMB_up"] = false; }
		else { map["gesture_RMB_down"] = false, map["gesture_RMB_right"] = false, map["gesture_RMB_left"] = false; map["gesture_RMB_up"] = false; }
	}
}

void Mouse_spell::check_spell(std::queue<std::string> &gesture_queue, std::map < int, std::map <std::string, std::string>> &spellbook, bool flag_fast) {

	std::string first_gesture;
	std::string second_gesture;

	if (gesture_queue.size() > 1) {
		first_gesture = gesture_queue.front();
		gesture_queue.pop();
		second_gesture = gesture_queue.front();
		gesture_queue.pop();
	}

	for (auto &spell : spellbook) {
		// Special case slow-down can have LMB / RMB down in any order. For now the spell is implented in world_system

		// First, check whether the first two gestures match the combination needed per spellbook
		if (first_gesture == spell.second["combo_1"] && second_gesture == spell.second["combo_2"]) {

			// Next, check whether speeds match the needed speed
			// Temporary: For now this only looks at the second swipe speed, not the speed of both swipes
			if (spell.second["speed"] == "fast" && flag_fast == true) {
				std::cout << "Cast " << spell.second["name"] << std::endl;
				spell.second["active"] = "true";
			}
			else if (spell.second["speed"] == "slow" && flag_fast == false) {
				std::cout << "Cast " << spell.second["name"] << std::endl;
				spell.second["active"] = "true";
			}
			else if (spell.second["speed"] == "none") {
				std::cout << "Cast " << spell.second["name"] << std::endl;
				spell.second["active"] = "true";
			}
		}
	}
}

void Mouse_spell::update_datastructs(std::map<std::string, bool> &gesture_statuses, std::queue<std::string> &gesture_queue, std::vector<vec2> &gesture_coords, std::string mouse_button, bool &flag_fast, float elapsed_ms) {
	// Forgiveness range
	// The leniency refers to whether the cursor x or y value strays too far from the starting point
	// Example: start: (100,100). Intent: Up motion. End: (110,40). Interpretation: 10 right, 60 up.
	// Is 10 right okay? If leniency is 20, then yes, 10 is acceptable, and we register this as swiping up.
	float forgiveness_range = 100;
	// The distance the cursor must travel to register as a swipe
	float min_distance = 100;
	// The threshold speed to be considered a fast or slow swipe
	float speed_threshold = 180;

	vec2 first = { gesture_coords.front().x , gesture_coords.front().y };
	vec2 last = { gesture_coords.back().x, gesture_coords.back().y };
	float dif_x = last.x - first.x;
	float dif_y = last.y - first.y;
	
	// Determine the swipe direction
	std::string gesture_button = (mouse_button == "RMB") ? "gesture_RMB_" : "gesture_LMB_";
	std::string leave_alone = (mouse_button == "RMB") ? "LMB" : "RMB";
	if (dif_x > min_distance && abs(dif_y) < forgiveness_range) {
		std::cout << mouse_button << "_swipe_right" << std::endl;
		gesture_statuses[gesture_button + "right"] = true;
		gesture_queue.push(gesture_button + "right");
		reset_swipe_status(gesture_statuses, leave_alone, "right");
	}
	else if (dif_x < -1 * min_distance && abs(dif_y) < forgiveness_range) {
		std::cout << mouse_button << "_swipe_left" << std::endl;
		gesture_statuses[gesture_button + "left"] = true;
		gesture_queue.push(gesture_button + "left");
		reset_swipe_status(gesture_statuses, leave_alone, "left");
	}
	else if (abs(dif_x) < forgiveness_range && dif_y > min_distance) {
		std::cout << mouse_button << "_swipe_down" << std::endl;
		gesture_statuses[gesture_button + "down"] = true;
		gesture_queue.push(gesture_button + "down");
		reset_swipe_status(gesture_statuses, leave_alone, "down");
	}
	else if (abs(dif_x) < forgiveness_range && dif_y < -1 * min_distance) {
		std::cout << mouse_button << "_swipe_up" << std::endl;
		gesture_statuses[gesture_button + "up"] = true;
		gesture_queue.push(gesture_button + "up");
		reset_swipe_status(gesture_statuses, leave_alone, "up");
	}

	// Determine the swipe speed
	if ((abs(dif_x) > min_distance  || abs(dif_y) > min_distance) && elapsed_ms < speed_threshold) {
		// Debug
		std::cout << "Fast swipe!" << std::endl;
		flag_fast = true;
	}
	
	// Clear the vector holding the gesture coordinates
	gesture_coords.clear();
}

void Mouse_spell::reset_spells(std::map<int, std::map<std::string, std::string>>& spellbook)
{
	// Set every spell's active status to "false"
	for (auto &spell : spellbook) { spell.second["active"] = "false";}
}

ProgramState state = ProgramState::INIT;


void Subject::addObserver(Observer* observer)
{
	if (numObservers_ < MAX_OBS) {
		observers_[numObservers_++] = observer;
	}
	else {
		std::cout << "too many observers" << std::endl;
	}
}

void Subject::removeObserver(Observer* observer)
{
	for (int i = 0; i < MAX_OBS; i++) {
		if (observers_[i] == observer) {
			observers_[i] = nullptr;
		}
	}
}

void Subject::notify(const entt::entity& entity, const entt::entity& other, Event event)
{
	for (int i = 0; i < numObservers_; i++)
	{
		observers_[i]->onNotify(entity, other, event);
	}
}