#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <map>
//ANY类型 可以接受任意数据的类型
class Any
{
public:
	Any() = default;
	~Any() = default;
	Any(const Any&) = delete;
	Any& operator=(const Any&) = delete;
	Any(Any&&) = default;
	Any& operator=(Any&&) = default;

	//这个构造函数可以让Any类型接收任意其他的数据
	template <typename T>
	Any(T data):base_(std::make_unique<Derive<T>>(data))
	{}

	//这个方法能把Any对象里面存储的data数据提取出来
	template <typename T>
	T cast_()
	{
		//怎么用base_找到它所指的Derive对象 从他里面取出data成员变量
		// 基类指针 =》派生类指针
		Derive<T>* pd = dynamic_cast<Derive<T> *>(base_.get());
		if (pd == nullptr)
		{
			throw "type is incompatiable";
		} 
		return pd->data_;
	}

private:
	//基类类型
	class Base
	{
	public:
		virtual ~Base() = default;
	private:
	};

	//派生类类型
	template <typename T>
	class Derive : public Base
	{
	public:
		Derive(T data) :data_(data)
		{}
		T data_;   //保存了任意的其他类型
	};
private: 
	// 定义一个基类指针
	std::unique_ptr<Base> base_;  //基类指针
};

//实现一个信号量类
class Semaphore
{
public:
	Semaphore(int limit = 0) :resLimit_(limit)
	{
	}

	~Semaphore() = default;
	//释放信号量资源
	void post()
	{
		std::unique_lock<std::mutex> lock(mtx_);
		resLimit_++;
		cond_.notify_all();
	}

	//获取信号量资源
	void wait()
	{
		std::unique_lock<std::mutex> lock(mtx_);
		//等待信号量有资源 没资源 阻塞当前线程
		cond_.wait(lock, [&]()->bool {return resLimit_ > 0;});
		resLimit_--;
	}
private:
	int resLimit_;
	std::mutex mtx_;
	std::condition_variable cond_;
};

// 实现接收提交到线程池的task任务执行完成后的返回值类型Result
// 前置声明
class Task;
class Result
{
public:
	Result(std::shared_ptr<Task> task, bool isValid = true);
	~Result() = default;

	// 问题一：setVal方法 获取任务执行完的返回值
	void setVal(Any any);

	//问题二：get方法 用户调用这个方法获取task的返回值
	Any get();
private:
	Any any_;  //存储任务的返回值
	Semaphore sem_;   //线程通信信号量
	std::shared_ptr<Task> task_;     //指向对应获取返回值的任务对象
	std::atomic_bool isValid_;      //返回值是否有效
};




// 任务抽象基类
class Task
{
public:
	Task();
	~Task() = default;
	
	
	// 用户可以自定义任务类型 从Task继承 重写run方法 实现自定义任务处理
	virtual Any run() = 0;

	void setResult(Result* res);


	void exec();

private:
	Result* result_;
};



//线程池支持的模式
enum class PoolMode
{
	MODE_FIXED,   //固定线程数量
	MODE_CACHED   //线程数量可动态增长
};


//线程类型
class Thread
{
public:
	using ThreadFunc = std::function<void(int)>;
	
	//构造函数
	Thread(ThreadFunc func);
	
	//析构函数
	~Thread();

	//启动线程
	void start();

	//获取线程id
	int getId() const;
private:
	ThreadFunc func_;

	static int generatrId; 
	int threadId_;    //保存线程ID
};

//线程池类型
class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(ThreadPool&) = delete;

	// 设置线程池线Cached程数量阈值
	void setThreadSizeThreshHold(int threshhold);




	//给线程池提交任务
	Result submitTask(std::shared_ptr<Task> sp);

	//设置task任务队列上限的阈值
	void setTaskQueMaxThreshHold(int threshhold);

	//设置线程池的工作模式
	void setMode(PoolMode mode);

	//开启线程池
	void start(int initThreadSize = std::thread::hardware_concurrency());

private:
	//定义线程函数
	void threadFunc(int threadid);

	//检查pool的运行状态
	bool checkRunningState() const;

private:
	//std::vector<std::unique_ptr<Thread>> threads_;   //线程列表
	std::unordered_map<int, std::unique_ptr<Thread>> threads_;   //线程列表
	size_t initThreadSize_;     // 初始线程数量
	int threadSizeThreshHold_;  //线程数量上线阈值
	std::atomic_int curThreadSize_; //记录当前线程池里面线程的总数量

	std::queue<std::shared_ptr<Task>> taskQue_;   // 任务队列
	std::atomic_int taskSize_;			// 任务数量
	int taskQueMaxThreshHold_;			//任务队列数上线的阈值

	std::mutex taskQueMtx_;   //保证任务队列的线程安全
	std::condition_variable notFull_;   //表示任务队列不满
	std::condition_variable notEmpty_;	// 表示任务队列不空
	std::condition_variable exitCond_;   //等待线程资源全部回收

	PoolMode poolMode_;   //记录当前线程池的工作模式

	std::atomic_bool isPoolRunning_;     //线程池是否在运行

	std::atomic_int idleThreadSize_;     //空余线程的数量
};





#endif