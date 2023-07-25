#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <future>
#include <assert.h>

class ThreadPool{
public:    
    ThreadPool(int threadNum = 4) : stop_(false){
        for(int i = 0; i < threadNum; i++){
            workers_.emplace_back([this, i]{
                for(;;){
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> locker(mtx_);
                        cv_.wait(locker, [this]{ return stop_ || !tasks_.empty(); });
                        if(stop_ && tasks_.empty())
                            return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    printf("%d号线程开始工作...\n", i);
                    task();
                    printf("%d号线程工作完成！\n", i);
                }
            });
        }
    }
    ~ThreadPool(){
        {
            std::unique_lock<std::mutex> locker(mtx_);
            stop_ = true;
        }
        cv_.notify_all();
        for(auto &worker : workers_)
            worker.join();
    }

    template<class F, class... Args>
    auto AddTask(F &&f, Args&&... args) -> std::future<decltype(f(args...))>{
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        {
            std::unique_lock<std::mutex> locker(mtx_);
            assert(!stop_);
            tasks_.push([task_ptr]{ (*task_ptr)(); });
        }
        cv_.notify_one();
        return task_ptr->get_future();
    }

private:
    std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<std::function<void()> > tasks_;
    std::vector<std::thread> workers_;
    bool stop_;

};