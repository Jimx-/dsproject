#ifndef SINGLETON_H
#define SINGLETON_H

/* Template class for creating single instance classes */

template <typename T>
class Singleton {
public:

    /* rely on megumi::Context to create the instance */
    Singleton()
    {
        singleton = static_cast<T*>(this);
    }

    Singleton(const Singleton<T>&) = delete;
    Singleton<T>& operator=(const Singleton<T>&) = delete;

    ~Singleton()
    {
        singleton = nullptr;
    }

    static T& get_singleton()
    {
        return *singleton;
    }

    static T* get_singleton_ptr()
    {
        return singleton;
    }

protected:
    static T* singleton;
};

#endif
