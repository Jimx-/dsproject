#ifndef DSPROJECT_CONTROLLERS_H
#define DSPROJECT_CONTROLLERS_H

#include "renderer.h"
#include "map.h"
#include "gui.h"

class Controller {
public:
    static void switch_controller(const std::string& name);

    virtual void update_view(Renderer& renderer) = 0;

    virtual void handle_key(int key, int scancode, int action, int mode) = 0;
    virtual void handle_mouse(double xpos, double ypos) = 0;

    virtual void enter() { }
    virtual void exit() { }
};

class GameController : public Controller {
public:
    GameController();

    virtual void update_view(Renderer& renderer) override;

    virtual void handle_key(int key, int scancode, int action, int mode) override;
    virtual void handle_mouse(double xpos, double ypos) override;

    virtual void enter() override;
    virtual void exit() override;
private:
    std::shared_ptr<Map> map;

    std::shared_ptr<GUILabel> hpbar;
};

class WidgetController : public Controller {
public:
    virtual void update_view(Renderer& ruenderer) override;

    virtual void handle_key(int key, int scancode, int action, int mode) override;
    virtual void handle_mouse(double xpos, double ypos) override;

protected:
    WidgetController(const std::string& bg);

    void add_widget(PGUIWidget widget);

private:
    POverlay background;
    float layout_y;
    std::vector<PGUIWidget> widgets;
};

class MainMenuController : public WidgetController {
public:
    MainMenuController();
};

class InGameMenuController : public WidgetController {
public:
    InGameMenuController();
};

class EndGameController : public WidgetController {
public:
    EndGameController();

    virtual void enter() override;

private:
    std::shared_ptr<GUILabel> title;
    std::shared_ptr<GUILabel> score;
};

#endif
