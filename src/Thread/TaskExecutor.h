//
// Created by xzl on 2018/9/13.
//

#ifndef ZLTOOLKIT_TASKEXECUTOR_H
#define ZLTOOLKIT_TASKEXECUTOR_H

#include <memory>
#include <functional>
using namespace std;

namespace ZL {
namespace Thread {

class TaskExecutor {
public:
    typedef function<void()> Task;
    typedef shared_ptr<TaskExecutor> Ptr;

    virtual ~TaskExecutor(){}
    //把任务打入线程池并异步执行
    virtual bool async(const Task &task, bool may_sync = true) = 0;
    virtual bool async_first(const Task &task, bool may_sync = true) {
        return async(task,may_sync);
    };
    virtual bool sync(const Task &task) = 0;
    virtual bool sync_first(const Task &task) {
        return sync(task);
    };
    //等待线程退出
    virtual void wait() = 0;
    virtual void shutdown() = 0;
    //任务个数
    virtual uint64_t size() = 0;
};

class TaskExecutorGetter {
public:
    typedef shared_ptr<TaskExecutorGetter> Ptr;
    virtual ~TaskExecutorGetter(){};
    virtual TaskExecutor::Ptr getExecutor() = 0;
    virtual void wait() = 0;
    virtual void shutdown() = 0;
};


template <typename T>
class TaskExecutorGetterImp : public TaskExecutorGetter{
public:
    TaskExecutorGetterImp(int threadnum = thread::hardware_concurrency()){
        _threadnum = threadnum;
        _threadPos = -1;
        for (int i = 0; i < _threadnum; i++) {
            _threads.emplace_back(std::make_shared<T>());
        }
    }

    ~TaskExecutorGetterImp(){
        shutdown();
        wait();
        _threads.clear();
    }

    TaskExecutor::Ptr getExecutor() override{
        if (++_threadPos >= _threadnum) {
            _threadPos = 0;
        }
        return _threads[_threadPos.load()];
    }

    void wait() override{
        for (auto &th : _threads){
            th->wait();
        }
    }
    void shutdown() override{
        for (auto &th : _threads){
            th->shutdown();
        }
    }

protected:
    int _threadnum;
    mutable atomic<int> _threadPos;
    vector <std::shared_ptr<T> > _threads;
};

}//Thread
}//ZL


#endif //ZLTOOLKIT_TASKEXECUTOR_H
