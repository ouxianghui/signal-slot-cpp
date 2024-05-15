#pragma once

#include <functional>

namespace sigslot {
    class i_executor
    {
    public:
        virtual ~i_executor() = default;

        virtual bool is_current() = 0;

        virtual void sync(const std::function<void()>& task) = 0;

        virtual void async(const std::function<void()>& task) = 0;
    };
}
