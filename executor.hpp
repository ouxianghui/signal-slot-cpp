#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <memory>
#include "i_executor.hpp"
#include "signal.hpp"

namespace BS {
    class thread_pool;
}

class executor : public sigslot::i_executor
{
public:
    executor();

    ~executor();

    bool is_current() override;

    void sync(const std::function<void()>& task) override;

    void async(const std::function<void()>& task) override;

private:
    std::shared_ptr<BS::thread_pool> m_pool;
};

#endif // EXECUTOR_H
