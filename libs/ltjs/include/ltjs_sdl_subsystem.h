#ifndef LTJS_SDL_SUBSYSTEM_INCLUDED
#define LTJS_SDL_SUBSYSTEM_INCLUDED


namespace ltjs
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

class SdlSubsystem
{
public:
	using Id = unsigned int;


	SdlSubsystem() noexcept;

	explicit SdlSubsystem(
		Id id);

	SdlSubsystem(
		const SdlSubsystem& rhs) = delete;

	SdlSubsystem(
		SdlSubsystem&& rhs) noexcept;

	SdlSubsystem& operator=(
		const SdlSubsystem& rhs) = delete;

	void operator=(
		SdlSubsystem&& rhs) noexcept;

	~SdlSubsystem();


	explicit operator bool() const noexcept;


private:
	Id id_{};


	void close();
}; // SdlSubsystem

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // ltjs


#endif // !LTJS_SDL_SUBSYSTEM_INCLUDED
