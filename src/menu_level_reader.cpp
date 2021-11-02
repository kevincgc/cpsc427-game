#include <string>
#include <iostream>
#include <filesystem>
#include "common.hpp"
#include "components.hpp"

extern "C" char **get_menu_options(int *num_options);
extern "C" void clear_menu_options(char **levels, int num_options);
extern "C" void set_level(char *level);

char **get_menu_options(int *num_options) {
    std::string path = levels_path("");

    std::vector<std::string> vec;
    // https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
    for (const auto &entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_directory()) {
            vec.push_back(entry.path().filename().string());
        }
    }
    *num_options = vec.size();

    // create array of same size as vector
    char **res = new char*[*num_options];
    for (int i = 0; i < *num_options; i++) {
        std::string str = vec[i];
        size_t str_size = str.size() + 1;
        res[i] = new char[str_size];
        strcpy(res[i], str.c_str());
    }

    return res;
}

void clear_menu_options(char **levels, int num_options) {
    for (int i = 0; i < num_options; i++) {
        delete[] levels[i];
    }

    delete[] levels;
}

void set_level(char *level) {
    game_state.level_id = std::string(level);
}
