#ifndef DSPROJECT_CHARACTER_MANAGER_H
#define DSPROJECT_CHARACTER_MANAGER_H

#include "characters.h"
#include "singleton.h"

#include <type_traits>
#include <vector>

class CharacterManager : public Singleton<CharacterManager> {
public:
	CharacterManager();

    template <typename T, typename ... Args>
    typename std::enable_if<std::is_base_of<BaseCharacter, typename std::decay<T>::type>::value>::type spawn(Args&&... args)
    {
        chars.push_back(std::shared_ptr<T>(new T(std::forward<Args>(args)...)));
    }

	template <typename T, typename ... Args>
	typename std::enable_if<std::is_base_of<BaseCharacter, typename std::decay<T>::type>::value>::type spawn_item(Args&&... args)
	{
		items.push_back(std::shared_ptr<T>(new T(std::forward<Args>(args)...)));
	}

    void submit(Renderer& renderer) const;
	void update(float dt);

	MainCharacter& main_char() { return *main_character; }

private:
	std::unique_ptr<MainCharacter> main_character;

    std::vector<PCharacter> chars;
	std::vector<PCharacter> items;
};

#define CHARACTER_MANAGER CharacterManager::get_singleton()

#endif
