#ifndef LTJS_OAL_LT_FILTER_INCLUDED
#define LTJS_OAL_LT_FILTER_INCLUDED


#include <memory>
#include <string>
#include <vector>

#include "al.h"

#include "iltsound.h"


namespace ltjs
{
namespace oal
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

enum class LtFilterState
{
	disable,
	enable,
}; // LtFilterState

struct LtFilterInfo
{
	using FeatureNames = std::vector<std::string>;


	std::string name;
	FeatureNames feature_names;
}; // LtFilterInfo

class LtFilter
{
public:
	LtFilter() noexcept;

	virtual ~LtFilter();


	virtual const LtFilterInfo& get_info() const noexcept = 0;


	virtual void initialize_source(
		ALuint al_source,
		int& lt_filter_direct_mb) = 0;

	virtual void set_listener(
		LtFilterState filter_state,
		const LTSOUNDFILTERDATA& lt_filter_data) = 0;

	virtual void set_source(
		ALuint al_source,
		const LTSOUNDFILTERDATA& lt_filter_data,
		int& lt_filter_direct_mb) = 0;
}; // LtFilter

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using LtFilterUPtr = std::unique_ptr<LtFilter>;

LtFilterUPtr make_lt_filter();

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // oal
} // ltjs


#endif // !LTJS_OAL_LT_FILTER_INCLUDED
