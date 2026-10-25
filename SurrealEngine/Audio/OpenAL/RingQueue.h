#pragma once

template<class T> class RingQueue
{
public:
	RingQueue()
	{
		Data = nullptr;
		Len = 0;
		Num = 0;
		Current = 0;
	}

	RingQueue(size_t n)
	{
		Data = static_cast<T*>(malloc(sizeof(T) * n));
		Len = n;
		Num = 0;
		Current = 0;
	}

	RingQueue(size_t n, const T& Value)
	{
		Data = static_cast<T*>(malloc(sizeof(T) * n));
		for (int i = 0; i < n; i++)
			Data[i] = Value;

		Len = n;
		Num = 0;
		Current = 0;
	}

	~RingQueue()
	{
		free(Data);
	}

	bool Empty()
	{
		return (Num == 0);
	}

	size_t Size()
	{
		return Num;
	}

	T& Front()
	{
		return Data[Current];
	}

	T& GetNextFree()
	{
		size_t Index = Current + Num;
		if (Index >= Len)
			Index -= Len;

		return Data[Index];
	}

	bool Push(const T& Val)
	{
		if (Num == Len)
			return false;

		GetNextFree() = Val;
		Num++;

		return true;
	}

	bool Push(T& Val)
	{
		if (Num == Len)
			return false;

		GetNextFree() = Val;
		Num++;

		return true;
	}

	T& Pop()
	{
		T& Out = Data[Current];
		if (Num > 0)
		{
			Current++;
			if (Current >= Len)
				Current = 0;

			Num--;
		}
		return Out;
	}

	bool Resize(size_t NewSize)
	{
		T* NewData = static_cast<T*>(realloc(Data, sizeof(T) * NewSize));
		if (NewData == NULL)
			return false;

		Data = NewData;
		Len = NewSize;
		return true;
	}

	void Clear()
	{
		Current = 0;
		Num = 0;
	}

private:
	T* Data = nullptr;
	size_t Len = 0;
	size_t Current = 0;
	size_t Num = 0;
};
