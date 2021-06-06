#ifndef LTJS_OAL_EAX_LT_FILTER_INCLUDED
#define LTJS_OAL_EAX_LT_FILTER_INCLUDED


#include <cstdint>

#include "al.h"

#include "ltjs_eax_api.h"

#include "ltjs_oal_efx_symbols.h"
#include "ltjs_oal_object.h"
#include "ltjs_oal_lt_filter.h"


namespace ltjs
{
namespace oal
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class EaxLtFilter final :
	public LtFilter
{
public:
	EaxLtFilter();


	// -------------------------------------------------------------------------
	// LtFilter

	const LtFilterInfo& get_info() const noexcept;


	void initialize_source(
		::ALuint al_source,
		int& lt_filter_direct_mb) override;

	void set_listener(
		LtFilterState filter_state,
		const LTSOUNDFILTERDATA& lt_filter_data) override;

	void set_source(
		::ALuint al_source,
		const LTSOUNDFILTERDATA& lt_filter_data,
		int& lt_filter_direct_mb) override;

	// LtFilter
	// -------------------------------------------------------------------------


private:
	EAXGet eax_get_{};
	EAXSet eax_set_{};

	SourceObject dummy_source_object_{};

	LtFilterInfo info_{};

	bool is_listener_muted_{};
	bool is_reverb_dirty_{};
	std::int32_t eax_reverb_room_{};
	EAXLISTENERPROPERTIES eax_reverb_{};



	void detect_eax();


	static void ensure_lt_reverb_filter(
		const LTSOUNDFILTERDATA& lt_filter_data);

	static void ensure_al_source(
		::ALuint al_source);


	static EAXLISTENERPROPERTIES make_eax_listener(
		const EAXREVERBPROPERTIES& eax_reverb) noexcept;

	void set_eax_reverb_defaults();


	void set_reverb_environment(
		std::uint32_t environment);

	void set_reverb_room(
		std::int32_t room);

	void set_reverb_room_hf(
		std::int32_t room_hf);

	void set_reverb_room_rolloff_factor(
		float room_rolloff_factor);

	void set_reverb_decay_time(
		float decay_time);

	void set_reverb_decay_hf_ratio(
		float decay_hf_ratio);

	void set_reverb_reflections(
		std::int32_t reflections);

	void set_reverb_reflections_delay(
		float reflections_delay);

	void set_reverb_reverb(
		std::int32_t reverb);

	void set_reverb_reverb_delay(
		float reverb_delay);

	void set_reverb_diffusion(
		float diffusion);

	void set_reverb_environment_size(
		float environment_size);

	void set_reverb_air_absorption_hf(
		float air_absorption_hf);

	void set_eax_listener();

	void set_source_direct(
		::ALuint al_source,
		std::int32_t direct);
}; // EaxLtFilter

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // oal
} // ltjs


#endif // !LTJS_OAL_EAX_LT_FILTER_INCLUDED
