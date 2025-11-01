#ifndef LTJS_RESOURCE_STRINGS_INCLUDED
#define LTJS_RESOURCE_STRINGS_INCLUDED

#include "ltjs_launcher_search_paths.h"
#include <memory>
#include <string>

namespace ltjs::launcher {

class ResourceStrings
{
public:
	constexpr static const int min_file_size = 5;
	constexpr static const int max_file_size = 16 * 1'024;

	ResourceStrings();
	ResourceStrings(ResourceStrings&& rhs) noexcept = default;
	~ResourceStrings();

	const std::string& get_error_message() const;
	bool initialize(const SearchPaths& search_path, const std::string& file_name);
	void uninitialize();
	bool is_initialized() const;

	const std::string& get(int id) const;
	const std::string& get(int id, const std::string& default_string) const;
	const std::string& operator[](int id) const;

private:
	class Impl;
	using ImplUPtr = std::unique_ptr<Impl>;

	ImplUPtr impl_;
};

} // namespace ltjs::launcher

#endif // LTJS_RESOURCE_STRINGS_INCLUDED
