#pragma once

#include "Math/vec.h"

class UActor;

// TODO: Consolidate these into one class
class TraceHit
{
public:
	TraceHit() = default;
	TraceHit(float fraction, vec3 normal) : Fraction(fraction), Normal(normal) { }

	float Fraction = 1.0;
	vec3 Normal = vec3(0.0);
};

class BspNode;

class SweepHit
{
public:
	SweepHit() = default;
	SweepHit(float fraction, vec3 normal, UActor* actor) : Fraction(fraction), Normal(normal), Actor(actor) { }
	SweepHit(TraceHit& hit) : Fraction(hit.Fraction), Normal(hit.Normal), Actor(nullptr) { }

	float Fraction = 1.0;
	vec3 Normal = vec3(0.0);
	UActor* Actor = nullptr;
	BspNode* node = nullptr;
};

class TraceHitList
{
public:
	TraceHitList()
	{
		items = (TraceHit*)buffer;
		count = 0;
		capacity = buffercapacity;
	}

	TraceHitList(const TraceHitList& other)
	{
		items = (TraceHit*)buffer;
		count = 0;
		capacity = buffercapacity;

		push_back(other);
	}

	~TraceHitList()
	{
		clear();

		if (capacity > buffercapacity)
			delete[] (uint64_t*)items;
	}

	TraceHitList& operator=(const TraceHitList& other)
	{
		if (this != &other)
		{
			clear();
			push_back(other);
		}
		return *this;
	}

	typedef TraceHit* iterator;
	typedef const TraceHit* const_iterator;

	iterator begin() { return items; }
	iterator end() { return items + count; }
	const_iterator begin() const { return items; }
	const_iterator end() const { return items + count; }

	bool empty() const { return count == 0; }

	TraceHit& front() { return items[0]; }
	const TraceHit& front() const { return items[0]; }

	TraceHit& back() { return items[count - 1]; }
	const TraceHit& back() const { return items[count - 1]; }

	void clear()
	{
		for (size_t i = 0, c = count; i < c; i++)
			items[i].~TraceHit();
		count = 0;
	}

	void push_back(const TraceHit& hit)
	{
		if (count != capacity)
		{
			new((uint8_t*)&items[count]) TraceHit(hit);
			count++;
		}
		else
		{
			reserve(capacity << 1);

			new((uint8_t*)&items[count]) TraceHit(hit);
			count++;
		}
	}

	void push_back(const TraceHitList& list)
	{
		for (size_t i = 0, count = list.count; i < count; i++)
		{
			push_back(list.items[i]);
		}
	}

private:
	void reserve(size_t newcapacity)
	{
		if (count < newcapacity)
		{
			size_t allocsize = (sizeof(TraceHit) * newcapacity + 7) / 8;
			TraceHit* newitems = (TraceHit*)new uint64_t[allocsize];

			for (size_t i = 0, c = count; i < c; i++)
			{
				new ((uint8_t*)&newitems[i]) TraceHit(std::move(items[i]));
				items[i].~TraceHit();
			}

			TraceHit* olditems = items;
			items = newitems;

			if (capacity > buffercapacity)
				delete[](uint64_t*)olditems;

			capacity = newcapacity;
		}
	}

	static const int buffercapacity = 16;
	uint64_t buffer[(sizeof(TraceHit) * buffercapacity + 7) / 8];

	TraceHit* items;
	size_t count;
	size_t capacity;
};

class SweepHitList
{
public:
	SweepHitList()
	{
		items = (SweepHit*)buffer;
		count = 0;
		capacity = buffercapacity;
	}

	SweepHitList(const SweepHitList& other)
	{
		items = (SweepHit*)buffer;
		count = 0;
		capacity = buffercapacity;

		push_back(other);
	}

	~SweepHitList()
	{
		clear();

		if (capacity > buffercapacity)
			delete[](uint64_t*)items;
	}

	SweepHitList& operator=(const SweepHitList& other)
	{
		if (this != &other)
		{
			clear();
			push_back(other);
		}
		return *this;
	}

	typedef SweepHit* iterator;
	typedef const SweepHit* const_iterator;

	iterator begin() { return items; }
	iterator end() { return items + count; }
	const_iterator begin() const { return items; }
	const_iterator end() const { return items + count; }

	bool empty() const { return count == 0; }

	SweepHit& front() { return items[0]; }
	const SweepHit& front() const { return items[0]; }

	SweepHit& back() { return items[count - 1]; }
	const SweepHit& back() const { return items[count - 1]; }

	void clear()
	{
		for (size_t i = 0, c = count; i < c; i++)
			items[i].~SweepHit();
		count = 0;
	}

	void push_back(const SweepHit& hit)
	{
		if (count != capacity)
		{
			new((uint8_t*)&items[count]) SweepHit(hit);
			count++;
		}
		else
		{
			reserve(capacity << 1);

			new((uint8_t*)&items[count]) SweepHit(hit);
			count++;
		}
	}

	void push_back(const SweepHitList& list)
	{
		for (size_t i = 0, count = list.count; i < count; i++)
		{
			push_back(list.items[i]);
		}
	}

private:
	void reserve(size_t newcapacity)
	{
		if (count < newcapacity)
		{
			size_t allocsize = (sizeof(SweepHit) * newcapacity + 7) / 8;
			SweepHit* newitems = (SweepHit*)new uint64_t[allocsize];

			for (size_t i = 0, c = count; i < c; i++)
			{
				new ((uint8_t*)&newitems[i]) SweepHit(std::move(items[i]));
				items[i].~SweepHit();
			}

			SweepHit* olditems = items;
			items = newitems;

			if (capacity > buffercapacity)
				delete[](uint64_t*)olditems;

			capacity = newcapacity;
		}
	}

	static const int buffercapacity = 16;
	uint64_t buffer[(sizeof(SweepHit) * buffercapacity + 7) / 8];

	SweepHit* items;
	size_t count;
	size_t capacity;
};
