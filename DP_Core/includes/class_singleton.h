#pragma once

#include <type_traits>

template <typename T, typename D = T>
class Singleton
{
    friend D;
    static_assert(std::is_base_of<T, D>::value, "T must be a base type for D");

public:
    static T& instance();

private:
    Singleton() = default;
    virtual ~Singleton() = default;
    Singleton( const Singleton& ) = delete;
    Singleton& operator=( const Singleton& ) = delete;
};

template <typename T, typename D>
T& Singleton<T, D>::instance()
{
    static D inst;
    return inst;
}

