#ifndef LTJS_RESOURCE_STRINGS_INCLUDED
#define LTJS_RESOURCE_STRINGS_INCLUDED


#include <memory>
#include <string>

#include "ltjs_launcher_search_paths.h"


namespace ltjs
{
namespace launcher
{


class ResourceStrings final
{
public:
	static constexpr auto max_file_size = 16 * 1'024;


	ResourceStrings();

	ResourceStrings(
		ResourceStrings&& rhs);

	~ResourceStrings();


	const std::string& get_error_message() const;


	bool initialize(
		const SearchPaths& search_path,
		const std::string& file_name);

	void uninitialize();

	bool is_initialized() const;


	const std::string& get(
		const int id) const;

	const std::string& get(
		const int id,
		const std::string& default_string) const;

	const std::string& operator[](
		const int id) const;


private:
	class Impl;

	using ImplUPtr = std::unique_ptr<Impl>;

	ImplUPtr impl_;
}; // ResourceStrings


} // launcher
} // ltjs


#endif // !LTJS_RESOURCE_STRINGS_INCLUDED
