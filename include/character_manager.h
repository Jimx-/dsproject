#ifndef WEEABOO_CHARACTER_MANAGER_H
#define WEEABOO_CHARACTER_MANAGER_H

#include "characters.h"
#include "singleton.h"

#include <type_traits>
#include <vector>

class CharacterManager : public Singleton<CharacterManager> {
public:
    template <typename T, typename ... Args>
    typename std::enable_if<std::is_base_of<BaseCharacter, typename std::decay<T>::type>::value>::type spawn(Args&&... args)
    {
        chars.push_back(std::shared_ptr<T>(new T(std::forward<Args>(args)...)));
    }

    void submit(Renderer& renderer) const;

private:
    std::vector<PCharacter> chars;
};

#define CHARACTER_MANAGER CharacterManager::get_singleton()

#endif
