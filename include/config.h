//
// Created by jimx on 16-12-17.
//

#ifndef WEEABOO_CONFIG_H
#define WEEABOO_CONFIG_H

#include "json/json.h"
#include "string_utils.h"
#include "map.h"

#include <memory>

class ConfigFile {
public:
    ConfigFile();
    ~ConfigFile();

    void load(const std::string& pathname);

    std::shared_ptr<Json::Value> get_root() const { return root; }
private:
    std::shared_ptr<Json::Value> root;
};

extern Map* g_map;
extern int g_map_width;
extern int g_map_height;
extern MapGenerator::Difficulty g_difficulty;

extern int g_screen_width;
extern int g_screen_height;
extern bool g_fullscreen;
extern int g_MSAA;

#endif //WEEABOO_CONFIG_H
