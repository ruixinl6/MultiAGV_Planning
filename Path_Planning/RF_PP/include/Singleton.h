#ifndef SINGLETON_H
#define SINGLETON_H

#include <mutex>


template<typename T>
class Singleton
{
public:
    static T &Instance()
    {
        static T inst;
        return inst;
    }

protected:
    Singleton() {}
    ~Singleton() {}

private:
    Singleton(const Singleton &) = delete;
    Singleton(const Singleton &&) = delete;
    Singleton &operator=(const Singleton &) = delete;
    Singleton &operator=(const Singleton &&) = delete;

};

#endif // SINGLETON_H
