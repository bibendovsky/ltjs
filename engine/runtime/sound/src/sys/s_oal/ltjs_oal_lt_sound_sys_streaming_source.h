#ifndef LTJS_OAL_LT_SOUND_SYS_STREAMING_SOURCE_INCLUDED
#define LTJS_OAL_LT_SOUND_SYS_STREAMING_SOURCE_INCLUDED


#include <array>
#include <vector>

#include "al.h"

#include "bibendovsky_spul_file_stream.h"
#include "bibendovsky_spul_substream.h"

#include "iltsound.h"

#include "ltjs_audio_decoder.h"
#include "ltjs_oal_lt_sound_sys_orientation_3d.h"
#include "ltjs_oal_lt_sound_user_data.h"
#include "ltjs_oal_lt_sound_sys_vector_3d.h"


namespace ul = bibendovsky::spul;


namespace ltjs
{


enum class OalLtSoundSysStreamingSourceType
{
	none,
	panning,
	spatial,
}; // Type

enum class OalLtSoundSysStreamingSourceSpatialType
{
	none,
	source,
	listener,
}; // SpatialType

enum class OalLtSoundSysStreamingSourceStorageType
{
	none,
	internal_buffer,
	decoder,
}; // StorageType

enum class OalLtSoundSysStreamingSourceStatus
{
	none,
	stopped,
	playing,
	failed,
}; // Status

struct OalLtSoundSysStreamingSourceOpenParam
{
	bool is_file_;
	const char* file_name_;
	uint32 file_offset_;

	bool is_mapped_file_;
	const void* mapped_storage_ptr;
	AudioDecoder* mapped_decoder_;

	bool is_memory_;
	const void* memory_ptr_;
	uint32 memory_size_;
	ul::WaveFormatEx memory_wave_format_;

	sint32 playback_rate_;
}; // OpenParam


class OalLtSoundSysStreamingSource
{
public:
	static constexpr auto mix_size_ms = 20;

	using MixBuffer = std::vector<std::uint8_t>;

	static constexpr auto oal_max_buffer_count = 3;
	static constexpr auto oal_max_pans = 2;

	using Data = std::vector<std::uint8_t>;
	using OalPans = std::array<float, oal_max_pans>;
	using OalBuffers = std::array<::ALuint, oal_max_buffer_count>;


	OalLtSoundSysStreamingSource(
		const OalLtSoundSysStreamingSourceType type,
		const OalLtSoundSysStreamingSourceSpatialType spatial_type);

	OalLtSoundSysStreamingSource(
		const OalLtSoundSysStreamingSource& that) = delete;

	OalLtSoundSysStreamingSource& operator=(
		const OalLtSoundSysStreamingSource& that) = delete;

	OalLtSoundSysStreamingSource(
		OalLtSoundSysStreamingSource&& that);

	~OalLtSoundSysStreamingSource();


	::ALuint get_al_source() const noexcept;

	int& get_lt_filter_direct_mb() noexcept;

	bool is_panning() const;

	bool is_spatial() const;

	bool is_listener() const;

	bool is_mono() const;

	bool is_stereo() const;

	int get_channel_count() const;

	int get_bit_depth() const;

	int get_block_align() const;

	int get_dst_sample_rate() const;

	bool is_playing();

	bool is_stopped();

	bool is_failed();

	void pause();

	void resume();

	void stop();

	int32 get_volume() const;

	void set_volume(
		const sint32 volume);

	int32 get_pan() const;

	void set_pan(
		const sint32 pan);

	bool open(
		const OalLtSoundSysStreamingSourceOpenParam& param);

	void set_loop_block(
		const sint32 loop_begin_offset,
		const sint32 loop_end_offset,
		const bool is_enable);

	void set_loop(
		const bool is_enable);

	void set_ms_position(
		const sint32 milliseconds);

	void mix();

	std::intptr_t get_user_value(
		const uint32 index) const;

	void set_user_value(
		const uint32 index,
		const std::intptr_t value);

	sint32 get_master_3d_listener_volume() const;

	void set_master_3d_listener_volume(
		const sint32 lt_volume);

	void mute_3d_listener(
		const bool is_muted);

	void set_3d_volume(
		const sint32 lt_volume);

	sint32 get_3d_volume() const;

	void set_3d_doppler_factor(
		const float doppler_factor);

	float get_3d_doppler_factor() const;

	void set_3d_distances(
		const float min_distance,
		const float max_distance);

	float get_3d_min_distance() const;

	float get_3d_max_distance() const;

