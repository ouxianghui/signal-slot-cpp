#pragma once

#include <string.h>
#include <algorithm>
#include <map>
#include <memory>
#include <queue>
#include <utility>
#include <thread>
#include <mutex>
#include <string_view>
#include "queued_task.h"
#include "event.h"
#include "task_queue_base.h"

namespace core {
    class TaskQueueStdlib final : public TaskQueueBase {
    public:
        TaskQueueStdlib(std::string_view queue_name);
        ~TaskQueueStdlib() override;

        void Delete() override;
        void PostTask(std::unique_ptr<QueuedTask> task) override;
        void PostDelayedTask(std::unique_ptr<QueuedTask> task, TimeDelta delay) override;
        void PostDelayedHighPrecisionTask(std::unique_ptr<QueuedTask> task, TimeDelta delay) override;
        const std::string& Name() const override;

    private:
        using OrderId = uint64_t;

        struct DelayedEntryTimeout {
            // TODO(bugs.webrtc.org/13756): Migrate to Timestamp.
            int64_t next_fire_at_us{};
            OrderId order{};

            bool operator<(const DelayedEntryTimeout& o) const {
                return std::tie(next_fire_at_us, order) <
                       std::tie(o.next_fire_at_us, o.order);
            }
        };

        struct NextTask {
            bool final_task{false};
            std::unique_ptr<QueuedTask> run_task;
            TimeDelta sleep_time = Event::kForever;
        };

        NextTask GetNextTask();

        void ProcessTasks();

        void NotifyWake();

        // Signaled whenever a new task is pending.
        Event flag_notify_;

        std::mutex pending_lock_;

        // Indicates if the worker thread needs to shutdown now.
        bool thread_should_quit_ = false;

        // Holds the next order to use for the next task to be
        // put into one of the pending queues.
        OrderId thread_posting_order_ = 0;

        // The list of all pending tasks that need to be processed in the
        // FIFO queue ordering on the worker thread.
        std::queue<std::pair<OrderId, std::unique_ptr<QueuedTask>>> pending_queue_;

        // The list of all pending tasks that need to be processed at a future
        // time based upon a delay. On the off change the delayed task should
        // happen at exactly the same time interval as another task then the
        // task is processed based on FIFO ordering. std::priority_queue was
        // considered but rejected due to its inability to extract the
        // move-only value out of the queue without the presence of a hack.
        std::map<DelayedEntryTimeout, std::unique_ptr<QueuedTask>> delayed_queue_;

        // Contains the active worker thread assigned to processing
        // tasks (including delayed tasks).
        // Placing this last ensures the thread doesn't touch uninitialized attributes
        // throughout it's lifetime.
        std::thread thread_;

        std::string name_;

        Event started_;
    };
}

