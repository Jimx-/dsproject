//
// Created by jimx on 16-12-18.
//

#ifndef WEEABOO_MAP_H
#define WEEABOO_MAP_H

#include "mesh.h"
#include "map_generator.h"
#include <memory>
#include <btBulletDynamicsCommon.h>

class Map : public Renderable {
public:
    Map(int width, int height);

    void draw(Renderer& renderer);
    static const float TILE_SIZE;

	char get_tile(int i, int j) { return generator.getTile(i, j); }
private:

    int width, height;
    std::unique_ptr<Mesh> map_mesh;
    MapGenerator generator;
    std::unique_ptr<btTriangleMesh> rigid_mesh;
    std::unique_ptr<btRigidBody> rigid_body;

    void setup_mesh();
};

#endif //WEEABOO_MAP_H
