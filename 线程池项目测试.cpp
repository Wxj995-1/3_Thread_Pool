#include "threadpool.h"
#include <iostream>
#include <chrono>
#include <thread>
using namespace std;

using ULong = unsigned long long;
class Mytask : public Task
{
public:

	Mytask(ULong begin, ULong end)
		:begin_(begin), end_(end)
	{

	}
	//问题一 怎么设计run函数的返回值 可以表示任意的类型
	Any run()   //run方法最终就在线程池分配的线程中去做执行了
	{
		std::cout << "tid: " << std::this_thread::get_id() << "begin! " << std::endl;

		std::this_thread::sleep_for(std::chrono::seconds(3));
		ULong sum = 0;
		for (ULong i = begin_; i < end_; i++)
		{
			sum += i;
		}
		std::cout << "tid: " << std::this_thread::get_id() << "end! " << std::endl;

		return sum;
	}

private:
	ULong begin_;
	ULong end_;
};

/*
有些场景 希望获取线程执行任务的返回值的
*/


int main()
{
	ThreadPool pool;
	pool.start(4);
	Result res1 = pool.submitTask(std::make_shared<Mytask>(1, 100000000));
	ULong sum1 = res1.get().cast_<ULong>();   //get返回了一个Any类型 怎么转成具体的类型
	cout << sum1 << endl;
	cout << "main over!" << endl;
#if 0
	// 问题：ThreadPool对象析构以后 怎么样把线程池相关的线程资源全部回收
	{
		ThreadPool pool;
		pool.setMode(PoolMode::MODE_CACHED);
		//启动线程池
		pool.start(4);

		//如何设计这里的Result机制
		Result res1 = pool.submitTask(std::make_shared<Mytask>(1, 100000000));
		Result res2 = pool.submitTask(std::make_shared<Mytask>(100000001, 200000000));
		Result res3 = pool.submitTask(std::make_shared<Mytask>(200000001, 300000000));
		pool.submitTask(std::make_shared<Mytask>(200000001, 300000000));

		pool.submitTask(std::make_shared<Mytask>(200000001, 300000000));
		pool.submitTask(std::make_shared<Mytask>(200000001, 300000000));
		//随着task被执行完 task对象没了 依赖于task的Result对象也没了
		ULong sum1 = res1.get().cast_<ULong>();   //get返回了一个Any类型 怎么转成具体的类型
		ULong sum2 = res2.get().cast_<ULong>();   //get返回了一个Any类型 怎么转成具体的类型
		ULong sum3 = res3.get().cast_<ULong>();   //get返回了一个Any类型 怎么转成具体的类型

		cout << (sum1 + sum2 + sum3) << endl;

	
	}


	//pool.submitTask(std::make_shared<Mytask>());
	//pool.submitTask(std::make_shared<Mytask>());
	//pool.submitTask(std::make_shared<Mytask>());
	//pool.submitTask(std::make_shared<Mytask>());
	//pool.submitTask(std::make_shared<Mytask>());
	//pool.submitTask(std::make_shared<Mytask>());
	//pool.submitTask(std::make_shared<Mytask>());

	getchar();
#endif
}