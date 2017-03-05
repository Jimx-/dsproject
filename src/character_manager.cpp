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
