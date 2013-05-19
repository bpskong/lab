#ifndef BPSLAB_REF_H
#define BPSLAB_REF_H

#include <stdint.h>
#include <stddef.h>

class Refable
{
public:
	enum
	{
		firstIncStrong = 0x0001
	};
	enum
	{
		lifetimeStrong = 0x0000,
		lifetimeWeak = 0x0001,
		lifetimeMask = 0x0001
	};
	class Ref
	{
	public:
		void incWeak(const void* id);
		void decWeak(const void* id);
		bool attemptIncStrong(const void* id);
	};
public:
	Refable();
	virtual ~Refable();
	void incStrong(const void* id) const;
	void decStrong(const void* id) const;
	void extendObjectLifetime(int32_t mode);
public:
	virtual void onFirstRef();
	virtual void onLastStrongRef(const void* id);
	virtual bool onIncStrongAttempted(uint32_t flags, const void* id);
	virtual void onLastWeakRef(const void* id);
protected:
	Ref* _ref;
	Refable(const Refable& o);
	Refable& operator=(const Refable& o);
	template<typename Y> friend class StrongPtr;
	template<typename Y> friend class WeakPtr;
};

template <typename T> class WeakPtr;
template <typename T>
class StrongPtr
{
public:
	inline StrongPtr()
		: _ptr(0)
	{

	}
	StrongPtr(T* other)
		: _ptr(other)
	{
		if (other)
			other->incStrong(this);
	}
	StrongPtr(const StrongPtr<T>& other)
		: _ptr(other._ptr)
	{
		if (_ptr)
			_ptr->incStrong(this);
	}
	template<typename U>
	StrongPtr(U* other)
		: _ptr(other)
	{
		if (other)
			((T*)other)->incStrong(this);
	}
	template<typename U>
	StrongPtr(const StrongPtr<U>& other)
		: _ptr(other._ptr)
	{
		if (_ptr)
			_ptr->incStrong(this);
	}
	~StrongPtr()
	{
		if (_ptr)
			_ptr->decStrong(this);
	}
	void clear()
	{
		if (_ptr)
		{
			_ptr->decStrong(this);
			_ptr = 0;
		}
	}

	//
	// operator=
	//
	StrongPtr<T>& operator=(const StrongPtr<T>& other)
	{
		if (other._ptr)
			other._ptr->incStrong(this);
		if (_ptr)
			_ptr->decStrong(this);
		_ptr = other._ptr;
		return *this;
	}
	StrongPtr<T>& operator=(T* other)
	{
		if (other)
			other->incStrong(this);
		if (_ptr)
			_ptr->decStrong(this);
		_ptr = other;
		return *this;
	}
	template<typename U>
	StrongPtr<T>& operator=(const StrongPtr<U>& other)
	{
		if (other._ptr)
			other._ptr->incStrong(this);
		if (_ptr)
			_ptr->decStrong(this);
		_ptr = other._ptr;
		return *this;
	}
	template<typename U>
	StrongPtr<T>& operator=(U* other)
	{
		if (other)
			((T*)other)->incStrong(this);
		if (_ptr)
			_ptr->decStrong(this);
		_ptr = other;
		return *this;
	}

	//
	// accessors
	//
	inline T& operator*() const
	{
		return *_ptr;
	}
	inline T* operator->() const
	{
		return _ptr;
	}
	inline T* get() const
	{
		return _ptr;
	}

	//
	// operator==
	//
	inline bool operator==(const StrongPtr<T>& o) const
	{
		return _ptr == o._ptr;
	}
	inline bool operator==(const T* o) const
	{
		return _ptr == o;
	}
	template<typename U>
	inline bool operator==(const StrongPtr<U>& o) const
	{
		return _ptr == o._ptr;
	}
	template<typename U>
	inline bool operator==(const U* o) const
	{
		return _ptr == o;
	}
	inline bool operator==(const WeakPtr<T>& o) const
	{
		return _ptr == o._ptr;
	}
	template<typename U>
	inline bool operator==(const WeakPtr<U>& o) const
	{
		return _ptr == o._ptr;
	}

