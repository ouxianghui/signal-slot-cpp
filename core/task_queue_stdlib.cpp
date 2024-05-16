#include "task_queue_stdlib.h"
#include <assert.h>
#include "time_utils.h"
#include "divide_round.h"

namespace core {

    TaskQueueStdlib::TaskQueueStdlib(std::string_view queue_name)
    : flag_notify_(/*manual_reset=*/false, /*initially_signaled=*/false)
    , name_(queue_name) {
        //Event started;
        thread_ = std::thread([this]{
            CurrentTaskQueueSetter setCurrent(this);
            this->started_.Set();
            this->ProcessTasks();
        });

        started_.Wait(Event::kForever);
    }

    void TaskQueueStdlib::Delete() {
        assert(!IsCurrent());

        {
            std::unique_lock<std::mutex> lock(pending_lock_);
            thread_should_quit_ = true;
        }

        NotifyWake();

        delete this;
    }

    void TaskQueueStdlib::PostTask(std::unique_ptr<QueuedTask> task) {
        {
            std::unique_lock<std::mutex> lock(pending_lock_);
            pending_queue_.push(std::make_pair(++thread_posting_order_, std::move(task)));
        }

        NotifyWake();
    }

    void TaskQueueStdlib::PostDelayedTask(std::unique_ptr<QueuedTask> task, TimeDelta delay) {
        DelayedEntryTimeout delayed_entry;
        delayed_entry.next_fire_at_us = TimeMicros() + delay.us();

        {
            std::unique_lock<std::mutex> lock(pending_lock_);
            delayed_entry.order = ++thread_posting_order_;
            delayed_queue_[delayed_entry] = std::move(task);
        }

        NotifyWake();
    }

    void TaskQueueStdlib::PostDelayedHighPrecisionTask(std::unique_ptr<QueuedTask> task, TimeDelta delay) {
        PostDelayedTask(std::move(task), delay);
    }

    const std::string& TaskQueueStdlib::Name() const {
        return name_;
    }

    TaskQueueStdlib::NextTask TaskQueueStdlib::GetNextTask() {
        NextTask result;

        const int64_t tick_us = TimeMicros();

        std::unique_lock<std::mutex> lock(pending_lock_);

        if (thread_should_quit_) {
            result.final_task = true;
            return result;
        }

        if (delayed_queue_.size() > 0) {
            auto delayed_entry = delayed_queue_.begin();
            const auto& delay_info = delayed_entry->first;
            auto& delay_run = delayed_entry->second;
            if (tick_us >= delay_info.next_fire_at_us) {
                if (pending_queue_.size() > 0) {
                    auto& entry = pending_queue_.front();
                    auto& entry_order = entry.first;
                    auto& entry_run = entry.second;
                    if (entry_order < delay_info.order) {
                        result.run_task = std::move(entry_run);
                        pending_queue_.pop();
                        return result;
                    }
                }

                result.run_task = std::move(delay_run);
                delayed_queue_.erase(delayed_entry);
                return result;
            }

            result.sleep_time = TimeDelta::Millis(DivideRoundUp(delay_info.next_fire_at_us - tick_us, 1'000));
        }

        if (pending_queue_.size() > 0) {
            auto& entry = pending_queue_.front();
            result.run_task = std::move(entry.second);
            pending_queue_.pop();
        }

        return result;
    }

    void TaskQueueStdlib::ProcessTasks() {
        while (true) {
            auto task = GetNextTask();

            if (task.final_task)
                break;

            if (task.run_task) {
                // process entry immediately then try again
                QueuedTask* release_ptr = task.run_task.release();
                if (release_ptr->run()) {
                    delete release_ptr;
                }
                // Attempt to run more tasks before going to sleep.
                continue;
            }

            flag_notify_.Wait(task.sleep_time);
        }
    }

    void TaskQueueStdlib::NotifyWake() {
        // The queue holds pending tasks to complete. Either tasks are to be
        // executed immediately or tasks are to be run at some future delayed time.
        // For immediate tasks the task queue's thread is busy running the task and
        // the thread will not be waiting on the flag_notify_ event. If no immediate
        // tasks are available but a delayed task is pending then the thread will be
        // waiting on flag_notify_ with a delayed time-out of the nearest timed task
        // to run. If no immediate or pending tasks are available, the thread will
        // wait on flag_notify_ until signaled that a task has been added (or the
        // thread to be told to shutdown).

               // In all cases, when a new immediate task, delayed task, or request to
               // shutdown the thread is added the flag_notify_ is signaled after. If the
               // thread was waiting then the thread will wake up immediately and re-assess
               // what task needs to be run next (i.e. run a task now, wait for the nearest
               // timed delayed task, or shutdown the thread). If the thread was not waiting
               // then the thread will remained signaled to wake up the next time any
               // attempt to wait on the flag_notify_ event occurs.

               // Any immediate or delayed pending task (or request to shutdown the thread)
               // must always be added to the queue prior to signaling flag_notify_ to wake
               // up the possibly sleeping thread. This prevents a race condition where the
               // thread is notified to wake up but the task queue's thread finds nothing to
               // do so it waits once again to be signaled where such a signal may never
               // happen.
        flag_notify_.Set();
    }
}
