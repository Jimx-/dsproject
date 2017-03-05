#ifndef MAP_GENERATOR_H
#define MAP_GENERATOR_H

#include <vector>

class MapGenerator
{
public:
    struct Rect
    {
        int x, y;
        int width, height;
    };

    enum Tile
    {
        Unused		= '.',
        Floor		= ' ',
        Corridor	= ',',
        Wall		= '#',
        ClosedDoor	= '+',
        OpenDoor	= '-',
        Traps       = '$',
        Key         = 'K',
        Treasure_traps ='X',
        Torch       = '*',
        Player      = 'P',
        Spawn       = 'S',
    };

    enum Direction
    {
        North,
        South,
        West,
        East,
        DirectionCount
    };

    enum Difficulty{
        Easy = 10,
        Normal = 20,
        Difficult = 25
    };

public:
    MapGenerator(int width, int height);

    void generate(int maxFeatures, Difficulty h);

    void print();
    char getTile(int x, int y) const;
    void setTile(int x, int y, char tile);

    bool set_torch(int x, int y, char dir);

private:
    bool createFeature();

    bool createFeature(int x, int y, Direction dir);
    bool makeRoom(int x, int y, Direction dir, bool firstRoom = false);
    bool makeCorridor(int x, int y, Direction dir);
    bool placeRect(const Rect& rect, char tile);
    bool placeObject(char tile);

private:
    int _width, _height;
    std::vector<char> _tiles;
    std::vector<Rect> _rooms; // rooms for place stairs or monsters
    std::vector<Rect> _exits; // 4 sides of rooms or corridors
};

#endif
