#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <functional>

template < typename T, typename Container = std::vector < T >, typename Compare = std::less < typename Container::value_type > >
class Threadsafe_Priority_Queue
{
public:

	Threadsafe_Priority_Queue() = default;

	Threadsafe_Priority_Queue(const Threadsafe_Priority_Queue& other)
	{
		std::lock_guard < std::mutex > lock(other.m_mutex);
		m_queue = other.m_queue;
	}

	Threadsafe_Priority_Queue& operator=(const Threadsafe_Priority_Queue& other)
	{
		std::scoped_lock lock(m_mutex, other.m_mutex);
		m_queue = other.m_queue;
	}

public:

	void push(T value)
	{
		std::lock_guard < std::mutex > lock(m_mutex);
		m_queue.push(value);
		m_condition_variable.notify_one();
	}

	void wait_and_pop(T& value)
	{
		std::unique_lock < std::mutex > lock(m_mutex);

		m_condition_variable.wait(lock, [this] {return !m_queue.empty(); });
		value = m_queue.top();
	}


	std::shared_ptr < T > wait_and_pop()
	{
		std::unique_lock < std::mutex > lock(m_mutex);

		m_condition_variable.wait(lock, [this] {return !m_queue.empty(); });
		auto result = std::make_shared < T >(m_queue.top());
		m_queue.pop();

		return result;
	}


	bool try_pop(T& value)
	{
		std::lock_guard < std::mutex > lock(m_mutex);

		if (m_queue.empty())
		{
			return false;
		}

		value = m_queue.top();
		m_queue.pop();

		return true;
	}


	std::shared_ptr < T > try_pop()
	{
		std::lock_guard < std::mutex > lock(m_mutex);

		if (m_queue.empty())
		{
			return std::shared_ptr < T >();
		}

		auto result = std::make_shared < T >(m_queue.top()));
		m_queue.pop();

		return result;
	}


	bool empty() const
	{
		std::lock_guard < std::mutex > lock(m_mutex);
		return m_queue.empty();
	}


	std::size_t size() const
	{
		std::lock_guard < std::mutex > lock(m_mutex);
		return m_queue.size();
	}


	const T& top()
	{
		std::unique_lock < std::mutex > lock(m_mutex);
		return m_queue.top();
	}

private:

	std::priority_queue < T, Container, Compare > m_queue;
	std::condition_variable m_condition_variable;

private:

	mutable std::mutex m_mutex;
};