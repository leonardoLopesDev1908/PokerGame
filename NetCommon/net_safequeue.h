#pragma once
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace net
{
	template <typename T>
	class safe_queue
	{
	public:
		safe_queue()
		{}

		safe_queue(const std::deque<T>& other)
			: m_Queue(other)
		{}

		T front()
		{
			std::unique_lock<std::mutex> lck(m_mtx);
			m_cond.wait(lck, [this] { return !m_Queue.empty();});
			T item = m_Queue.front();
			return item;
		}
		
		T back()
		{
			std::unique_lock<std::mutex> lck(m_mtx);
			m_cond.wait(lck, [this] {return !m_Queue.empty();});
			T item = m_Queue.back();
			return item;
		}

		void push_front(T&& item)
		{
			std::lock_guard<std::mutex> lck(m_mtx);
			m_Queue.push_front(std::forward<T>(item));
			m_cond.notify_one();
		}

		T pop_front()
		{
			std::unique_lock<std::mutex> lck(m_mtx);
			m_cond.wait(lck, [this] {return !m_Queue.empty();});
			T item = m_Queue.front();
			m_Queue.pop_front();
			return item;
		}

		void pop_front(T& item)
		{
			std::unique_lock<std::mutex> lck(m_mtx);
			m_cond.wait(lck, [this] {return !m_Queue.empty();});
			item = m_Queue.front();
			m_Queue.pop_front();
		}

		void push_back(T&& item)
		{
			std::lock_guard<std::mutex> lck(m_mtx);
			m_Queue.push_back(std::forward<T>(item));
			m_cond.notify_one();
		}

		T pop_back()
		{
			std::unique_lock<std::mutex> lck(m_mtx);
			m_cond.wait(lck, [this] {return !m_Queue.empty();});
			T item = m_Queue.back();
			m_Queue.pop_back();
			return item;
		}

		void pop_back(T& item)
		{
			std::unique_lock<std::mutex> lck(m_mtx);
			m_cond.wait(lck, [this] {return !m_Queue.empty();});
			item = m_Queue.back();
			m_Queue.pop_back();
		}

		bool empty()
		{
			std::lock_guard<std::mutex> lck(m_mtx);
			return m_Queue.empty();
		}

		size_t size()
		{
			std::lock_guard<std::mutex> lck(m_mtx);
			return m_Queue.size();
		}

		void clear()
		{
			std::lock_guard<std::mutex> lck(m_mtx);
			m_Queue.clear();
		}

	private:
		std::deque<T> m_Queue;
		std::mutex m_mtx;
		std::condition_variable m_cond;
	};
}