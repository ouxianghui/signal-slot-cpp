#include "task_queue.h"
#include "task_queue_base.h"
#include "task_queue_stdlib.h"

namespace core {

    TaskQueue::TaskQueue(std::unique_ptr<TaskQueueBase, TaskQueueDeleter> taskQueue)
    : impl_(taskQueue.release()) {}

    TaskQueue::~TaskQueue() {
        // There might running task that tries to rescheduler itself to the TaskQueue
        // and not yet aware TaskQueue destructor is called.
        // Calling back to TaskQueue::PostTask need impl_ pointer still be valid, so
        // do not invalidate impl_ pointer until Delete returns.
        impl_->Delete();
    }

    bool TaskQueue::IsCurrent() const {
        return impl_->IsCurrent();
    }

    void TaskQueue::PostTask(std::unique_ptr<QueuedTask> task) {
        return impl_->PostTask(std::move(task));
    }

    void TaskQueue::PostDelayedTask(std::unique_ptr<QueuedTask> task, TimeDelta delay) {
        return impl_->PostDelayedTask(std::move(task), delay);
    }

    std::unique_ptr<TaskQueue> TaskQueue::Create(std::string_view name) {
        return std::make_unique<TaskQueue>(std::unique_ptr<TaskQueueBase, TaskQueueDeleter>(new TaskQueueStdlib(name)));
    }

}
