#ifndef LTJS_DMUSIC_SEGMENT_INCLUDED
#define LTJS_DMUSIC_SEGMENT_INCLUDED


#include <memory>
#include <string>


namespace ltjs
{


class DMusicSegment final
{
public:
	DMusicSegment();

	DMusicSegment(
		const DMusicSegment& that) = delete;

	DMusicSegment& operator=(
		const DMusicSegment& that) = delete;

	DMusicSegment(
		DMusicSegment&& that);

	~DMusicSegment();


	bool open(
		const std::string& file_name);

	void close();


	const std::string& get_error_message() const;


private:
	class Impl;


	using ImplUPtr = std::unique_ptr<Impl>;


	ImplUPtr pimpl_;
}; // DMusicSegment



}; // ltjs


#endif // LTJS_DMUSIC_SEGMENT_INCLUDED
