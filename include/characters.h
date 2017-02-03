#ifndef WEEABOO_CHARACTERS_H
#define WEEABOO_CHARACTERS_H

#include "renderable.h"
#include "animation_model.h"

class BaseCharacter : public Renderable {
public:
    virtual void draw(Renderer& renderer) override;

protected:
    void init_model();
    virtual AnimationModel* get_model() const = 0;
    virtual float get_scale_factor() const = 0;

    BaseCharacter(glm::vec3 pos);

    AnimationModel* model;
    glm::vec3 pos;
};

using PCharacter = std::shared_ptr<BaseCharacter>;

class SkeletonCharacter : public BaseCharacter {
private:
    static PModel _prepare_model();
    virtual AnimationModel* get_model() const override;
    virtual float get_scale_factor() const { return 0.02f; }

public:
    SkeletonCharacter(glm::vec3 pos);
};

#endif
