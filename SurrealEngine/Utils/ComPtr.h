#pragma once

template<typename T>
class ComPtr
{
public:
	ComPtr()
	{
		Ptr = nullptr;
	}
	
	ComPtr(const ComPtr& other)
	{
		Ptr = other.Ptr;
		if (Ptr)
			Ptr->AddRef();
	}
	
	ComPtr(ComPtr&& move) noexcept
	{
		Ptr = move.Ptr;
		move.Ptr = nullptr;
	}

	~ComPtr()
	{
		reset();
	}

	ComPtr& operator=(const ComPtr& other)
	{
		if (this != &other)
		{
			if (Ptr)
				Ptr->Release();
			Ptr = other.Ptr;
			if (Ptr)
				Ptr->AddRef();
		}
		return *this;
	}

	ComPtr& operator=(ComPtr&& move) noexcept
	{
		std::swap(Ptr, move.Ptr);
		return *this;
	}

	void reset()
	{
		if (Ptr)
			Ptr->Release();
		Ptr = nullptr;
	}

	T* get() { return Ptr; }

	static IID GetIID() { return __uuidof(T); }

	void** InitPtr()
	{
		return (void**)TypedInitPtr();
	}

	T** TypedInitPtr()
	{
		reset();
		return &Ptr;
	}

	operator T*() const { return Ptr; }
	T* operator ->() const { return Ptr; }

	T* Ptr;
};
