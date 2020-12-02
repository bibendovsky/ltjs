#ifndef LTJS_OAL_SYSTEM_INCLUDED
#define LTJS_OAL_SYSTEM_INCLUDED


#include <memory>
#include <string>


namespace ltjs
{
namespace oal
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

struct SystemInfo
{
	std::string device_name;
	std::string renderer;
	std::string vendor;
	std::string version;
}; // SystemInfo

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class System
{
public:
	System() noexcept;

	virtual ~System();


	virtual const SystemInfo& get_info() const noexcept = 0;
}; // System

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

struct SystemParam
{
	const char* device_name;
}; // SystemParam

using SystemUPtr = std::unique_ptr<System>;

SystemUPtr make_system(
	const SystemParam& param);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // oal
} // ltjs


#endif // !LTJS_OAL_SYSTEM_INCLUDED
