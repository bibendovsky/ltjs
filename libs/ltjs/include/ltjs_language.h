#ifndef LTJS_LANGUAGE_INCLUDED
#define LTJS_LANGUAGE_INCLUDED


#include "ltjs_index_type.h"


namespace ltjs
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

template<
	typename T
>
struct LanguageData
{
	T* data{};
	Index size{};
}; // Language


struct Language
{
	using IdString = LanguageData<const char>;
	using Name = LanguageData<const char>;


	IdString id_string{};
	Name name{};
}; // Language

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs


#endif // !LTJS_LANGUAGE_INCLUDED
