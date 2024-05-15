#include "executor.hpp"
#include "BS_thread_pool.hpp"

executor::executor()
: m_pool(std::make_shared<BS::thread_pool>(1))
{

}

executor::~executor()
{

}

bool executor::is_current()
{
    return std::this_thread::get_id() == m_pool->get_thread_ids()[0];
}

void executor::sync(const std::function<void()>& task)
{
    m_pool->submit_task([&task](){ task(); }).wait();
}

void executor::async(const std::function<void()>& task)
{
    m_pool->detach_task([task](){ task(); });
}
