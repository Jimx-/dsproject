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
	virtual void intrinsic_transform(Renderer&) { }

    BaseCharacter(glm::vec3 pos);

    AnimationModel* model;
    glm::vec3 pos;
};

using PCharacter = std::shared_ptr<BaseCharacter>;

class SkeletonCharacter : public BaseCharacter {
private:
    static PModel _prepare_model();
    virtual AnimationModel* get_model() const override;
	virtual void intrinsic_transform(Renderer& renderer);

public:
    SkeletonCharacter(glm::vec3 pos);
};

class TrapItem : public BaseCharacter {
private:
	static PModel _prepare_model();
	virtual AnimationModel* get_model() const override;
	virtual void intrinsic_transform(Renderer& renderer);

public:
	TrapItem(glm::vec3 pos);
};

class ChestTrapItem : public BaseCharacter {
private:
	static PModel _prepare_model();
	virtual AnimationModel* get_model() const override;
	virtual void intrinsic_transform(Renderer& renderer);

public:
	ChestTrapItem(glm::vec3 pos);
};

class TorchItem : public BaseCharacter {
private:
	static PModel _prepare_model();
	virtual AnimationModel* get_model() const override;
	virtual void intrinsic_transform(Renderer& renderer);

public:
	TorchItem(glm::vec3 pos);
};

#endif