	//
	// operator!=
	//
	inline bool operator!=(const StrongPtr<T>& o) const
	{
		return _ptr != o._ptr;
	}
	inline bool operator!=(const T* o) const
	{
		return _ptr != o;
	}
	template<typename U>
	inline bool operator!=(const StrongPtr<U>& o) const
	{
		return _ptr != o._ptr;
	}
	template<typename U>
	inline bool operator!=(const U* o) const
	{
		return _ptr != o;
	}
	inline bool operator!=(const WeakPtr<T>& o) const
	{
		return _ptr != o._ptr;
	}
	template<typename U>
	inline bool operator!=(const WeakPtr<U>& o) const
	{
		return _ptr != o._ptr;
	}

	//
	// operator>
	//
	inline bool operator>(const StrongPtr<T>& o) const
	{
		return _ptr > o._ptr;
	}
	inline bool operator>(const T* o) const
	{
		return _ptr > o;
	}
	template<typename U>
	inline bool operator>(const StrongPtr<U>& o) const
	{
		return _ptr > o._ptr;
	}
	template<typename U>
	inline bool operator>(const U* o) const
	{
		return _ptr > o;
	}
	inline bool operator>(const WeakPtr<T>& o) const
	{
		return _ptr > o._ptr;
	}
	template<typename U>
	inline bool operator>(const WeakPtr<U>& o) const
	{
		return _ptr > o._ptr;
	}

	//
	// operator<
	//
	inline bool operator<(const StrongPtr<T>& o) const
	{
		return _ptr < o._ptr;
	}
	inline bool operator<(const T* o) const
	{
		return _ptr < o;
	}
	template<typename U>
	inline bool operator<(const StrongPtr<U>& o) const
	{
		return _ptr < o._ptr;
	}
	template<typename U>
	inline bool operator<(const U* o) const
	{
		return _ptr < o;
	}
	inline bool operator<(const WeakPtr<T>& o) const
	{
		return _ptr < o._ptr;
	}
	template<typename U>
	inline bool operator<(const WeakPtr<U>& o) const
	{
		return _ptr < o._ptr;
	}

	//
	// operator<=
	//
	inline bool operator<=(const StrongPtr<T>& o) const
	{
		return _ptr <= o._ptr;
	}
	inline bool operator<=(const T* o) const
	{
		return _ptr <= o;
	}
	template<typename U>
	inline bool operator<=(const StrongPtr<U>& o) const
	{
		return _ptr <= o._ptr;
	}
	template<typename U>
	inline bool operator<=(const U* o) const
	{
		return _ptr <= o;
	}
	inline bool operator<=(const WeakPtr<T>& o) const
	{
		return _ptr <= o._ptr;
	}
	template<typename U>
	inline bool operator<=(const WeakPtr<U>& o) const
	{
		return _ptr <= o._ptr;
	}

	//
	// operator>=
	//
	inline bool operator>=(const StrongPtr<T>& o) const
	{
		return _ptr >= o._ptr;
	}
	inline bool operator>=(const T* o) const
	{
		return _ptr >= o;
	}
	template<typename U>
	inline bool operator>=(const StrongPtr<U>& o) const
	{
		return _ptr >= o._ptr;
	}
	template<typename U>
	inline bool operator>=(const U* o) const
	{
		return _ptr >= o;
	}
	inline bool operator>=(const WeakPtr<T>& o) const
	{
		return _ptr >= o._ptr;
	}
	template<typename U>
	inline bool operator>=(const WeakPtr<U>& o) const
	{
		return _ptr >= o._ptr;
	}

	//
	// operator!
	//
	inline bool operator!() const
	{
		return _ptr == NULL;
	}

private:
	template<typename Y> friend class WeakPtr;
	template<typename Y> friend class StrongPtr;
private:
	T* _ptr;
};

