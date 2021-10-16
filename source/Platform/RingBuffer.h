#pragma once
#include <mutex>
#include <optional>
#include <algorithm>

template <typename T> class RingBuffer
{
public:
	explicit RingBuffer(const uint32_t size)
		: _Capacity(size),
		  _Buffer(std::unique_ptr<T[]>(new T[size])),
		  _Head(0),
		  _Tail(0),
		  _IsFull(false)
	{
	}

	bool IsFull() const;
	bool IsEmpty() const;
	uint_fast16_t Capacity() const;
	uint_fast16_t Count() const;
	void Enqueue(T element);
	std::optional<T> Dequeue();
	bool Contains(const T& element) const;

	struct Iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;

		reference operator*() const { return *_Ptr; }
		pointer operator->() { return _Ptr; }

		explicit Iterator(const RingBuffer* Owner, pointer Pointer)
            : _Ptr(Pointer),
              _RingBuffer(Owner)
		{}

		Iterator& operator++()
		{
			++_Ptr;
			if(_Ptr == (&_RingBuffer->_Buffer[0] + _RingBuffer->_Capacity))
			{
				_Ptr = &_RingBuffer->_Buffer[0];
			}
			return *this;
		}

		Iterator& operator++(int)
		{
			Iterator temp = *this;
			++(*this);
			return temp;
		}

		friend bool operator==(const Iterator A, const Iterator& B)
		{
			return A._Ptr == B._Ptr;
		}

		friend bool operator!=(const Iterator A, const Iterator& B)
		{
			return A._Ptr != B._Ptr;
		}

	private:
		pointer _Ptr;
		const RingBuffer<T>* _RingBuffer;
	};

	// ReSharper disable once CppInconsistentNaming
	Iterator begin() const
	{
		return Iterator(this, &_Buffer[_Tail]);
	}

	// ReSharper disable once CppInconsistentNaming
	Iterator end() const
	{
		return Iterator(this, &_Buffer[_Head]);
	}

private:
	const uint32_t _Capacity;
	const std::unique_ptr<T[]> _Buffer;
	uint_fast16_t _Head;
	uint_fast16_t _Tail;

	bool _IsFull;
};


template <typename T> bool
RingBuffer<T>::IsFull() const
{
	return _IsFull;
}

template <typename T> bool
RingBuffer<T>::IsEmpty() const
{
	return (!_IsFull && _Head == _Tail);
}

template <typename T> uint_fast16_t
RingBuffer<T>::Capacity() const
{
	return _Capacity;
}

template <typename T> uint_fast16_t
RingBuffer<T>::Count() const
{
	if(_IsFull)
		return _Capacity;

	if(_Head >= _Tail)
	{
		return _Head - _Tail;
	}
	else // _Tail > _Head
	{
		return _Capacity + _Head - _Tail;
	}
}

template <typename T> void
RingBuffer<T>::Enqueue(T element)
{
	//@NOTE: We do no checking here and just assume that
	// we aren't overrunning the buffer and stomping data.

	//@TODO: Provide a checked version of this?
	_Buffer[_Head] = element;

	if(_IsFull)
	{
		// We're stomping data, advance the tail!
		_Tail = (_Tail + 1) % _Capacity;
	}

	_Head = (_Head + 1) % _Capacity;

	_IsFull = (_Head == _Tail);
}

template <typename T> std::optional<T>
RingBuffer<T>::Dequeue()
{
	std::optional<T> retVal;
	if(!IsEmpty())
	{
		retVal  = _Buffer[_Tail];
		_IsFull = false;
		//@TODO: Avoid this mod by using powers of 2?
		_Tail = (_Tail + 1) % _Capacity;
	}

	return retVal;
}

template <typename T> bool
RingBuffer<T>::Contains(const T& element) const
{
	auto start = &_Buffer[0];
	auto end   = &_Buffer[_Capacity];
	auto head  = &_Buffer[_Head];
	auto tail  = &_Buffer[_Tail];

	if(IsFull())
		return std::find(start, end, element) != end;

	if(_Head >= _Tail)
	{
		return std::find(tail, head, element) != head;
	}
	else // _Tail > _Head
	{
		return (std::find(tail, end, element) != end) ||
			(std::find(start, head, element) != head);
	}
}
