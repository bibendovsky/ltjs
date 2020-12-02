#ifndef LTJS_OAL_OBJECT_INCLUDED
#define LTJS_OAL_OBJECT_INCLUDED


#include <memory>
#include <utility>
#include <type_traits>

#include "al.h"
#include "alc.h"
#include "alext.h"

#include "ltjs_oal_efx_symbols.h"


namespace ltjs
{
namespace oal
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

enum class ObjectDeleterStateType
{
	mono,
	unique,
}; // ObjectDeleterStateType

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


namespace detail
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

template<
	typename TValue,
	typename TDeleter,
	ObjectDeleterStateType TDeleterStateType
>
class ObjectStorage;

template<
	typename TValue,
	typename TDeleter
>
class ObjectStorage<TValue, TDeleter, ObjectDeleterStateType::mono>
{
public:
	using Value = TValue;
	using Deleter = TDeleter;


	ObjectStorage() noexcept
		:
		value_{}
	{
	}

	explicit ObjectStorage(
		Value value) noexcept
		:
		value_{value}
	{
	}

	ObjectStorage(
		const ObjectStorage& rhs) = delete;

	ObjectStorage(
		ObjectStorage&& rhs) noexcept
		:
		ObjectStorage{}
	{
		std::swap(value_, rhs.value_);
	}

	ObjectStorage& operator=(
		const ObjectStorage& rhs) = delete;

	void operator=(
		ObjectStorage&& rhs) noexcept
	{
		destroy();
		std::swap(value_, rhs.value_);
	}

	~ObjectStorage()
	{
		destroy();
	}


	bool has_value() const noexcept
	{
		return value_ != Value{};
	}

	Value get_value() const noexcept
	{
		return value_;
	}

	void reset() noexcept
	{
		destroy();
	}

	void reset(
		Value value) noexcept
	{
		destroy();
		value_ = value;
	}

	void release() noexcept
	{
		const auto value = value_;
		value_ = Value{};
		return value;
	}

	void destroy() noexcept
	{
		if (value_ != Value{})
		{
			(Deleter{})(value_);
			value_ = Value{};
		}
	}


private:
	Value value_;
}; // ObjectStorage

template<
	typename TValue,
	typename TDeleter>
class ObjectStorage<TValue, TDeleter, ObjectDeleterStateType::unique>
{
public:
	using Value = TValue;
	using Deleter = TDeleter;


	explicit ObjectStorage(
		Deleter&& deleter) noexcept
		:
		value_{},
		deleter_{deleter}
	{
	}

	ObjectStorage(
		Value value,
		Deleter&& deleter) noexcept
		:
		value_{value},
		deleter_{deleter}
	{
	}

	ObjectStorage(
		const ObjectStorage& rhs) = delete;

	ObjectStorage(
		ObjectStorage&& rhs) noexcept
		:
		value_{},
		deleter_{}
	{
		std::swap(value_, rhs.value_);
		std::swap(deleter_, rhs.deleter_);
	}

	ObjectStorage& operator=(
		const ObjectStorage& rhs) = delete;

	void operator=(
		ObjectStorage&& rhs) noexcept
	{
		destroy();
		std::swap(value_, rhs.value_);
		std::swap(deleter_, rhs.deleter_);
	}

	~ObjectStorage()
	{
		destroy();
	}


	bool has_value() const noexcept
	{
		return value_ != Value{};
	}

	Value get_value() const noexcept
	{
		return value_;
	}

	void reset() noexcept
	{
		destroy();
	}

	void reset(
		Value value) noexcept
	{
		destroy();
		value_ = value;
	}

	void release() noexcept
	{
		const auto value = value_;
		value_ = Value{};
		return value;
	}

	void destroy() noexcept
	{
		if (value_ != Value{})
		{
			deleter_(value_);
			value_ = Value{};
		}
	}


private:
	Value value_;
	Deleter deleter_;
}; // ObjectStorage

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // detail


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

template<
	typename TDeleter,
	ObjectDeleterStateType TDeleterStateType
>
class Object
{
public:
	using Value = ALuint;
	using Deleter = TDeleter;
	static constexpr auto deleter_state_type = TDeleterStateType;


	template<
		ObjectDeleterStateType UDeleterStateType = deleter_state_type,
		std::enable_if_t<UDeleterStateType == ObjectDeleterStateType::mono, int> = 0
	>
	Object() noexcept
		:
		storage_{}
	{
	}

	template<
		ObjectDeleterStateType UDeleterStateType = deleter_state_type,
		std::enable_if_t<UDeleterStateType == ObjectDeleterStateType::unique, int> = 0
	>
	explicit Object(
		Deleter&& deleter) noexcept
		:
		storage_{std::forward<Deleter>(deleter)}
	{
	}

