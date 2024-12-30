//#include "AsyncTaskQueue.hpp"
//
//#include <condition_variable>
//#include <mutex>
//#include <string>
//#include <string_view>
//#include <thread>
//#include <queue>
//#include <functional>
//#include <future>
//
//#ifdef WIN32
//
//#include <Windows.h>
//
//#endif // !WIN32
//
//
//using namespace Coral;
//
//
//AsyncTaskQueue::AsyncTaskQueue()
//	:mThread([this] { run(); })
//{
//}
//
//
//AsyncTaskQueue::AsyncTaskQueue(std::string_view threadName)
//	: AsyncTaskQueue()
//{
//#ifdef WIN32
//	std::wstring name(threadName.begin(), threadName.end());
//	SetThreadDescription(mThread.native_handle(), name.c_str());
//#else
//	static_assert(false && "Not implemented")
//#endif // !WIN32		
//}
//
//
//AsyncTaskQueue::~AsyncTaskQueue()
//{
//	// Upon destruction, flush the remaining tasks from the queue
//	flush();
//
//	// Notify the queue that it should stop immediately
//	{
//		std::lock_guard lock(mQueueProtection);
//		mStop = true;
//	}
//	mConditionalVariable.notify_all();
//
//	// Wait for the thread to finish
//	mThread.join();
//}
//
//
//void
//AsyncTaskQueue::enqueue(AsyncTaskQueue::Task&& task)
//{
//	{
//		std::lock_guard lock(mQueueProtection);
//		mQueue.push(std::forward<Task&&>(task));
//	}
//
//	mConditionalVariable.notify_one();
//}
//
//
//void
//AsyncTaskQueue::flush()
//{
//	std::promise<void> p;
//	std::future<void> f = p.get_future();
//
//	// We flush the queue by inserting a dummy task to wait for execution to finish. This function blocks until the
//	// dummy task is executed.
//	enqueue([p = &p]
//		{
//			p->set_value();
//		});
//
//	f.get();
//}
//
//
//void
//AsyncTaskQueue::run()
//{
//	while (true)
//	{
//		std::unique_lock lock(mQueueProtection);
//
//		// Wait until tasks have been queued for execution or the queue is destructed
//		mConditionalVariable.wait(lock, [this]
//			{ return !mQueue.empty() || mStop; });
//
//		// The destructor notifies the runner thread to exit
//		if (mStop)
//		{
//			return;
//		}
//
//		auto task = std::move(mQueue.front());
//		mQueue.pop();
//		lock.unlock();
//
//		// Execute the task after unlocking the mutex to not block other
//		// threads from enqueuing new tasks
//		task();
//	}
//}
