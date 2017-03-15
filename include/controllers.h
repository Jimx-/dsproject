#ifndef WEEABOO_CONTROLLERS_H
#define WEEABOO_CONTROLLERS_H

#include "renderer.h"
#include "map.h"

class Controller {
public:
    virtual void update_view(Renderer& renderer) = 0;

    virtual void handle_key(int key, int scancode, int action, int mode) = 0;
    virtual void handle_mouse(double xpos, double ypos) = 0;
};

class GameController : public Controller {
public:
    GameController();

    virtual void update_view(Renderer& renderer) override;

    virtual void handle_key(int key, int scancode, int action, int mode) override;
    virtual void handle_mouse(double xpos, double ypos) override;

private:
    std::shared_ptr<Map> map;
};

#endif
