//
// Created by jimx on 16-12-18.
//

#ifndef WEEABOO_MAP_H
#define WEEABOO_MAP_H

#include "mesh.h"
#include "map_generator.h"
#include <memory>

class Map : public Renderable {
public:
    Map(int width, int height);

    void draw(Renderer& renderer);
    static const float TILE_SIZE;
private:


    int width, height;
    std::unique_ptr<Mesh> map_mesh;
    MapGenerator generator;

    void setup_mesh();
};

#endif //WEEABOO_MAP_H
