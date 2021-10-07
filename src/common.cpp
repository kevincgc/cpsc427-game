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


// ====== Mouse Gestures ======

//std::vector<vec2> Mouse_spell::load_gesture(std::string file_name) {
//	std::string file_path = "data/gestures/" + file_name;
//	std::vector<vec2> result = read_csv(file_path);
//	return result;
//}
//std::vector<vec2> circle_points = {
//
//}

//std::map < std::string, std::vector<vec2> > gesture_templates = {
//	{"circle", circle_points}
//}

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

		if (first_gesture == spell.second["combo_1"] && second_gesture == spell.second["combo_2"]) {
			if (spell.second["speed"] == "fast" && flag_fast == true) {
				std::cout << "Casted " << spell.second["name"] << std::endl;
				spell.second["active"] = "true";
			}
			else if (spell.second["speed"] == "slow" && flag_fast == false) {
				std::cout << "Casted " << spell.second["name"] << std::endl;
				spell.second["active"] = "true";
			}
			else if (spell.second["speed"] == "none") {
				std::cout << "Casted " << spell.second["name"] << std::endl;
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
	if (elapsed_ms < speed_threshold) {

		// Debug
		std::cout << "Fast!" << std::endl;

		flag_fast = true;
	}
	gesture_coords.clear();
}

void Mouse_spell::reset_spells(std::map<int, std::map<std::string, std::string>>& spellbook)
{
	// Set every spell's active status to "false"
	for (auto &spell : spellbook) { spell.second["active"] = "false";}
}

std::vector<vec2> Mouse_spell::pre_process(std::vector<vec2> &points, float square_size, bool debug) {
	// Debug: Write raw input
	if (debug) { write_csv("raw_circle.csv", points); }

	// In practice, we found N = 64 to be adequate, as was any 32 <= N <= 256.
	int n = 64;

	// Path-Length(points)
	float path_length = 0;
	for (int i = 1; i != points.size(); i++) {
		path_length = path_length + sqrt(pow(points[i].x - points[i - 1].x, 2) + pow(points[i].y - points[i - 1].y, 2));
	}

	// Resample
	// I = length of each increment between n new points
	float I = path_length / (n - 1);
	float D = 0;
	std::vector<vec2> resampled_points;
	// When the path is stepped through s.t. when the distance covered exceeds I,
	// a new point is added through linear interpolation.

	for (int i = 1; i != points.size(); i++) {
		float d = sqrt(pow(points[i].x - points[i - 1].x, 2) + pow(points[i].y - points[i - 1].y, 2));
		if ((D + d) >= I) {
			float q_x = points[i - 1].x + ((I - D) / d) * (points[i].x - points[i - 1].x);
			float q_y = points[i - 1].y + ((I - D) / d) * (points[i].y - points[i - 1].y);
			resampled_points.push_back({ q_x,q_y });
			points.insert(points.begin() + i, { q_x,q_y }); //q will be the next p[i]
			D = 0;
		}
		else {
			D = D + d;
		}
	}

	// Debug: Write resampled points
	if (debug) {
		write_csv("resampled_circle.csv", resampled_points);
	}

	//Rotate-To-Zero(resampled_points)
	float x_sum = 0;
	float y_sum = 0;
	for (auto &p : resampled_points) {
		x_sum += p.x;
		y_sum += p.y;
	}
	float x_bar = x_sum / resampled_points.size();
	float y_bar = y_sum / resampled_points.size();
	vec2 c = { x_bar, y_bar }; // the centroid(x,y)
	float theta = atan2(c.y - resampled_points[0].y, c.x - resampled_points[0].x)*3.14 / 180; // -3.14 <= theta <= 3.14
	std::vector<vec2> rotated_points;
	for (int i = 0; i != resampled_points.size(); i++) {
		float q_x = (resampled_points[i].x - c.x) * cos(theta) - (resampled_points[i].y - c.y)*sin(theta) + c.x;
		float q_y = (resampled_points[i].y - c.y) * sin(theta) - (resampled_points[i].y - c.y)*cos(theta) + c.y;
		rotated_points.push_back({ q_x,q_y });
	}

	// Debug: Write rotated points
	if (debug) {
		write_csv("rotated_points.csv", rotated_points);
	}

	// Scale
	float max_x = 0;
	float max_y = 0;
	float min_x = 10000;
	float min_y = 10000;
	std::vector<vec2> scaled_points;
	for (auto &p : rotated_points) {
		if (p.x > max_x) { max_x = p.x; }
		else if (p.x < min_x) { min_x = p.x; }
		if (p.y > max_y) { max_y = p.y; }
		else if (p.y < min_y) { min_y = p.y; }
	}
	float bounding_width = max_x - min_x;
	float bounding_height = max_y - min_y;
	for (int i = 0; i != rotated_points.size(); i++) {
		float q_x = rotated_points[i].x * (square_size / bounding_width);
		float q_y = rotated_points[i].y * (square_size / bounding_height);
		scaled_points.push_back({ q_x,q_y });
	}

	// Debug: Write scaled points
	if (debug) {
		write_csv("scaled_points.csv", scaled_points);
	}


	// Translate to Origin
	// It should not matter where on the sreen you draw
	std::vector<vec2> translated_points;
	float x_sum2 = 0;
	float y_sum2 = 0;
	for (auto &p : scaled_points) {
		x_sum2 += p.x;
		y_sum2 += p.y;
	}
	float x_bar2 = x_sum2 / scaled_points.size();
	float y_bar2 = y_sum2 / scaled_points.size();
	vec2 c2 = { x_bar2, y_bar2 }; // the centroid(x,y)
	for (int i = 0; i != scaled_points.size(); i++) {
		float q_x = scaled_points[i].x - c2.x;
		float q_y = scaled_points[i].y - c2.y;
		translated_points.push_back({ q_x,q_y });
	}

	// Debug: Write translated points
	if (debug) {
		write_csv("translated_points.csv", translated_points);
	}

	return translated_points;

}

void Mouse_spell::gesture_implementation(std::vector<vec2> &points) {
	// Settings
	float square_size = 500;

	// Debug: Right now we'll just have one item in templates
	std::vector<vec2> circle_template_points = {
		{200.0, 100.0},
		{199.8027, 106.2791},
		{199.2115, 112.5333},
		{198.2287, 118.7381},
		{196.8583, 124.869},
		{195.1057, 130.9017},
		{192.9776, 136.8125},
		{190.4827, 142.5779},
		{187.6307, 148.1754},
		{184.4328, 153.5827},
		{180.9017, 158.7785},
		{177.0513, 163.7424},
		{172.8969, 168.4547},
		{168.4547, 172.8969},
		{163.7424, 177.0513},
		{158.7785, 180.9017},
		{153.5827, 184.4328},
		{148.1754, 187.6307},
		{142.5779, 190.4827},
		{136.8125, 192.9776},
		{130.9017, 195.1057},
		{124.869, 196.8583},
		{118.7381, 198.2287},
		{112.5333, 199.2115},
		{106.2791, 199.8027},
		{100.0, 200.0},
		{93.7209, 199.8027},
		{87.4667, 199.2115},
		{81.2619, 198.2287},
		{75.131, 196.8583},
		{69.0983, 195.1057},
		{63.1875, 192.9776},
		{57.4221, 190.4827},
		{51.8246, 187.6307},
		{46.4173, 184.4328},
		{41.2215, 180.9017},
		{36.2576, 177.0513},
		{31.5453, 172.8969},
		{27.1031, 168.4547},
		{22.9487, 163.7424},
		{19.0983, 158.7785},
		{15.5672, 153.5827},
		{12.3693, 148.1754},
		{9.5173, 142.5779},
		{7.0224, 136.8125},
		{4.8943, 130.9017},
		{3.1417, 124.869},
		{1.7713, 118.7381},
		{0.7885, 112.5333},
		{0.1973, 106.2791},
		{0.0, 100.0},
		{0.1973, 93.7209},
		{0.7885, 87.4667},
		{1.7713, 81.2619},
		{3.1417, 75.131},
		{4.8943, 69.0983},
		{7.0224, 63.1875},
		{9.5173, 57.4221},
		{12.3693, 51.8246},
		{15.5672, 46.4173},
		{19.0983, 41.2215},
		{22.9487, 36.2576},
		{27.1031, 31.5453},
		{31.5453, 27.1031},
		{36.2576, 22.9487},
		{41.2215, 19.0983},
		{46.4173, 15.5672},
		{51.8246, 12.3693},
		{57.4221, 9.5173},
		{63.1875, 7.0224},
		{69.0983, 4.8943},
		{75.131, 3.1417},
		{81.2619, 1.7713},
		{87.4667, 0.7885},
		{93.7209, 0.1973},
		{100.0, 0.0},
		{106.2791, 0.1973},
		{112.5333, 0.7885},
		{118.7381, 1.7713},
		{124.869, 3.1417},
		{130.9017, 4.8943},
		{136.8125, 7.0224},
		{142.5779, 9.5173},
		{148.1754, 12.3693},
		{153.5827, 15.5672},
		{158.7785, 19.0983},
		{163.7424, 22.9487},
		{168.4547, 27.1031},
		{172.8969, 31.5453},
		{177.0513, 36.2576},
		{180.9017, 41.2215},
		{184.4328, 46.4173},
		{187.6307, 51.8246},
		{190.4827, 57.4221},
		{192.9776, 63.1875},
		{195.1057, 69.0983},
		{196.8583, 75.131},
		{198.2287, 81.2619},
		{199.2115, 87.4667},
		{199.8027, 93.7209},
		{200.0, 100.0}
	};
	std::vector<vec2> lightning_template_points = {
		{390, 120},
		{388, 124},
		{382, 137},
		{370, 157},
		{362, 169},
		{348, 196},
		{333, 222},
		{321, 244},
		{314, 262},
		{310, 276},
		{308, 284},
		{307, 285},
		{311, 285},
		{329, 280},
		{343, 277},
		{384, 271},
		{433, 267},
		{482, 267},
		{530, 271},
		{567, 276},
		{585, 282},
		{589, 286},
		{589, 299},
		{589, 315},
		{585, 336},
		{577, 359},
		{564, 388},
		{546, 420},
		{527, 452},
		{510, 478},
		{499, 495},
		{495, 505},
		{493, 508},
		{492, 513},
		{491, 517},
		{490, 519},
		{490, 521}
	};
	std::map<std::string, std::vector<vec2>> templates = {
		{"circle", circle_template_points},
		{"lightning", lightning_template_points}
	};

	std::vector<vec2> translated_points = pre_process(points, square_size, true);

	// Recognize
	double b = std::numeric_limits<double>::infinity();
	std::string best_T_name = "";
	double theta2 = 45 * 3.14/180; //rads
	double neg_theta = -45 * 3.14/180; //rads
	double theta_delta = 2 * 3.14/180; //rads
	double phi = 0.5*(-1 + sqrt(5));

	float f_1;
	float f_2;

	for (auto &T : templates) {
		std::vector<vec2> T_points = pre_process(T.second, square_size, false);

		// Calculate distance-at-best-angle(points,T,-theta,theta,theta_delta)
		float x_1 = phi * neg_theta + (1 - phi)*theta2;
		f_1 = distance_at_angle(translated_points, T_points, x_1);
		float x_2 = 1 - phi * neg_theta + phi * theta2;
		f_2 = distance_at_angle(translated_points, T_points, x_2);
		while (abs(theta2 - neg_theta) > theta_delta) {
			if (f_1 < f_2) {
				theta2 = x_2;
				x_2 = x_1;
				f_2 = f_1;
				x_1 = phi * neg_theta + (1 - phi)*theta2;
				f_1 = distance_at_angle(translated_points, T_points, x_1);
			}
			else {
				neg_theta = x_1;
				x_1 = x_2;
				f_1 = f_2;
				x_2 = (1 - phi)*neg_theta + phi * theta2;
				f_2 = distance_at_angle(translated_points, T_points, x_2);
			}
		}

		float dist_at_best_angle = min(f_1, f_2);
		if (dist_at_best_angle < b) {
			b = dist_at_best_angle;
			best_T_name = T.first;
		}
	}
	float score = 1 - b / (0.5*sqrt(2 * pow(square_size, 2)));

	std::cout << "Identified: " << best_T_name << " with score: " << score << std::endl;


}

float Mouse_spell::distance_at_angle(std::vector<vec2> points, std::vector<vec2> T_points, float theta)
{
	std::vector<vec2> new_points = rotate_by(points, theta);
	float d = path_distance(new_points, T_points);
	return d;
}

float Mouse_spell::path_distance(std::vector<vec2>points, std::vector<vec2>T_points) {
	float d = 0;
	for (int i = 0; i != points.size(); i++) {
		d += sqrt(pow(points[i].x - T_points[i].x, 2) + pow(points[i].y - T_points[i].y, 2));
	}
	return d / points.size();
}

std::vector<vec2> Mouse_spell::rotate_by(std::vector<vec2> points, float theta) {
	float x_sum = 0;
	float y_sum = 0;
	for (auto &p : points) {
		x_sum += p.x;
		y_sum += p.y;
	}
	float num_eles = points.size();
	float x_bar = x_sum / num_eles;
	float y_bar = y_sum / num_eles;
	vec2 c = { x_bar, y_bar }; // the centroid(x,y)
	std::vector<vec2> new_points;
	for (int i = 0; i != points.size(); i++) {
		float q_x = (points[i].x - c.x) * cos(theta) - (points[i].y - c.y)*sin(theta) + c.x;
		float q_y = (points[i].y - c.y) * sin(theta) - (points[i].y - c.y)*cos(theta) + c.y;
		new_points.push_back({ q_x,q_y });
	}
	return new_points;
}

void Mouse_spell::write_csv(std::string filename, std::vector<vec2> points) {

	filename = "C:/Users/SKu T-Type/Desktop/427 Assignments/Project/Github/team10/data/gestures/" + filename;
	std::ofstream outputFile;
	std::ofstream fs;
	outputFile.open(filename, std::ios::out);
	if (outputFile) {
		for (auto &p : points) {
			outputFile << p.x << ", " << p.y << std::endl;
		}
	}
	else {
		std::cout << " no outputFile" << std::endl;
	}

	outputFile.close();
}