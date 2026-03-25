#include "ltjs_oal_object.h"

#include <cassert>

#include "al.h"
#include "alc.h"

#include "ltjs_exception.h"

#include "ltjs_oal_efx_symbols.h"


namespace ltjs
{
namespace oal
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class ObjectException :
	public Exception
{
public:
	explicit ObjectException(
		const char* message) noexcept
		:
		Exception{"OAL_OBJECT", message}
	{
	}
}; // ObjectException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void DeviceObjectDeleter::operator()(
	::ALCdevice* al_device) noexcept
{
	const auto al_result = alcCloseDevice(al_device);
	assert(al_result != ALC_FALSE);
}

DeviceObjectUPtr make_device_object(
	const char* device_name)
{
	const auto al_device = alcOpenDevice(device_name);

	if (al_device == nullptr)
	{
		throw ObjectException{"Failed to open AL device."};
	}

	return DeviceObjectUPtr{al_device};
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void ContextObjectDeleter::operator()(
	::ALCcontext* al_context) noexcept
{
	const auto al_current_context = alcGetCurrentContext();

	if (al_current_context == al_context)
	{
		const auto al_result = alcMakeContextCurrent(nullptr);
		assert(al_result != ALC_FALSE);
	}

	alcDestroyContext(al_context);
}

ContextObjectUPtr make_context_object(
	::ALCdevice* al_device,
	::ALCint* al_attributes)
{
	const auto al_context = alcCreateContext(al_device, al_attributes);

	if (al_context == nullptr)
	{
		throw ObjectException{"Failed to create AL context."};
	}

	return ContextObjectUPtr{al_context};
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


namespace detail
{

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

template<
	typename TObject,
	typename TNullGeneratorException,
	typename TCreateException
>
TObject make_al_object(
	void (AL_APIENTRY* al_generator)(::ALsizei, ::ALuint*))
{
	if (al_generator == nullptr)
	{
		throw TNullGeneratorException{};
	}

	auto al_object = ::ALuint{};

	al_generator(1, &al_object);

	if (al_object == ::ALuint{})
	{
		throw TCreateException{};
	}

	return TObject{al_object};
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

} // detail


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class BufferObjectNullGeneratorException :
	public Exception
{
public:
	BufferObjectNullGeneratorException()
		:
		Exception{"OAL_BUFFER", "Null AL buffer generator function."}
	{
	}
}; // BufferObjectNullGeneratorException

class BufferObjectCreateException :
	public Exception
{
public:
	BufferObjectCreateException()
		:
		Exception{"OAL_BUFFER", "Failed to create AL buffer."}
	{
	}
}; // BufferObjectCreateException


void BufferObjectDeleter::operator()(
	::ALuint al_buffer) noexcept
{
	::alDeleteBuffers(1, &al_buffer);
}

BufferObject make_buffer_object()
{
	return detail::make_al_object<
		BufferObject,
		BufferObjectNullGeneratorException,
		BufferObjectCreateException
	>
	(
		::alGenBuffers
	);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SourceObjectNullGeneratorException :
	public Exception
{
public:
	SourceObjectNullGeneratorException()
		:
		Exception{"OAL_SOURCE", "Null AL source generator function."}
	{
	}
}; // SourceObjectNullGeneratorException

class SourceObjectCreateException :
	public Exception
{
public:
	SourceObjectCreateException()
		:
		Exception{"OAL_SOURCE", "Failed to create AL source."}
	{
	}
}; // SourceObjectCreateException


void SourceObjectDeleter::operator()(
	::ALuint al_buffer) noexcept
{
	::alDeleteSources(1, &al_buffer);
}

SourceObject make_source_object()
{
	return detail::make_al_object<
		SourceObject,
		SourceObjectNullGeneratorException,
		SourceObjectCreateException
	>
	(
		::alGenSources
	);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


namespace detail
{

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

template<
	typename TObject,
	typename TNullException,
	typename TCreateException
>
TObject make_efx_object(
	void (AL_APIENTRY* al_generator)(::ALsizei, ::ALuint*),
	void (AL_APIENTRY* al_deleter)(::ALsizei, const ::ALuint*))
{
	if (al_generator == nullptr || al_deleter == nullptr)
	{
		throw TNullException{};
	}

	auto al_object = ::ALuint{};

	al_generator(1, &al_object);

	if (al_object == ::ALuint{})
	{
		throw TCreateException{};
	}

	using UDeleter = typename TObject::Deleter;

	return TObject{al_object, UDeleter{al_deleter}};
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

} // detail


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class EffectSlotObjectNullFunctionsException :
	public Exception
{
public:
	EffectSlotObjectNullFunctionsException()
		:
		Exception{"OAL_EFX_SLOT", "Null EFX effect slot functions."}
	{
	}
}; // EffectSlotObjectNullFunctionsException

class EffectSlotObjectCreateException :
	public Exception
{
public:
	EffectSlotObjectCreateException()
		:
		Exception{"OAL_EFX_SLOT", "Failed to create EFX effect slot."}
	{
	}
}; // EffectSlotObjectCreateException


EffectSlotObjectDeleter::EffectSlotObjectDeleter() noexcept
	:
	al_deleter_{}
{
}

EffectSlotObjectDeleter::EffectSlotObjectDeleter(
	LPALDELETEAUXILIARYEFFECTSLOTS al_deleter) noexcept
	:
	al_deleter_{al_deleter}
{
	assert(al_deleter_ != nullptr);
}

void EffectSlotObjectDeleter::operator()(
	::ALuint al_effect_slot) noexcept
{
	al_deleter_(1, &al_effect_slot);
}

EffectSlotObject make_effect_slot_object(
	const EfxSymbols& efx_symbols)
{
	return detail::make_efx_object<
		EffectSlotObject,
		EffectSlotObjectNullFunctionsException,
		EffectSlotObjectCreateException>
	(
		efx_symbols.alGenAuxiliaryEffectSlots,
		efx_symbols.alDeleteAuxiliaryEffectSlots
	);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class EffectObjectNullFunctionsException :
	public Exception
{
public:
	EffectObjectNullFunctionsException()
		:
		Exception{"OAL_EFX_EFFECT", "Null EFX effect functions."}
	{
	}
}; // EffectObjectNullFunctionsException

class EffectObjectCreateException :
	public Exception
{
public:
	EffectObjectCreateException()
		:
		Exception{"OAL_EFX_EFFECT", "Failed to create EFX effect."}
	{
	}
}; // EffectObjectCreateException


EffectObjectDeleter::EffectObjectDeleter() noexcept
	:
	al_deleter_{}
{
}

EffectObjectDeleter::EffectObjectDeleter(
	LPALDELETEEFFECTS al_deleter) noexcept
	:
	al_deleter_{al_deleter}
{
	assert(al_deleter_ != nullptr);
}

void EffectObjectDeleter::operator()(
	::ALuint al_effect) noexcept
{
	al_deleter_(1, &al_effect);
}

EffectObject make_effect_object(
	const EfxSymbols& efx_symbols)
{
	return detail::make_efx_object<
		EffectObject,
		EffectObjectNullFunctionsException,
		EffectObjectCreateException>
	(
		efx_symbols.alGenEffects,
		efx_symbols.alDeleteEffects
	);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class FilterObjectNullFunctionsException :
	public Exception
{
public:
	FilterObjectNullFunctionsException()
		:
		Exception{"OAL_EFX_FILTER", "Null EFX filter functions."}
	{
	}
}; // FilterObjectNullFunctionsException

class FilterObjectCreateException :
	public Exception
{
public:
	FilterObjectCreateException()
		:
		Exception{"OAL_EFX_FILTER", "Failed to create EFX filter."}
	{
	}
}; // FilterObjectCreateException


FilterObjectDeleter::FilterObjectDeleter() noexcept
	:
	al_deleter_{}
{
}

FilterObjectDeleter::FilterObjectDeleter(
	LPALDELETEFILTERS al_deleter) noexcept
	:
	al_deleter_{al_deleter}
{
	assert(al_deleter_ != nullptr);
}

void FilterObjectDeleter::operator()(
	::ALuint al_filter) noexcept
{
	al_deleter_(1, &al_filter);
}

FilterObject make_filter_object(
	const EfxSymbols& efx_symbols)
{
	return detail::make_efx_object<
		FilterObject,
		FilterObjectNullFunctionsException,
		FilterObjectCreateException>
	(
		efx_symbols.alGenFilters,
		efx_symbols.alDeleteFilters
	);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // oal
} // ltjs
