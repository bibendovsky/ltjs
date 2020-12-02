#include "ltjs_oal_lt_filter.h"

#include "ltjs_exception.h"

#include "ltjs_oal_eax_lt_filter.h"
#include "ltjs_oal_efx_lt_filter.h"


namespace ltjs
{
namespace oal
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

LtFilter::LtFilter() noexcept = default;

LtFilter::~LtFilter() = default;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class LtFilterException :
	public SafeException
{
public:
	explicit LtFilterException(
		const char* message)
		:
		SafeException{message}
	{
	}
}; // LtFilterException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using LtFilterUPtr = std::unique_ptr<LtFilter>;

LtFilterUPtr make_lt_filter()
{
#if 1
	try
	{
		return std::make_unique<EfxLtFilter>();
	}
	catch (...)
	{
	}
#endif

#if 1
	try
	{
		return std::make_unique<EaxLtFilter>();
	}
	catch (...)
	{
	}
#endif

	throw LtFilterException{"No suitable LT filter."};
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // oal
} // ltjs
