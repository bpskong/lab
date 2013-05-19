#include <atomic.h>
#include <ref.h>

static const int32_t _initialStrongValue = (1 << 28);

inline static bool _isStrongLifeTime(int32_t flags)
{
	return ((flags & Refable::lifetimeMask) == Refable::lifetimeStrong);
}

inline static bool _isWeakLifeTime(int32_t flags)
{
	return ((flags & Refable::lifetimeMask) == Refable::lifetimeWeak);
}

class RefImpl
	: public Refable::Ref
{
public:
	volatile int32_t strong;
	volatile int32_t weak;
	Refable* const host;
	volatile int32_t flags;

public:
	RefImpl(Refable* const h)
		: host(h)
	{
		strong = _initialStrongValue;
		weak = 0;
		flags = 0;
	}
};

void Refable::Ref::incWeak(const void* id)
{
	RefImpl* const impl = static_cast<RefImpl*>(this);
	atomic_inc(&impl->weak);
}

void Refable::Ref::decWeak(const void* id)
{
	RefImpl* const impl = static_cast<RefImpl*>(this);
	const int32_t oldWeak = atomic_dec(&impl->weak);
	if (oldWeak == 1)
	{
		if (_isStrongLifeTime(impl->flags))
		{
			if (impl->strong == _initialStrongValue)
				delete impl->host;
			else
				delete impl;
		}
		else
		{
			impl->host->onLastWeakRef(id);
			if (_isWeakLifeTime(impl->flags))
				delete impl->host;
		}
	}
}

bool Refable::Ref::attemptIncStrong(const void* id)
{
	RefImpl* const impl = static_cast<RefImpl*>(this);
	incWeak(id);

	int32_t oldStrong = impl->strong;
	while (oldStrong > 0 && oldStrong != _initialStrongValue)
	{
		if (atomic_exch(oldStrong, oldStrong + 1, &impl->strong) == 0)
			break;
		oldStrong = impl->strong;
	}

	if (oldStrong <= 0 || oldStrong == _initialStrongValue)
	{
		bool allow = false;
		if (oldStrong == _initialStrongValue)
			allow = (!_isWeakLifeTime(impl->flags) ||
					 impl->host->onIncStrongAttempted(firstIncStrong, id));
		else
			allow = (_isWeakLifeTime(impl->flags) &&
					 impl->host->onIncStrongAttempted(firstIncStrong, id));
		if (!allow)
		{
			decWeak(id);
			return false;
		}
		oldStrong = atomic_inc(&impl->strong);

		if (oldStrong > 0 && oldStrong < _initialStrongValue)
			impl->host->onLastStrongRef(id);
	}

	if (oldStrong == _initialStrongValue)
	{
		atomic_add(-_initialStrongValue, &impl->strong);
		impl->host->onFirstRef();
	}

	return true;
}

Refable::Refable()
{
	_ref = new RefImpl(this);
}

Refable::~Refable()
{
	RefImpl* const impl = static_cast<RefImpl*>(_ref);
	if ((impl->strong == _initialStrongValue) ||
		(!_isStrongLifeTime(impl->flags) && impl->weak == 0))
		delete impl;
}

void Refable::incStrong(const void* id) const
{
	RefImpl* const impl = static_cast<RefImpl*>(_ref);
	impl->incWeak(id);
	const int32_t oldStrong = atomic_inc(&impl->strong);
	if (oldStrong == _initialStrongValue)
	{
		atomic_add(-_initialStrongValue, &impl->strong);
		impl->host->onFirstRef();
	}
}

void Refable::decStrong(const void* id) const
{
	RefImpl* const impl = static_cast<RefImpl*>(_ref);
	const int32_t oldStrong = atomic_dec(&impl->strong);
	if (oldStrong == 1)
	{
		impl->host->onLastStrongRef(id);
		if (_isStrongLifeTime(impl->flags))
			delete this;
	}
	impl->decWeak(id);
}

void Refable::extendObjectLifetime(int32_t mode)
{
	RefImpl* const impl = static_cast<RefImpl*>(_ref);
	atomic_or(mode, &impl->flags);
}

void Refable::onFirstRef()
{
}

void Refable::onLastStrongRef(const void*)
{
}

bool Refable::onIncStrongAttempted(uint32_t flags, const void*)
{
	return (flags & firstIncStrong) ? true : false;
}

void Refable::onLastWeakRef(const void*)
{
}