	template<
		ObjectDeleterStateType UDeleterStateType = deleter_state_type,
		std::enable_if_t<UDeleterStateType == ObjectDeleterStateType::mono, int> = 0
	>
	explicit Object(
		Value value) noexcept
		:
		storage_{value}
	{
	}

	template<
		ObjectDeleterStateType UDeleterStateType = deleter_state_type,
		std::enable_if_t<UDeleterStateType == ObjectDeleterStateType::unique, int> = 0
	>
	explicit Object(
		Value value,
		Deleter&& deleter) noexcept
		:
		storage_{value, std::forward<Deleter>(deleter)}
	{
	}

	Object(
		const Object& rhs) = delete;

	Object(
		Object&& rhs) noexcept
		:
		storage_{std::move(rhs.storage_)}
	{
	}

	Object& operator=(
		const Object& rhs) = delete;

	void operator=(
		Object&& rhs) noexcept
	{
		storage_ = std::move(rhs.storage_);
	}

	~Object()
	{
		storage_.destroy();
	}


	bool has_value() const noexcept
	{
		return storage_.has_value();
	}

	Value get() const noexcept
	{
		return storage_.get_value();
	}

	void reset() noexcept
	{
		storage_.reset();
	}

	void reset(
		Value value) noexcept
	{
		storage_.reset(value);
	}

	Value release() noexcept
	{
		return storage_.release();
	}


private:
	using Storage = detail::ObjectStorage<Value, Deleter, deleter_state_type>;


	Storage storage_;
}; // Object

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

struct DeviceObjectDeleter
{
	void operator()(
		ALCdevice* al_device) noexcept;
}; // DeviceObjectDeleter

using DeviceObjectUPtr = std::unique_ptr<ALCdevice, DeviceObjectDeleter>;

DeviceObjectUPtr make_device_object(
	const char* device_name);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

struct ContextObjectDeleter
{
	void operator()(
		ALCcontext* al_context) noexcept;
}; // ContextObjectDeleter

using ContextObjectUPtr = std::unique_ptr<ALCcontext, ContextObjectDeleter>;

ContextObjectUPtr make_context_object(
	ALCdevice* al_device,
	ALCint* al_attributes);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

struct BufferObjectDeleter
{
	void operator()(
		ALuint al_buffer) noexcept;
}; // BufferObjectDeleter

using BufferObject = Object<BufferObjectDeleter, ObjectDeleterStateType::mono>;

BufferObject make_buffer_object();

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

struct SourceObjectDeleter
{
	void operator()(
		ALuint al_buffer) noexcept;
}; // SourceObjectDeleter

using SourceObject = Object<SourceObjectDeleter, ObjectDeleterStateType::mono>;

SourceObject make_source_object();

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class EffectSlotObjectDeleter
{
public:
	EffectSlotObjectDeleter() noexcept;

	explicit EffectSlotObjectDeleter(
		LPALDELETEAUXILIARYEFFECTSLOTS al_deleter) noexcept;

	void operator()(
		ALuint al_effect_slot) noexcept;


private:
	LPALDELETEAUXILIARYEFFECTSLOTS al_deleter_;
}; // EffectSlotObjectDeleter

using EffectSlotObject = Object<EffectSlotObjectDeleter, ObjectDeleterStateType::unique>;

EffectSlotObject make_effect_slot_object(
	const EfxSymbols& efx_symbols);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class EffectObjectDeleter
{
public:
	EffectObjectDeleter() noexcept;

	explicit EffectObjectDeleter(
		LPALDELETEEFFECTS al_deleter) noexcept;

	void operator()(
		ALuint al_effect) noexcept;


private:
	LPALDELETEEFFECTS al_deleter_;
}; // EffectObjectDeleter

using EffectObject = Object<EffectObjectDeleter, ObjectDeleterStateType::unique>;

EffectObject make_effect_object(
	const EfxSymbols& efx_symbols);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class FilterObjectDeleter
{
public:
	FilterObjectDeleter() noexcept;

	explicit FilterObjectDeleter(
		LPALDELETEFILTERS al_deleter) noexcept;

	void operator()(
		ALuint al_filter) noexcept;


private:
	LPALDELETEFILTERS al_deleter_;
}; // FilterObjectDeleter

using FilterObject = Object<FilterObjectDeleter, ObjectDeleterStateType::unique>;

FilterObject make_filter_object(
	const EfxSymbols& efx_symbols);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // oal
} // ltjs


#endif // !LTJS_OAL_OBJECT_INCLUDED
