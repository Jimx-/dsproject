//
// Created by jimx on 16-12-17.
//

#include "config.h"
#include "exception.h"

#include <fstream>

Map* g_map;
int g_map_width;
int g_map_height;
MapGenerator::Difficulty g_difficulty;

int g_screen_width;
int g_screen_height;
bool g_fullscreen;
int g_MSAA;
std::string g_font;

ConfigFile::ConfigFile() : root(nullptr)
{

}

ConfigFile::~ConfigFile()
{

}

void ConfigFile::load(const std::string& pathname)
{
    std::ifstream is;
    is.open(pathname.c_str(), std::ios::in | std::ios::binary);

    if (!is) {
        THROW_EXCEPT(E_FILE_NOT_FOUND, "ConfigFile::load()", "Config file '" + pathname + "' not found");
    }

    root.reset(new Json::Value());

    Json::Reader reader;
    reader.parse(is, *root);

    is.close();
}

