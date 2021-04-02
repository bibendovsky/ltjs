#include "ltjs_sdl_subsystem.h"


#include <algorithm>

#include "SDL.h"

#include "ltjs_exception.h"
#include "ltjs_sdl_ensure_result.h"


namespace ltjs
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

class SdlSubsystemException :
	public Exception
{
public:
	explicit SdlSubsystemException(
		const char* message)
		:
		Exception{"LTJS_SDL_SUBSYSTEM", message}
	{
	}
}; // SdlSubsystemException

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

SdlSubsystem::SdlSubsystem() noexcept = default;

SdlSubsystem::SdlSubsystem(
	Id id)
{
	switch (id)
	{
		case SDL_INIT_EVENTS:
		case SDL_INIT_GAMECONTROLLER:
		case SDL_INIT_JOYSTICK:
		case SDL_INIT_VIDEO:
			break;

		default:
			throw SdlSubsystemException{"Unsupported SDL subsystem."};
	}

	sdl_ensure_result(::SDL_InitSubSystem(id));

	id_ = id;
}

SdlSubsystem::SdlSubsystem(
	SdlSubsystem&& rhs) noexcept
	:
	id_{rhs.id_}
{
	rhs.id_ = 0;
}

void SdlSubsystem::operator=(
	SdlSubsystem&& rhs) noexcept
{
	close();

	std::swap(id_, rhs.id_);
}

SdlSubsystem::~SdlSubsystem()
{
	close();
}

SdlSubsystem::operator bool() const noexcept
{
	return id_ != 0;
}

void SdlSubsystem::close()
{
	if (id_ == 0)
	{
		return;
	}

	::SDL_QuitSubSystem(id_);
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // ltjs
