#pragma once

#include <initializer_list>
#include <iterator>
#include <cstdlib>
#include <limits>
#include <stdexcept>

// Basically std::vector<T> except it has better performance in debug builds
template<typename T>
class Array
{
public:
	typedef T value_type;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T* iterator;
	typedef const T* const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	Array() noexcept = default;

	explicit Array(size_type count, const T& value)
	{
		resize(count, value);
	}

	Array(size_type count)
	{
		resize(count);
	}

	Array(const Array& other)
	{
		reserve(other._size);
		try
		{
			for (size_type i = 0, count = other._size; i < count; i++)
			{
				new ((char*)(_data + _size)) T(other._data[i]);
				_size++;
			}
		}
		catch (...)
		{
			clear();
			std::free(_data);
			throw;
		}
	}

	Array(Array&& other)
	{
		_data = other._data;
		_size = other._size;
		_capacity = other._capacity;
		other._data = nullptr;
		other._size = 0;
		other._capacity = 0;
	}

	template<typename InputIt>
	Array(InputIt first, InputIt last)
	{
		for (auto it = first; it != last; ++it)
		{
			emplace_back(*it);
		}
	}

	Array(std::initializer_list<T> init)
	{
		reserve(init.size());
		try
		{
			for (auto& v : init)
			{
				new ((char*)(_data + _size)) T(std::move(v));
				_size++;
			}
		}
		catch (...)
		{
			clear();
			std::free(_data);
			throw;
		}
	}

	~Array()
	{
		while (_size > 0)
		{
			_size--;
			_data[_size].~T();
		}
		std::free(_data);
	}

	Array& operator=(const Array& other)
	{
		if (&other != this)
		{
			clear();
			reserve(other.size());
			for (size_type i = 0, count = other._size; i < count; i++)
			{
				new ((char*)(_data + _size)) T(other._data[i]);
				_size++;
			}
		}
		return *this;
	}

	Array& operator=(Array&& other) noexcept
	{
		if (&other != this)
		{
			clear();
			shrink_to_fit();
			_data = other._data;
			_size = other._size;
			_capacity = other._capacity;
			other._data = nullptr;
			other._size = 0;
			other._capacity = 0;
		}
		return *this;
	}

	Array& operator=(std::initializer_list<value_type> ilist)
	{
		clear();
		reserve(ilist.size());
		for (auto& v : ilist)
		{
			new ((char*)(_data + _size)) T(std::move(v));
			_size++;
		}
		return *this;
	}

	void assign(size_type count, const T& value)
	{
		clear();
		resize(count, value);
	}

	template<typename InputIt>
	void assign(InputIt first, InputIt last)
	{
		clear();
		for (auto it = first; it != last; ++it)
		{
			emplace_back(*it);
		}
	}

	void assign(std::initializer_list<T> ilist)
	{
		clear();
		reserve(ilist.size());
		for (auto& v : ilist)
		{
			new ((char*)(_data + _size)) T(std::move(v));
			_size++;
		}
	}

	reference at(size_type pos)
	{
		return _data[pos];
	}

	const_reference at(size_type pos) const
	{
		return _data[pos];
	}

	reference operator[](size_type pos)
	{
		return _data[pos];
	}

	const_reference operator[](size_type pos) const
	{
		return _data[pos];
	}

	reference front()
	{
		return _data[0];
	}

	const_reference front() const
	{
		return _data[0];
	}

	reference back()
	{
		return _data[_size - 1];
	}

	const_reference back() const
	{
		return _data[_size - 1];
	}

	T* data()
	{
		return _data;
	}

	const T* data() const
	{
		return _data;
	}

	iterator begin()
	{
		return _data;
	}

	const_iterator begin() const
	{
		return _data;
	}

	const_iterator cbegin() const noexcept
	{
		return _data;
	}

	iterator end() noexcept
	{
		return _data + _size;
	}

	const_iterator end() const noexcept
	{
		return _data + _size;
	}

	const_iterator cend() const noexcept
	{
		return _data + _size;
	}

	reverse_iterator rbegin()
	{
		return reverse_iterator(_data + _size);
	}

	const_reverse_iterator rbegin() const
	{
		return const_reverse_iterator(_data + _size);
	}