	void set_3d_position(
		const OalLtSoundSysVector3d& position);

	const OalLtSoundSysVector3d& get_3d_position() const;

	void set_3d_velocity(
		const OalLtSoundSysVector3d& velocity);

	const OalLtSoundSysVector3d& get_3d_velocity() const;

	void set_3d_direction(
		const OalLtSoundSysVector3d& direction);

	const OalLtSoundSysVector3d& get_3d_direction() const;

	void set_3d_orientation(
		const OalLtSoundSysOrientation3d& orientation);

	const OalLtSoundSysOrientation3d& get_3d_orientation() const;


private:
	static constexpr auto pan_center = 64;
	static constexpr auto default_pitch = 1.0F;


	static const float min_doppler_factor;
	static const float max_doppler_factor;
	static const float default_3d_doppler_factor;

	static const float min_min_distance;
	static const float max_min_distance;
	static const float default_3d_min_distance;

	static const float min_max_distance;
	static const float max_max_distance;
	static const float default_3d_max_distance;

		static const OalLtSoundSysVector3d default_3d_position;
	static const OalLtSoundSysVector3d default_3d_velocity;
	static const OalLtSoundSysVector3d default_3d_direction;
	static const OalLtSoundSysOrientation3d default_3d_orientation;


	template<
		int TBitDepth,
		typename TDummy = void
	>
	struct MonoToStereoSample{};

	template<
		typename TDummy
	>
	struct MonoToStereoSample<8, TDummy>;

	template<
		typename TDummy
	>
	struct MonoToStereoSample<16, TDummy>;


	OalLtSoundSysStreamingSourceType type_;
	OalLtSoundSysStreamingSourceSpatialType spatial_type_;
	OalLtSoundSysStreamingSourceStorageType storage_type_;
	OalLtSoundSysStreamingSourceStatus status_;

	int channel_count_;
	int bit_depth_;
	int block_align_;
	int sample_rate_;

	bool is_looping_;
	bool has_loop_block_;
	sint32 loop_begin_;
	sint32 loop_end_;
	sint32 volume_;
	sint32 pan_;
	float pitch_;
	Data data_;
	OalLtSoundSysUserDataArray user_data_array_;

	bool is_playing_;
	int mix_size_;
	int mix_sample_count_;
	int data_size_;
	int data_offset_;
	ul::FileStream file_stream_;
	ul::Substream file_substream_;
	AudioDecoder decoder_;
	MixBuffer mix_mono_buffer_;
	MixBuffer mix_stereo_buffer_;

	sint32 master_3d_listener_volume_;
	bool is_3d_listener_muted_;

	float doppler_factor_;
	float min_distance_;
	float max_distance_;
	OalLtSoundSysVector3d position_;
	OalLtSoundSysVector3d velocity_;
	OalLtSoundSysVector3d direction_;
	OalLtSoundSysOrientation3d orientation_;

	bool oal_are_buffers_created_;
	bool oal_is_source_created_;

	float oal_master_3d_listener_gain_;
	float oal_gain_;
	OalPans oal_pans_;

	::ALuint oal_source_;

	::ALenum oal_buffer_format_;
	OalBuffers oal_buffers_;
	int oal_queued_count_;

	int lt_filter_direct_mb_;


	static ::ALenum get_oal_buffer_format(
		const int channel_count,
		const int bit_depth);

	static bool validate_wave_format_ex(
		const ul::WaveFormatEx& wave_format);


	void create();

	void destroy();

	OalLtSoundSysStreamingSourceStatus get_status();

	void fail();

	void reset();

	void oal_reset_common();

	void oal_reset_spatial();

	void open_set_wave_format_internal(
		const ul::WaveFormatEx& wave_format);

	bool open_file_internal(
		const OalLtSoundSysStreamingSourceOpenParam& param);

	bool open_mapped_file_internal(
		const OalLtSoundSysStreamingSourceOpenParam& param);

	bool open_memory_internal(
		const OalLtSoundSysStreamingSourceOpenParam& param);

	bool open_initialize_internal(
		const OalLtSoundSysStreamingSourceOpenParam& param);

	bool open_internal(
		const OalLtSoundSysStreamingSourceOpenParam& param);

	void close_internal();

	template<
		int TBitDepth
	>
	void mix_mono_to_stereo(
		const int byte_offset);

	int mix_fill_buffer();
}; // OalLtSoundSysStreamingSource


} // ltjs


#endif // !LTJS_OAL_LT_SOUND_SYS_STREAMING_SOURCE_INCLUDED
