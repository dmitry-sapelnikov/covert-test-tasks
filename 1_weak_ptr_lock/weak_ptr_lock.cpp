#include <atomic>
#include <memory>
#include <iostream>

class ControlBlock
{
public:
	/// Increment the strong reference count if it's not zero
	bool increment_strong_ref_not_zero() noexcept
	{
		long int count = m_ref_counter.load(std::memory_order_relaxed);
		while (count != 0)
		{
			if (m_ref_counter.compare_exchange_weak(
				count,
				count + 1,
				std::memory_order_relaxed,
				std::memory_order_relaxed))
			{
				return true;
			}
		}
		return false;
	}

	/// Here it should be increment strong ref, increment and decrement weak ref, etc.

	/// Decrement the strong reference count
	int decrement_strong_ref() noexcept
	{
		return m_ref_counter.fetch_sub(1, std::memory_order_acq_rel) - 1;
	}

	/// The strong reference counter
	std::atomic<long int> m_ref_counter{1};
	
	// Here it should be weak ref counter to destruct
	// the control block only after dereferencing of both 
	// shared and weak pointers,
	// but we omit it for simplicity
};


// Forward declaration
template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr
{
public:
	/// Default constructor
	WeakPtr() noexcept = default;

	/// Constructor from SharedPtr
	explicit WeakPtr(const SharedPtr<T> &sp) noexcept;

	/// Locks the weak pointer to create a shared pointer
	/// If the managed object has already been deleted, returns an empty SharedPtr
	[[nodiscard]] SharedPtr<T> lock() const noexcept
	{
		SharedPtr<T> result;
		result.construct_from_weak(*this);
		return result;
	}

private:
	friend class SharedPtr<T>;

	ControlBlock *m_control_block{nullptr};

	// Here it should be pointer to T, but for simplicity we omit it
};

template <typename T>
class SharedPtr
{
public:
	/// Default constructor
	SharedPtr() noexcept = default;

	/// Constructor from raw pointer
	/// Doesn't actually manage pointer to the object for simplicity
	explicit SharedPtr(T *ptr) :
		m_control_block(ptr != nullptr ? new ControlBlock() : nullptr)
	{
	}

	/// Destructor
	~SharedPtr()
	{
		if (m_control_block != nullptr &&
			m_control_block->decrement_strong_ref() == 0)
		{
			std::cout << "SharedPtr destructor: deleting control block\n";
			delete m_control_block;
		}
	}

	// Stubs for copy, move constructors and assignment operators
	// It breaks the rule of 5 and should be implemented properly 
	// in a complete implementation
	// We keep only the move constructor for simplicity
	SharedPtr(SharedPtr &&) = default;

	SharedPtr(const SharedPtr &) = delete;
	SharedPtr &operator=(const SharedPtr &) = delete;
	SharedPtr &operator=(SharedPtr &&) = delete;

	/// bool operator
	explicit operator bool() const noexcept
	{
		return m_control_block != nullptr;
	}

private:
	friend class WeakPtr<T>;

	/// Construct SharedPtr from WeakPtr
	void construct_from_weak(const WeakPtr<T> &weak_ptr) noexcept
	{
		if (weak_ptr.m_control_block != nullptr &&
			weak_ptr.m_control_block->increment_strong_ref_not_zero())
		{
			m_control_block = weak_ptr.m_control_block;
			// Here it should also be assignment of the resource pointer,
			// but we omit it for simplicity
		}
	}

	ControlBlock *m_control_block{nullptr};
};

/// WeakPtr constructor from SharedPtr
template <typename T>
WeakPtr<T>::WeakPtr(const SharedPtr<T> &sp) noexcept : 
	m_control_block(sp.m_control_block)
{
}

/// Main function to do some basic tests
int main()
{
	// A fake transition of the allocated raw pointer to SharedPtr
	// unique_ptr is used here just to avoid memory leaks in this test
	std::unique_ptr<int> ptr(new int(123));
	SharedPtr<int> shared_ptr(ptr.get());

	WeakPtr<int> weak_ptr(shared_ptr);
	SharedPtr<int> shared_ptr_from_weak_lock = weak_ptr.lock();
	if (shared_ptr_from_weak_lock)
	{
		std::cout << "Successfully locked WeakPtr to SharedPtr." << std::endl;
	}
	else
	{
		std::cout << "Failed to lock WeakPtr; object no longer exists." << std::endl;
	}

	// Test on an empty WeakPtr
	WeakPtr<int> empty_weak_ptr;
	SharedPtr<int> empty_shared_ptr = empty_weak_ptr.lock();
	if (!empty_shared_ptr)
	{
		std::cout << "Correctly failed to lock an empty WeakPtr." << std::endl;
	}
	else
	{
		std::cout << "Error: locked an empty WeakPtr." << std::endl;
	}

	return 0;
}