	const_reverse_iterator crbegin() const noexcept
	{
		return const_reverse_iterator(_data + _size);
	}

	reverse_iterator rend()
	{
		return reverse_iterator(_data);
	}

	const_reverse_iterator rend() const
	{
		return const_reverse_iterator(_data);
	}

	const_reverse_iterator crend() const noexcept
	{
		return const_reverse_iterator(_data);
	}

	bool empty() const
	{
		return _size == 0;
	}

	size_type size() const
	{
		return _size;
	}

	size_type max_size() const
	{
		return std::numeric_limits<difference_type>::max();
	}

	void reserve(size_type new_cap)
	{
		if (new_cap > _capacity)
		{
			Array copy;
			copy._data = (T*)std::malloc(new_cap * sizeof(T));
			if (!copy._data) throw std::bad_alloc();
			copy._capacity = new_cap;

			for (size_type i = 0, count = _size; i < count; i++)
			{
				new((char*)(copy._data + i)) T(std::move(_data[i]));
				copy._size++;
			}

			swap(copy);
		}
	}

	size_type capacity() const
	{
		return _capacity;
	}

	void shrink_to_fit()
	{
		if (_size != _capacity)
		{
			Array copy;
			copy.reserve(_size);

			for (size_type i = 0, count = _size; i < count; i++)
			{
				new((char*)(copy._data + i)) T(std::move(_data[i]));
				copy._size++;
			}

			swap(copy);
		}
	}

	void clear()
	{
		while (_size > 0)
		{
			_data[_size - 1].~T();
			_size--;
		}
	}

	iterator insert(const_iterator pos, const T& value)
	{
		size_type p = (size_type)(difference_type)(pos - _data);
		if (_size == _capacity)
			reserve((_capacity + 1) << 1);

		if (p == _size)
		{
			new((char*)(_data + _size)) T(value);
			_size++;
		}
		else
		{
			for (size_type i = _size; i > p; i--)
			{
				new((char*)(_data + i)) T(std::move(_data[i - 1]));
			}
			_size++;

			_data[p] = value;
		}
		return _data + p;
	}

	iterator insert(const_iterator pos, T&& value)
	{
		size_type p = (size_type)(difference_type)(pos - _data);
		if (_size == _capacity)
			reserve((_capacity + 1) << 1);

		if (p == _size)
		{
			new((char*)(_data + _size)) T(std::move(value));
			_size++;
		}
		else
		{
			for (size_type i = _size; i > p; i--)
			{
				new((char*)(_data + i)) T(std::move(_data[i - 1]));
			}
			_size++;

			_data[p] = std::move(value);
		}
		return _data + p;
	}

	iterator insert(const_iterator pos, size_type count, const T& value)
	{
		size_type p = (size_type)(difference_type)(pos - _data);
		if (_size + count > _capacity)
			reserve((_capacity + count) << 1);

		if (p == _size)
		{
			for (size_t i = 0; i < count; i++)
			{
				new((char*)(_data + _size)) T(value);
				_size++;
			}
		}
		else
		{
			for (size_type i = _size; i > p; i--)
			{
				new((char*)(_data + (i + count - 1))) T(std::move(_data[i - 1]));
			}
			_size += count;

			for (size_type i = 0; i < count; i++)
			{
				_data[p + i] = value;
			}
		}
		return _data + p;
	}

	template<typename InputIt>
	iterator insert(const_iterator pos, InputIt first, InputIt last)
	{
		size_type p = (size_type)(difference_type)(pos - _data);
		size_type count = std::distance(first, last);
		if (_size + count > _capacity)
			reserve((_capacity + count) << 1);

		if (p == _size)
		{
			for (auto it = first; it != last; ++it)
			{
				new((char*)(_data + _size)) T(*it);
				_size++;
			}
		}
		else
		{
			for (size_type i = _size; i > p; i--)
			{
				new((char*)(_data + (i + count - 1))) T(std::move(_data[i - 1]));
			}
			_size += count;

			size_type i = p;
			for (auto it = first; it != last; ++it)
			{
				_data[i++] = *it;
			}
		}
		return _data + p;
	}

