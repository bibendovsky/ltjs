#include "ltjs_oal_system.h"

#include <cassert>

#include <string>

#include "alc.h"
#include "alext.h"

#include "ltjs_exception.h"
#include "ltjs_oal_efx_symbols.h"
#include "ltjs_oal_object.h"


namespace ltjs
{
namespace oal
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

System::System() noexcept = default;

System::~System() = default;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SystemException :
	public SafeException
{
public:
	explicit SystemException(
		const char* message)
		:
		SafeException{message}
	{
	}
}; // SystemException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SystemImpl final :
	public System
{
public:
	SystemImpl(
		const SystemParam& param);


	// ----------------------------------------------------------------------
	// System

	const SystemInfo& get_info() const noexcept override;

	// System
	// ----------------------------------------------------------------------


private:
	DeviceObjectUPtr device_{};
	ContextObjectUPtr context_{};
	SystemInfo info_{};


	const char* get_al_string(
		ALenum al_param);

	const char* get_alc_string(
		ALCdevice* al_device,
		ALenum al_param);
}; // SystemImpl

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SystemImpl::SystemImpl(
	const SystemParam& param)
{
	device_ = make_device_object(param.device_name);
	context_ = make_context_object(device_.get(), nullptr);

	const auto al_is_context_current = (alcMakeContextCurrent(context_.get()) != ALC_FALSE);

	if (!al_is_context_current)
	{
		throw SystemException{"Failed to make context current."};
	}


	// Strings.
	//

	info_.device_name = get_alc_string(device_.get(), ALC_DEVICE_SPECIFIER);
	info_.renderer = get_al_string(AL_RENDERER);
	info_.vendor = get_al_string(AL_VENDOR);
	info_.version = get_al_string(AL_VERSION);
}

// ----------------------------------------------------------------------
// System

const SystemInfo& SystemImpl::get_info() const noexcept
{
	return info_;
}

// System
// ----------------------------------------------------------------------

const char* SystemImpl::get_al_string(
	ALenum al_param)
{
	const auto al_string = alGetString(al_param);

	if (al_string == nullptr)
	{
		throw SystemException{"Failed to get AL string."};
	}

	return al_string;
}

const char* SystemImpl::get_alc_string(
	ALCdevice* al_device,
	ALenum al_param)
{
	const auto al_string = alcGetString(al_device, al_param);

	if (al_string == nullptr)
	{
		throw SystemException{"Failed to get ALC string."};
	}

	return al_string;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

SystemUPtr make_system(
	const SystemParam& param)
{
	return std::make_unique<SystemImpl>(param);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // oal
} // ltjs
