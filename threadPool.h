#include<pthread.h>
#include<vector>
#include<functional>
#include<memory>


struct ThreadPoolTask{
    std::function<void(std::shared_ptr<void>)> fun;
    std::shared_ptr<void> args;
};


class ThreadPool{

private:
static pthread_mutex_t lock;
static pthread_cond_t notify;

static std::vector<pthread_t> threads;
static std::vector<ThreadPoolTask> queue;

static int tail;
static int count;
static int started;
static int shutdown;

static int thread_count;
static int head;
static int queue_size;

public:

static int threadpool_create(int _thread_count,int _queue_size);
static int threadpool_add(std::shared_ptr<void> args,std::function<void(std::shared_ptr<void>)> fun);
static int threadpool_destroy(int shutdown);
static int threadpool_free();
static void* threadpool_thread(void* args);

};