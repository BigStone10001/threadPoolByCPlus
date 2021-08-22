
#include"threadPool.h"


static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t notify=PTHREAD_COND_INITIALIZER;

std::vector<pthread_t> ThreadPool::threads;
std::vector<ThreadPoolTask> ThreadPool::queue;

int ThreadPool::thread_count = 0;
int ThreadPool::queue_size = 0;
int ThreadPool::head = 0;
int ThreadPool::tail = 0;
int ThreadPool::count = 0;
int ThreadPool::shutdown = 0;
int ThreadPool::started = 0;



int ThreadPool::threadpool_create(int _thread_num,int _queue_size){

    int err=0;
    thread_count=_thread_num;
    queue_size=_queue_size;
    threads.resize(thread_count);
    queue.resize(queue_size);

    for(int i=0;i<thread_count;i++){
        if(pthread_create(&threads[i],NULL,threadpool_thread,(void*)(0))!=0){
            return -1;
        }
        ++thread_count;
        ++started;

    }
    return err;
}


int ThreadPool::threadpool_add(std::shared_ptr<void> args,std::function<void(std::shared_ptr<void>)> fun){
    
    if(pthread_mutex_lock(&lock)!=0){
        return -1;
    }
    do{
        int next=0;
        next=(tail+1)%queue_size;
        if(count==queue_size){
            break;
        }
        if(shutdown){
            break;
        }
        queue[tail].fun=fun;
        queue[tail].args=args;
        tail=next;
        ++count;
        if(pthread_cond_signal(&notify)!=0){
            break;
        }
    }while(false);
    pthread_mutex_unlock(&lock);
    return 0;
}




void* ThreadPool::threadpool_thread(void* args){
    while(true){
        ThreadPoolTask task;
        pthread_mutex_lock(&lock);
        while(count==0&&(shutdown!=0)){
            pthread_cond_wait(&notify,&lock);
        }
        if(shutdown){
            break;
        }
        task.fun=queue[head].fun;
        task.args=queue[head].args;
        queue[head].fun=NULL;
        queue[head].args.reset();
        head=(head+1)%queue_size;
        --count;
        pthread_mutex_unlock(&lock);
        (task.fun)(task.args);
    }
    --started;
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
    return NULL;
}


int ThreadPool::threadpool_free(){
    if(started>0){
        return -1;
    }
    pthread_mutex_lock(&lock);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&notify);
    return 0;
}


int ThreadPool::threadpool_destroy(int _shutdown){
    pthread_mutex_lock(&lock);
    do{
        if(shutdown){
            break;
        }
        shutdown=_shutdown;
        pthread_cond_broadcast(&notify);
        pthread_mutex_unlock(&lock);

        for(int i=0;i<thread_count;i++){
            pthread_join(threads[i],NULL);
        }
    }while(false);
    threadpool_free();
    return 0;
}