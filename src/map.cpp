//
// Created by jimx on 16-12-18.
//

#include "map.h"
#include "config.h"
#include "character_manager.h"
#include "particle_system.h"
#include "random_utils.h"
#include "simulation.h"

#include <glm/gtc/type_ptr.hpp>
#include <random>
#include <iostream>

using namespace std;
const float Map::TILE_SIZE = 1.5f;

Map::Map(int width, int height) : width(width), height(height), generator(width, height)
{
    generator.generate(width, g_difficulty);
    generator.print();
    setup_mesh();
}

void Map::draw(Renderer& renderer)
{
    renderer.uniform(ShaderProgram::BONE_TRANSFORMS, 1, false, glm::value_ptr(glm::mat4()));
    map_mesh->draw(renderer);
}

void Map::setup_mesh()
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    for(GLuint i = 0; i < width; i++){
        for (GLuint j = 0; j < height; j++) {

            char tile = generator.getTile(i, j);
            if (tile == MapGenerator::Tile::Floor || tile == MapGenerator::Spawn || tile == MapGenerator::Traps || 
				tile == MapGenerator::Torch || tile == MapGenerator::Treasure_traps || tile == MapGenerator::ClosedDoor ||
				tile == MapGenerator::OpenDoor || tile == MapGenerator::Player || tile == MapGenerator::Corridor ||
                tile == MapGenerator::Key) {
                for (int h = 0; h < 2; h++) {
                    int base = vertices.size();
                    for (int x = 0; x < 2; x++) {
                        for (int y = 0; y < 2; y++) {
                            Vertex vertex;
#define SET_VERTEX(n, v) vertex.position[n] = v
                            SET_VERTEX(0, (i + x) * TILE_SIZE);
                            SET_VERTEX(1, h * TILE_SIZE * 2);
                            SET_VERTEX(2, (j + y) * TILE_SIZE);
#define SET_NORMAL(n, v) vertex.normal[n] = v
                            SET_NORMAL(0, 0.0f);
                            SET_NORMAL(1, (h > 0 ? -1 : 1) * 1.0f);
                            SET_NORMAL(2, 0.0f);
#define SET_TANGENT(n, v) vertex.tangent[n] = v
                            SET_TANGENT(0, 1.0f);
                            SET_TANGENT(1, 0.0f);
                            SET_TANGENT(2, 0.0f);

                            vertex.tex_coord[0] = x * 0.5f;
                            vertex.tex_coord[1] = y * 0.5f;

                            vertex.add_bone_data(0, 1.0f);
                            vertices.push_back(vertex);
                        }
                    }
                    indices.push_back(base + 0);
                    indices.push_back(base + 1);
                    indices.push_back(base + 2);
                    //RIGID_ADD_TRIANGLE(base + 0, base + 1, base + 2);

                    indices.push_back(base + 1);
                    indices.push_back(base + 3);
                    indices.push_back(base + 2);
                    //RIGID_ADD_TRIANGLE(base + 1, base + 3, base + 2);
                }
                if (tile == MapGenerator::Spawn) {
                    CHARACTER_MANAGER.spawn<SkeletonCharacter>(glm::vec3((float)i * TILE_SIZE, 0.0f, (float)j * TILE_SIZE));
				} else if (tile == MapGenerator::Tile::Traps) {
					CHARACTER_MANAGER.spawn_item<TrapItem>(glm::vec3((float)i * TILE_SIZE, 0.0f, (float)j * TILE_SIZE));
				} else if (tile == MapGenerator::Tile::Torch) {
					RENDERER.add_light(glm::vec3((i + 0.5f) * TILE_SIZE, 1.0f, (j + 0.5f) * TILE_SIZE), glm::vec3(1.0f, 0.57f, 0.16f), 0.1f, 0.1f);
					CHARACTER_MANAGER.spawn_item<TorchItem>(glm::vec3((i + 0.5f) * TILE_SIZE, 0.0f, (j + 0.5) * TILE_SIZE));
					PARTICLE_SYSTEM.spawn_particle<FlameParticle>(glm::vec3{ (i + 0.5f) * TILE_SIZE, 1.15f, (j + 0.5f) * TILE_SIZE });
				} else if (tile == MapGenerator::Tile::Treasure_traps) {
					CHARACTER_MANAGER.spawn_item<ChestTrapItem>(glm::vec3((i - 0.5f) * TILE_SIZE, 0.0f, (j + 0.5f) * TILE_SIZE));
				} else if (tile == MapGenerator::Tile::Floor) {
					if (generator.getTile(i + 1, j) == MapGenerator::Wall ||
						generator.getTile(i - 1, j) == MapGenerator::Wall ||
						generator.getTile(i, j - 1) == MapGenerator::Wall ||
						generator.getTile(i, j + 1) == MapGenerator::Wall) {
						float prob = RandomUtils::random_int(0, 1000) / 1000.0f;
						if (prob < 0.1f) {
							CHARACTER_MANAGER.spawn_item<BarrelItem>(glm::vec3((i + 0.5f) * TILE_SIZE, 0.0f, (j + 0.5) * TILE_SIZE));
						}
					}
				} else if (tile == MapGenerator::Tile::Player) {
                    CHARACTER_MANAGER.main_char().set_position(glm::vec3{ i * TILE_SIZE, 2.0f, j * TILE_SIZE });
                } else if (tile == MapGenerator::Tile::Key) {
                    CHARACTER_MANAGER.spawn_item<ChestKeyItem>(
                        glm::vec3((i - 0.5f) * TILE_SIZE, 0.0f, (j + 0.5f) * TILE_SIZE));
                }
            } else if (tile == MapGenerator::Tile::Wall) {
                float start_x = i * TILE_SIZE;
                float start_z = j * TILE_SIZE;

                int dx[] = {1, 0, -1, 0};
                int dz[] = {0, 1, 0, -1};

                btCollisionShape* shape = new btBoxShape(btVector3(0.5 * TILE_SIZE, 10, 0.5 * TILE_SIZE));
                btDefaultMotionState* motion_state =
                    new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(start_x + 0.5 * TILE_SIZE, 10, start_z + 0.5 * TILE_SIZE)));
                btScalar mass = 0;
                btVector3 inertia(0, 0, 0);
                shape->calculateLocalInertia(mass, inertia);
                btRigidBody::btRigidBodyConstructionInfo CI(mass, motion_state, shape, inertia);
                btRigidBody* wall_rigid = new btRigidBody(CI);
                SIMULATION.add_rigidbody(wall_rigid);

                for (int k = 0; k < 4; k++) {
                    float x1 = start_x;
                    float z1 = start_z;
                    float x2 = start_x + dx[k] * TILE_SIZE;
                    float z2 = start_z + dz[k] * TILE_SIZE;
                    int base = vertices.size();
                    for (int h = 0; h < 2; h++) {
                        Vertex v1;
                        v1.position[0] = x1;
                        v1.position[1] = (float) h * 2 * TILE_SIZE;
                        v1.position[2] = z1;
                        v1.normal[0] = dz[k];
                        v1.normal[1] = 0.0f;
                        v1.normal[2] = -dx[k];
                        v1.tangent[0] = dx[k];
                        v1.tangent[1] = 0.0f;
                        v1.tangent[2] = dz[k];

                        v1.tex_coord[0] = 0.5f;
                        v1.tex_coord[1] = h;

                        v1.add_bone_data(0, 1.0f);
                        vertices.push_back(v1);

                        Vertex v2;
                        v2.position[0] = x2;
                        v2.position[1] = (float) h * 2 * TILE_SIZE;
                        v2.position[2] = z2;
                        v2.normal[0] = dz[k];
                        v2.normal[1] = 0.0f;
                        v2.normal[2] = -dx[k];
                        v2.tangent[0] = dx[k];
                        v2.tangent[1] = 0.0f;
                        v2.tangent[2] = dz[k];

                        v2.tex_coord[0] = 1.0;
                        v2.tex_coord[1] = h;

                        v2.add_bone_data(0, 1.0f);
                        vertices.push_back(v2);
                    }

					indices.push_back(base);
					indices.push_back(base + 1);
					indices.push_back(base + 2);

					indices.push_back(base + 1);
					indices.push_back(base + 3);
					indices.push_back(base + 2);

                    start_x = x2;
                    start_z = z2;
                }
            }
        }
    }

    PMaterial material(new Material(0.8f, 0.1f, "dungeon.png", "dungeon_normal_map.png"));

    Mesh::BoneMapping bones;

    map_mesh.reset(new Mesh(nullptr, vertices, indices, material, bones, glm::mat4()));
}