template <typename T>
class WeakPtr
{
public:
	inline WeakPtr()
		: _ptr(0)
	{

	}
	WeakPtr(T* other)
		: _ptr(other)
		, _ref(_ptr ? _ptr->_ref : NULL)
	{
		if (_ptr)
			_ref->incWeak(this);
	}
	WeakPtr(const WeakPtr<T>& other)
		: _ptr(other._ptr)
		, _ref(other._ref)
	{
		if (_ptr)
			_ref->incWeak(this);
	}
	WeakPtr(const StrongPtr<T>& other)
		: _ptr(other._ptr)
		, _ref(_ptr ? _ptr->_ref : NULL)
	{
		if (_ptr)
			_ref->incWeak(this);
	}
	template<typename U>
	WeakPtr(U* other)
		: _ptr(other)
		, _ref(_ptr ? _ptr->_ref : NULL)
	{
		if (_ptr)
			_ref->incWeak(this);
	}
	template<typename U>
	WeakPtr(const StrongPtr<U>& other)
		: _ptr(other._ptr)
		, _ref(_ptr ? _ptr->_ref : NULL)
	{
		if (_ptr)
			_ref->incWeak(this);
	}
	template<typename U>
	WeakPtr(const WeakPtr<U>& other)
		: _ptr(other._ptr)
		, _ref(_ptr ? _ptr->_ref : NULL)
	{
		if (_ptr)
			_ref->incWeak(this);
	}
	~WeakPtr()
	{
		if (_ptr)
			_ref->decWeak(this);
	}
	void clear()
	{
		if (_ptr)
		{
			_ref->decWeak(this);
			_ptr = 0;
		}
	}
	StrongPtr<T> promote() const
	{
		StrongPtr<T> result;
		if (_ptr && _ref->attemptIncStrong(&result))
			result._ptr = _ptr;
		return result;
	}

	//
	// operator=
	//
	WeakPtr& operator=(T* other)
	{
		if (other)
			other->_ref->incWeak(this);
		if (_ptr)
			_ref->decWeak(this);
		_ptr = other;
		_ref = other->_ref;
		return *this;
	}
	WeakPtr& operator=(const WeakPtr<T>& other)
	{
		if (other._ptr)
			other._ref->incWeak(this);
		if (_ptr)
			_ref->decWeak(this);
		_ptr = other._ptr;
		_ref = other._ref;
		return *this;
	}
	WeakPtr& operator=(const StrongPtr<T>& other)
	{
		if (other._ptr)
			other->_ref->incWeak(this);
		if (_ptr)
			_ref->decWeak(this);
		_ptr = other._ptr;
		_ref = other->_ref;
		return *this;
	}
	template<typename U>
	WeakPtr& operator=(U* other)
	{
		if (other)
			other->_ref->incWeak(this);
		if (_ptr)
			_ref->decWeak(this);
		_ptr = other;
		_ref = other->_ref;
		return *this;
	}
	template<typename U>
	WeakPtr& operator=(const WeakPtr<U>& other)
	{
		if (other._ptr)
			other._ref->incWeak(this);
		if (_ptr)
			_ref->decWeak(this);
		_ptr = other._ptr;
		_ref = other._ref;
		return *this;
	}
	template<typename U>
	WeakPtr& operator=(const StrongPtr<U>& other)
	{
		if (other._ptr)
			other._ref->incWeak(this);
		if (_ptr)
			_ref->decWeak(this);
		_ptr = other._ptr;
		_ref = other._ref;
		return *this;
	}

	// operator==
	inline bool operator==(const StrongPtr<T>& o) const
	{
		return _ptr == o._ptr;
	}
	inline bool operator==(const T* o) const
	{
		return _ptr == o;
	}
	template<typename U>
	inline bool operator==(const StrongPtr<U>& o) const
	{
		return _ptr == o._ptr;
	}
	template<typename U>
	inline bool operator==(const U* o) const
	{
		return _ptr == o;
	}
	inline bool operator==(const WeakPtr<T>& o) const
	{
		return (_ptr == o._ptr) && (_ref == o._ref);
	}
	template<typename U> inline bool operator==(const WeakPtr<U>& o) const
	{
		return _ptr == o._ptr;
	}

