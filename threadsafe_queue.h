#ifndef THREADSAFE_QUEUE_H
#define THREADSAFE_QUEUE_H


#include <queue>
#include <mutex>
#include <condition_variable>
template<typename T>
class threadsafe_queue
{
private:
    std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
//    void push(T new_value)
//    {
//        std::lock_guard<std::mutex> lk(mut);
//        data_queue.push(new_value);
//        data_cond.notify_one();
//    }
    // 新增的移动版本的 push 函数
    void push(T&& new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.emplace(std::move(new_value));
        data_cond.notify_one();
    }
    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{ return !data_queue.empty(); });
        value = std::move(data_queue.front());  // 使用 std::move 确保资源转移
        data_queue.pop();
    }
};


#endif // THREADSAFE_QUEUE_H
