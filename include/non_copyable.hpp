#include <iostream>

// Non-copyable mixin
class NonCopyable
{
public:
    NonCopyable(const NonCopyable &) = delete;
    void operator=(const NonCopyable &) = delete;

protected:
    NonCopyable() {};
    ~NonCopyable() {};
};