	iterator insert(const_iterator pos, std::initializer_list<T> ilist)
	{
		size_type p = (size_type)(difference_type)(pos - _data);
		size_type count = ilist.size();
		if (_size + count > _capacity)
			reserve((_capacity + count) << 1);

		if (p == _size)
		{
			for (size_t i = 0; i < count; i++)
			{
				new((char*)(_data + _size)) T(ilist[i]);
				_size++;
			}
		}
		else
		{
			for (size_type i = _size; i > p; i--)
			{
				new((char*)(_data + (i + count - 1))) T(std::move(_data[i - 1]));
			}
			_size += count;

			for (size_type i = 0; i < count; i++)
			{
				_data[p + i] = ilist[i];
			}
		}
		return _data + p;
	}

	template<typename... Args>
	iterator emplace(const_iterator pos, Args&&... args)
	{
		if (pos == end())
		{
			if (_size == _capacity)
				reserve((_capacity + 1) << 1);
			iterator it = _data + _size;
			new((char*)(_data + _size)) T(std::forward<Args>(args)...);
			_size++;
			return it;
		}
		else
		{
			T obj(std::forward<Args>(args)...);
			return insert(pos, std::move(obj));
		}
	}

	iterator erase(const_iterator pos)
	{
		size_type p = (size_type)(difference_type)(pos - _data);
		for (size_type i = p + 1, s = _size; i < s; i++)
		{
			_data[i - 1] = std::move(_data[i]);
		}
		_data[_size - 1].~T();
		_size--;
		return _data + p;
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		size_type p0 = (size_type)(difference_type)(first - _data);
		size_type p1 = (size_type)(difference_type)(last - _data);
		size_type count = p1 - p0;
		for (size_type i = p1, s = _size; i < s; i++)
		{
			_data[i - count] = std::move(_data[i]);
		}
		for (size_type i = _size - count; i < _size; i++)
		{
			_data[i].~T();
		}
		_size -= count;
		return _data + p0;
	}

	void push_back(const T& value)
	{
		if (_size == _capacity)
			reserve((_capacity + 1) << 1);
		new((char*)(_data + _size)) T(value);
		_size++;
	}

	void push_back(T&& value)
	{
		if (_size == _capacity)
			reserve((_capacity + 1) << 1);
		new((char*)(_data + _size)) T(std::move(value));
		_size++;
	}

	template<typename... Args>
	iterator emplace_back(Args&&... args)
	{
		if (_size == _capacity)
			reserve((_capacity + 1) << 1);
		iterator it = _data + _size;
		new((char*)(_data + _size)) T(std::forward<Args>(args)...);
		_size++;
		return it;
	}

	void pop_back()
	{
		_data[_size - 1].~T();
		_size--;
	}

	void resize(size_type count)
	{
		if (count > _size)
		{
			reserve(count);
			while (_size < count)
			{
				new((char*)(_data + _size)) T();
				_size++;
			}
		}
		else if (count < _size)
		{
			while (_size > count)
			{
				_data[_size - 1].~T();
				_size--;
			}
		}
	}

	void resize(size_type count, const value_type& value)
	{
		if (count > _size)
		{
			reserve(count);
			while (_size < count)
			{
				new((char*)(_data + _size)) T(value);
				_size++;
			}
		}
		else if (count < _size)
		{
			while (_size > count)
			{
				_data[_size - 1].~T();
				_size--;
			}
		}
	}

	void swap(Array& other) noexcept
	{
		std::swap(_data, other._data);
		std::swap(_size, other._size);
		std::swap(_capacity, other._capacity);
	}

	bool operator==(const Array<T>& rhs) const
	{
		if (this->size() != rhs.size())
			return false;

		auto count = this->size();
		for (size_type i = 0; i < count; i++)
		{
			if (((*this)[i] == rhs[i]) == false)
				return false;
		}
		return true;
	}

	bool operator!=(const Array<T>& rhs) const
	{
		if (this->size() != rhs.size())
			return true;
		size_type count = this->size();
		for (size_type i = 0; i < count; i++)
		{
			if ((*this)[i] != rhs[i])
				return true;
		}
		return false;
	}

private:
	T* _data = nullptr;
	size_type _size = size_type(0);
	size_type _capacity = size_type(0);
};

namespace std
{
	template<typename T>
	void swap(const Array<T>& lhs, const Array<T>& rhs)
	{
		lhs.swap(rhs);
	}
}
