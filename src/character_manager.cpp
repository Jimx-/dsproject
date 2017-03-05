#include "character_manager.h"

template<>
CharacterManager* Singleton<CharacterManager>::singleton = nullptr;

CharacterManager::CharacterManager()
{
    main_character.reset(new MainCharacter());
}

void CharacterManager::submit(Renderer& renderer) const
{
    for (auto& p : chars) {
        renderer.enqueue_renderable(p);
    }
	for (auto& p : items) {
		renderer.enqueue_renderable(p);
	}
}

void CharacterManager::update(float dt)
{
	main_character->update(dt);
	for (auto& p : chars) {
		p->update(dt);
	}
	for (auto& p : items) {
		p->update(dt);
	}
}