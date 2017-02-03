#include "character_manager.h"

template<>
CharacterManager* Singleton<CharacterManager>::singleton = nullptr;

void CharacterManager::submit(Renderer& renderer) const
{
    for (auto& p : chars) {
        renderer.enqueue_renderable(p);
    }
	for (auto& p : items) {
		renderer.enqueue_renderable(p);
	}
}
