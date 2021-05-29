#ifndef LTJS_FILE_SYSTEM_PATH_INCLUDED
#define LTJS_FILE_SYSTEM_PATH_INCLUDED


#include <string>

#include "ltjs_index_type.h"


namespace ltjs
{
namespace file_system
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

constexpr auto native_separator =
#if _WIN32
		'\\'
#else
		'/'
#endif // _WIN32
;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class Path
{
public:
	Path();

	Path(
		const char* rhs_path,
		Index rhs_size);

	explicit Path(
		const char* rhs_path);

	Path(
		const Path& rhs_path);

	Path(
		Path&& rhs_path) noexcept;


	Path& operator=(
		const char* rhs_path);

	Path& operator=(
		const Path& rhs_path);

	Path& operator=(
		Path&& rhs_path) noexcept;


	bool is_empty() const noexcept;

	const char* get_data() const noexcept;

	Index get_size() const noexcept;


	Path& assign(
		const char* rhs_path,
		Index rhs_size);

	Path& assign(
		const char* rhs_path);

	Path& assign(
		const Path& rhs_path);


	Path& append(
		const char* rhs_path,
		Index rhs_size);

	Path& append(
		const char* rhs_path);

	Path& append(
		const Path& rhs_path);


	void clear();


private:
	struct CapacityTag{};
	struct SizeTag{};


	using Data = std::string;


	Data data_{};


	friend Path operator/(
		const char* lhs_path,
		const Path& rhs_path);

	friend Path operator/(
		const Path& lhs_path,
		const char* rhs_path);

	friend Path operator/(
		const Path& lhs_path,
		const Path& rhs_path);


	Path(
		CapacityTag,
		Index capacity);

	Path(
		SizeTag,
		Index size);

	char* get_data() noexcept;

	void normalize_separators(
		Index offset);

	void set_capacity(
		Index size);

	void add_capacity(
		Index size);

	void resize(
		Index size);
}; // Path

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

Path& operator/=(
	Path& lhs,
	const char* rhs_path);

Path& operator/=(
	Path& lhs,
	const Path& rhs_path);


Path operator/(
	const char* lhs_path,
	const Path& rhs_path);

Path operator/(
	const Path& lhs_path,
	const char* rhs_path);

Path operator/(
	const Path& lhs_path,
	const Path& rhs_path);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // file_system
} // ltjs


#endif // !LTJS_FILE_SYSTEM_PATH_INCLUDED