	// operator!=
	inline bool operator!=(const StrongPtr<T>& o) const
	{
		return _ptr != o._ptr;
	}
	inline bool operator!=(const T* o) const
	{
		return _ptr != o;
	}
	template<typename U>
	inline bool operator!=(const StrongPtr<U>& o) const
	{
		return _ptr != o._ptr;
	}
	template<typename U>
	inline bool operator!=(const U* o) const
	{
		return _ptr != o;
	}
	inline bool operator!=(const WeakPtr<T>& o) const
	{
		return _ref != o._ref;
	}
	template<typename U> inline bool operator!=(const WeakPtr<U>& o) const
	{
		return !operator==(o);
	}

	// operator<
	inline bool operator<(const StrongPtr<T>& o) const
	{
		return _ptr < o._ptr;
	}
	inline bool operator<(const T* o) const
	{
		return _ptr < o;
	}
	template<typename U>
	inline bool operator<(const StrongPtr<U>& o) const
	{
		return _ptr < o._ptr;
	}
	template<typename U>
	inline bool operator<(const U* o) const
	{
		return _ptr < o;
	}
	inline bool operator<(const WeakPtr<T>& o) const
	{
		return (_ptr == o._ptr) ? (_ref < o._ref) : (_ptr < o._ptr);
	}
	template<typename U> inline bool operator<(const WeakPtr<U>& o) const
	{
		return (_ptr == o._ptr) ? (_ref < o._ref) : (_ptr < o._ptr);
	}

	// operator>
	inline bool operator>(const StrongPtr<T>& o) const
	{
		return _ptr > o._ptr;
	}
	inline bool operator>(const T* o) const
	{
		return _ptr > o;
	}
	template<typename U>
	inline bool operator>(const StrongPtr<U>& o) const
	{
		return _ptr > o._ptr;
	}
	template<typename U>
	inline bool operator>(const U* o) const
	{
		return _ptr > o;
	}
	inline bool operator>(const WeakPtr<T>& o) const
	{
		return (_ptr == o._ptr) ? (_ref > o._ref) : (_ptr > o._ptr);
	}
	template<typename U>
	inline bool operator>(const WeakPtr<U>& o) const
	{
		return (_ptr == o._ptr) ? (_ref > o._ref) : (_ptr > o._ptr);
	}

	// operator<=
	inline bool operator<=(const StrongPtr<T>& o) const
	{
		return _ptr <= o._ptr;
	}
	inline bool operator<=(const T* o) const
	{
		return _ptr <= o;
	}
	template<typename U>
	inline bool operator<=(const StrongPtr<U>& o) const
	{
		return _ptr <= o._ptr;
	}
	template<typename U>
	inline bool operator<=(const U* o) const
	{
		return _ptr <= o;
	}
	inline bool operator<=(const WeakPtr<T>& o) const
	{
		return !operator>(o);
	}
	template<typename U> inline bool operator<=(const WeakPtr<U>& o) const
	{
		return !operator>(o);
	}

	// operator>=
	inline bool operator>=(const StrongPtr<T>& o) const
	{
		return _ptr >= o._ptr;
	}
	inline bool operator>=(const T* o) const
	{
		return _ptr >= o;
	}
	template<typename U>
	inline bool operator>=(const StrongPtr<U>& o) const
	{
		return _ptr >= o._ptr;
	}
	template<typename U>
	inline bool operator>=(const U* o) const
	{
		return _ptr >= o;
	}
	inline bool operator>=(const WeakPtr<T>& o) const
	{
		return !operator<(o);
	}
	template<typename U> inline bool operator>=(const WeakPtr<U>& o) const
	{
		return !operator<(o);
	}

private:
	typedef typename Refable::Ref Ref;
	template<typename Y> friend class StrongPtr;
	template<typename Y> friend class WeakPtr;
private:
	T* _ptr;
	Refable::Ref* _ref;
};

#endif // BPSLAB_REF_H
