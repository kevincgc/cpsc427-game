#include "common.hpp"

#include <iostream>
#include <map>

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

void Mouse_spell::check_swipe_count(std::map<std::string, bool>& map, int & count, int max_swipes) {
	if (count == max_swipes) {
		std::cout << "Reseting swipes..." << std::endl;
		reset_swipe_status(map);
		count = 0;
	}
}

void Mouse_spell::check_spell(std::map<std::string, bool> &map) {
	if (map["gesture_LMB_down"] && map["gesture_RMB_right"]) {
		std::cout << "Invincibility!" << std::endl;reset_swipe_status(map);
		map["gesture_RMB_down"] = false, map["gesture_RMB_right"] = false, map["gesture_RMB_left"] = false; map["gesture_RMB_up"] = false;
		map["gesture_LMB_down"] = false, map["gesture_LMB_right"] = false, map["gesture_LMB_left"] = false; map["gesture_LMB_up"] = false;
	}
	else if (map["gesture_LMB_up"] && map["gesture_RMB_left"]) {
		std::cout << "Speed!" << std::endl;
		map["gesture_RMB_down"] = false, map["gesture_RMB_right"] = false, map["gesture_RMB_left"] = false; map["gesture_RMB_up"] = false;
		map["gesture_LMB_down"] = false, map["gesture_LMB_right"] = false, map["gesture_LMB_left"] = false; map["gesture_LMB_up"] = false;
	}
}