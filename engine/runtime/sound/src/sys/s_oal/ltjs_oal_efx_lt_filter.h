#ifndef LTJS_OAL_EFX_LT_FILTER_INCLUDED
#define LTJS_OAL_EFX_LT_FILTER_INCLUDED


#include <cstdint>

#include <array>

#include "ltjs_eax_api.h"

#include "ltjs_oal_efx_symbols.h"
#include "ltjs_oal_object.h"
#include "ltjs_oal_lt_filter.h"


namespace ltjs
{
namespace oal
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class EfxLtFilter final :
	public LtFilter
{
public:
	EfxLtFilter();


	// -------------------------------------------------------------------------
	// LtFilter

	const LtFilterInfo& get_info() const noexcept override;


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
	struct XReverbIndex
	{
		static constexpr auto density = 0;
		static constexpr auto diffusion = 1;
		static constexpr auto gain = 2;
		static constexpr auto gain_hf = 3;
		static constexpr auto gain_lf = 4;
		static constexpr auto decay_time = 5;
		static constexpr auto decay_hf_ratio = 6;
		static constexpr auto decay_lf_ratio = 7;
		static constexpr auto reflections_gain = 8;
		static constexpr auto reflections_delay = 9;
		static constexpr auto reflections_pan = 10;
		static constexpr auto late_reverb_gain = 11;
		static constexpr auto late_reverb_delay = 12;
		static constexpr auto late_reverb_pan = 13;
		static constexpr auto echo_time = 14;
		static constexpr auto echo_depth = 15;
		static constexpr auto modulation_time = 16;
		static constexpr auto modulation_depth = 17;
		static constexpr auto air_absorption_gain_hf = 18;
		static constexpr auto hf_reference = 19;
		static constexpr auto lf_reference = 20;
		static constexpr auto room_rolloff_factor = 21;
		static constexpr auto decay_hf_limit = 22;

		static constexpr auto count = 23;
	}; // XReverbIndex

	struct AlXReverbDescriptor
	{
		::ALenum param;
		float min_value;
		float max_value;
	}; // AlXReverbDescriptor

	using AlXReverbDescriptors = std::array<AlXReverbDescriptor, XReverbIndex::count>;


	bool is_std_reverb_{};
	float max_low_pass_gain_{};

	LtFilterInfo info_{};

	EfxSymbols efx_symbols_{};

	EffectSlotObject effect_slot_;
	EffectObject reverb_effect_;
	FilterObject direct_filter_;
	const AlXReverbDescriptors& al_xreverb_descriptors_;

	bool is_listener_muted_{};

	bool is_reverb_dirty_{};
	EAXREVERBPROPERTIES eax_reverb_{};


	static const AlXReverbDescriptors& get_std_reverb_descriptors();

	static const AlXReverbDescriptors& get_eax_reverb_descriptors();


	EffectObject make_reverb_effect();

	FilterObject make_direct_filter();

	const AlXReverbDescriptors& make_xreverb_descriptors();

	void make_info();


	static void ensure_lt_reverb_filter(
		const LTSOUNDFILTERDATA& lt_filter_data);

	static void ensure_al_source(
		::ALuint al_source);


	bool detect_al_softx_filter_gain_ex();

	void detect_max_low_pass_gain();

	void set_eax_reverb_defaults();


	void set_efx_reverb_density();

	void set_efx_reverb_diffusion();

	void set_efx_reverb_gain();

	void set_efx_reverb_gain_hf();

	void set_efx_reverb_gain_lf();

	void set_efx_reverb_decay_time();

	void set_efx_reverb_decay_hf_ratio();

	void set_efx_reverb_decay_lf_ratio();

	void set_efx_reverb_reflections_gain();

	void set_efx_reverb_reflections_delay();

	void set_efx_reverb_reflections_pan();

	void set_efx_reverb_late_reverb_gain();

	void set_efx_reverb_late_reverb_delay();

	void set_efx_reverb_late_reverb_pan();

	void set_efx_reverb_echo_time();

	void set_efx_reverb_echo_depth();

	void set_efx_reverb_modulation_time();

	void set_efx_reverb_modulation_depth();

	void set_efx_reverb_air_absorption_gain_hf();

	void set_efx_reverb_hf_reference();

	void set_efx_reverb_lf_reference();

	void set_efx_reverb_room_rolloff_factor();

	void set_efx_reverb_decay_hf_limit();

	void set_efx_reverb_all();


	void set_efx_slot_effect_gain(
		float gain);

	void mute_efx_slot_effect();

	void unmute_efx_slot_effect();


	void set_efx_effect();


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

	void set_source_direct(
		::ALuint al_source,
		int direct);
}; // EfxLtFilter

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // oal
} // ltjs


#endif // !LTJS_OAL_EFX_LT_FILTER_INCLUDED
