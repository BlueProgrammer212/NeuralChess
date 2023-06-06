#include <iostream>
#include "non_copyable.hpp"

//TODO: Make this thread-safe.
class Singleton : public NonCopyable
{
public:
    Singleton(const Singleton&&) = delete;
    void operator=(const Singleton&&) = delete;

protected:
    Singleton() {};
    ~Singleton() {};
};