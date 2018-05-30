#include "bdefs.h"


#ifdef _DEBUG
//#define LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_STRUCTURE
//#define LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_WAVES
#endif // _DEBUG


#include "ltjs_dmusic_segment.h"
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <array>
#include <fstream>
#include <random>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include "bibendovsky_spul_ascii_utils.h"
#include "bibendovsky_spul_encoding_utils.h"
#include "bibendovsky_spul_endian.h"
#include "bibendovsky_spul_enum_flags.h"
#include "bibendovsky_spul_file_stream.h"
#include "bibendovsky_spul_four_cc.h"
#include "bibendovsky_spul_memory_stream.h"
#include "bibendovsky_spul_path_utils.h"
#include "bibendovsky_spul_riff_four_ccs.h"
#include "bibendovsky_spul_riff_reader.h"
#include "bibendovsky_spul_scope_guard.h"
#include "bibendovsky_spul_un_value.h"
#include "bibendovsky_spul_uuid.h"
#include "bibendovsky_spul_wave_format.h"
#include "bibendovsky_spul_wave_format_utils.h"
#include "bibendovsky_spul_wave_four_ccs.h"
#include "client_filemgr.h"
#include "ltjs_audio_decoder.h"


namespace ltjs
{


namespace ul = bibendovsky::spul;


namespace
{

static IClientFileMgr* client_file_manager;
define_holder(IClientFileMgr, client_file_manager);

} // namespace


class DMusicSegment::Impl final
{
public:
	Impl()
		:
		is_open_{},
		error_message_{},
		file_image_{},
		memory_stream_{},
		riff_reader_{},
		wave_cache_{},
		sample_rate_{},
		segment_{}
#ifdef LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_STRUCTURE
		,
		debug_filebuf_{}
#endif // LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_STRUCTURE
	{
	}

	Impl(
		const Impl& that) = delete;

	Impl& operator=(
		const Impl& that) = delete;

	Impl(
		Impl&& that)
		:
		is_open_{std::move(that.is_open_)},
		error_message_{std::move(that.error_message_)},
		file_image_{std::move(that.file_image_)},
		memory_stream_{std::move(that.memory_stream_)},
		riff_reader_{std::move(that.riff_reader_)},
		wave_cache_{std::move(that.wave_cache_)},
		sample_rate_{std::move(that.sample_rate_)},
		segment_{std::move(that.segment_)}
#ifdef LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_STRUCTURE
		,
		debug_filebuf_{std::move(that.debug_filebuf_)}
#endif // LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_STRUCTURE
	{
	}

	~Impl()
	{
		close_internal();
	}


	bool api_open(
		const std::string& file_name,
		const int sample_rate)
	{
		error_message_.clear();

		close_internal();

		if (sample_rate <= 0)
		{
			error_message_ = "Invalid sample rate.";
			return false;
		}

		sample_rate_ = sample_rate;

		if (!open_internal(file_name))
		{
			close_internal();
			return false;
		}

		return true;
	}

	void api_close()
	{
		close_internal();
	}

	bool api_rewind()
	{
		if (!is_open_)
		{
			error_message_ = "Not open.";
			return false;
		}

		return rewind();
	}

	int api_mix(
		const int src_decode_size,
		std::int16_t* dst_decode_buffer,
		float* dst_mix_buffer)
	{
		if (!is_open_)
		{
			error_message_ = "Not open.";
			return -1;
		}

		if (src_decode_size < 0)
		{
			error_message_ = "Invalid a source decode size.";
			return -1;
		}

		if (!dst_decode_buffer)
		{
			error_message_ = "No decode buffer.";
			return -1;
		}

		if (!dst_mix_buffer)
		{
			error_message_ = "No mix buffer.";
			return -1;
		}

		return mix(src_decode_size, dst_decode_buffer, dst_mix_buffer);
	}

	int api_get_length() const
	{
		return segment_.length_;
	}

	bool api_is_finished() const
	{
		return segment_.is_finished_;
	}

	bool api_is_silence() const
	{
		return segment_.waves_.items_.empty();
	}

	const std::string& api_get_error_message() const
	{
		return error_message_;
	}


private:
	static constexpr auto max_lt_wave_size = std::uint32_t{8 * 1'024 * 1'024};
	static constexpr auto all_flags_on = std::uint32_t{0xFFFFFFFF};
	static constexpr auto channel_count = 2;
	static constexpr auto bit_depth = 16;
	static constexpr auto default_qbpm = 120; // quarter beats per minute
	static constexpr auto units_per_quarter_beat = 768;
	static constexpr auto default_variation_mask = std::uint32_t{1};


	using Buffer = std::vector<std::uint8_t>;
	using WaveCache = std::vector<ul::UnValue<std::uint8_t>>;

	using IoMusicTime8 = std::int32_t;
	using IoReferenceTime8 = std::int64_t;


	struct ValidFlags8 :
		ul::EnumFlagsT<std::uint32_t>
	{
		ValidFlags8(const Value flags = none)
			:
			EnumFlagsT<std::uint32_t>{flags}
		{
		}

		enum : Value
		{
			file_name = 0B0001'0000,
		}; // Value
	}; // Flags


	struct IoSegmentHeader8
	{
		static constexpr auto class_size = 40;


		std::uint32_t repeat_count_;
		IoMusicTime8 mt_length_;
		IoMusicTime8 mt_play_start_;
		IoMusicTime8 mt_loop_start_;
		IoMusicTime8 mt_loop_end_;
		std::uint32_t resolution_;
		IoReferenceTime8 rt_length_;
		std::uint32_t flags_;
		std::uint32_t reserved_;


		IoSegmentHeader8()
			:
			repeat_count_{},
			mt_length_{},
			mt_play_start_{},
			mt_loop_start_{},
			mt_loop_end_{},
			resolution_{},
			rt_length_{},
			flags_{},
			reserved_{}
		{
			static_assert(class_size == sizeof(IoSegmentHeader8), "Invalid class size.");
		}

		bool read(
			ul::StreamPtr stream_ptr)
		{
			if (stream_ptr->read(this, class_size) != class_size)
			{
				return false;
			}

			if (!ul::Endian::is_little())
			{
				ul::Endian::little_i(repeat_count_);
				ul::Endian::little_i(mt_length_);
				ul::Endian::little_i(mt_play_start_);
				ul::Endian::little_i(mt_loop_start_);
				ul::Endian::little_i(mt_loop_end_);
				ul::Endian::little_i(resolution_);
				ul::Endian::little_i(rt_length_);
				ul::Endian::little_i(flags_);
				ul::Endian::little_i(reserved_);
			}

			return true;
		}

		bool validate(
			std::string& error_message) const
		{
			if (repeat_count_ != 0)
			{
				error_message = "Expected zero repeat count.";
				return false;
			}

			if (mt_length_ <= 0)
			{
				error_message = "Expected positive music length.";
				return false;
			}

			if (mt_play_start_ != 0)
			{
				error_message = "Expected zero music play start.";
				return false;
			}

			if (mt_loop_start_ != 0)
			{
				error_message = "Expected zero music loop start.";
				return false;
			}

			if (mt_loop_end_ != 0)
			{
				error_message = "Expected zero music loop end.";
				return false;
			}

			if (resolution_ != 0)
			{
				error_message = "Expected zero resolution.";
				return false;
			}

			if (rt_length_ != 0)
			{
				error_message = "Expected zero reference length.";
				return false;
			}

			if (flags_ != 0)
			{
				error_message = "Expected zero flags.";
				return false;
			}

			return true;
		}
	}; // IoSegmentHeader8

	enum class IoTrackType8
	{
		none,
		tempo,
		time_signature,
		sequence,
		wave,
	}; // IoTrackType8

	struct IoTempoItem8
	{
		static constexpr auto class_size = 16;


		IoMusicTime8 time_;
		double tempo_;


		IoTempoItem8()
			:
			time_{},
			tempo_{}
		{
			static_assert(class_size == sizeof(IoTempoItem8), "Invalid class size.");
		}

		bool read(
			ul::StreamPtr stream_ptr)
		{
			if (!stream_ptr || !stream_ptr->is_readable())
			{
				return false;
			}

			if (stream_ptr->read(this, class_size) != class_size)
			{
				return false;
			}

			if (!ul::Endian::is_little())
			{
				ul::Endian::little_i(time_);
				ul::Endian::little_i(tempo_);
			}

			return true;
		}

		bool validate(
			std::string& error_message) const
		{
			if (time_ < 0)
			{
				error_message = "Negative music time.";
				return false;
			}

			if (tempo_ < 1.0)
			{
				error_message = "Invalid tempo value.";
				return false;
			}

			auto tempo_int = 0.0;

			if (std::modf(tempo_, &tempo_int) != 0.0)
			{
				error_message = "Expected integer tempo value.";
				return false;
			}

			return true;
		}
	}; // IoTempoItem8

	struct IoTimeSignatureItem8
	{
		static constexpr auto class_size = 8;


		IoMusicTime8 time_;
		std::uint8_t beats_per_measure_;
		std::uint8_t beat_;
		std::uint16_t grids_per_beat_;


		IoTimeSignatureItem8()
			:
			time_{},
			beats_per_measure_{},
			beat_{},
			grids_per_beat_{}
		{
			static_assert(class_size == sizeof(IoTimeSignatureItem8), "Invalid class size.");
		}

		bool read(
			ul::StreamPtr stream_ptr)
		{
			if (!stream_ptr || !stream_ptr->is_readable())
			{
				return false;
			}

			if (stream_ptr->read(this, class_size) != class_size)
			{
				return false;
			}

			if (!ul::Endian::is_little())
			{
				ul::Endian::little_i(time_);
				ul::Endian::little_i(beats_per_measure_);
				ul::Endian::little_i(beat_);
				ul::Endian::little_i(grids_per_beat_);
			}

			return true;
		}

		bool validate(
			std::string& error_message) const
		{
			if (time_ < 0)
			{
				error_message = "Negative music time.";
				return false;
			}

			if (beats_per_measure_ == 0)
			{
				error_message = "Zero beats per measure.";
				return false;
			}

			switch (beat_)
			{
			case 0: // 1 / 256
			case 1:
			case 2:
			case 4:
			case 8:
			case 16:
			case 32:
			case 64:
			case 128:
				break;

			default:
				error_message = "Invalid beat value.";
				return false;
			}

			if (grids_per_beat_ == 0)
			{
				error_message = "Zero grids per beat.";
				return false;
			}

			return true;
		}

	}; // IoTimeSignatureItem8

	struct IoSequenceItem8
	{
		static constexpr auto class_size = 20;


		IoMusicTime8 mt_time_;
		IoMusicTime8 mt_duration_;
		std::uint32_t channel_;
		std::int16_t offset_;
		std::uint8_t status_;
		std::uint8_t byte_1_;
		std::uint8_t byte_2_;


		IoSequenceItem8()
			:
			mt_time_{},
			mt_duration_{},
			channel_{},
			offset_{},
			status_{},
			byte_1_{},
			byte_2_{}
		{
			static_assert(class_size == sizeof(IoSequenceItem8), "Invalid class size.");
		}

		bool read(
			ul::StreamPtr stream_ptr)
		{
			if (!stream_ptr || !stream_ptr->is_readable())
			{
				return false;
			}

			if (stream_ptr->read(&mt_time_, 4) != 4)
			{
				return false;
			}

			if (!ul::Endian::is_little())
			{
				ul::Endian::little_i(mt_time_);
			}

			mt_duration_ = {};
			channel_ = {};
			offset_ = {};
			status_ = {};
			byte_1_ = {};
			byte_2_ = {};

			return true;
		}

		bool validate(
			std::string& error_message) const
		{
			if (mt_time_ < 0)
			{
				error_message = "Negative music time.";
				return false;
			}

			if (mt_duration_ != 0)
			{
				error_message = "Expected zero music duration.";
				return false;
			}

			if (channel_ != 0)
			{
				error_message = "Expected zero channel.";
				return false;
			}

			if (offset_ != 0)
			{
				error_message = "Expected zero offset.";
				return false;
			}

			if (status_ != 0)
			{
				error_message = "Expected zero MIDI status.";
				return false;
			}

			if (byte_1_ != 0)
			{
				error_message = "Expected zero first byte of the MIDI data.";
				return false;
			}

			if (byte_2_ != 0)
			{
				error_message = "Expected zero second byte of the MIDI data.";
				return false;
			}

			return true;
		}
	}; // IoSequenceItem8

	using IoSequenceItems8 = std::vector<IoSequenceItem8>;


	struct IoCurveItem8
	{
		static constexpr auto class_size = 32;

		enum class Type :
			std::uint8_t
		{
			pitch_bend = 0x03,
			control_change = 0x04,
			mono_aftertouch = 0x05,
			poly_aftertouch = 0x06,
			rpn = 0x07,
			nrpn = 0x08,
		}; // Type

		enum class Shape :
			std::uint8_t
		{
			linear = 0,
			instant = 1,
			exponential = 2,
			logarithmic = 3,
			sine = 4,
		}; // Type

		struct Flags :
			ul::EnumFlagsT<std::uint8_t>
		{
			Flags(const Value flags = none)
				:
				EnumFlagsT<std::uint8_t>{flags}
			{
			}

			enum : Value
			{
				reset = 0B0000'0001,
				start_from_current = 0B0000'0010,
			}; // Value
		}; // Flags


		IoMusicTime8 mt_start_;
		IoMusicTime8 mt_duration_;
		IoMusicTime8 mt_reset_duration_;
		std::uint32_t channel_;
		std::int16_t offset_;
		std::int16_t start_value_;
		std::int16_t end_value_;
		std::int16_t reset_value_;
		Type type_;
		Shape curve_shape_;
		std::uint8_t cc_data_;
		Flags flags_;
		std::uint16_t param_type_;
		std::uint16_t merge_index_;


		IoCurveItem8()
			:
			mt_start_{},
			mt_duration_{},
			mt_reset_duration_{},
			channel_{},
			offset_{},
			start_value_{},
			end_value_{},
			reset_value_{},
			type_{},
			curve_shape_{},
			cc_data_{},
			flags_{},
			param_type_{},
			merge_index_{}
		{
			static_assert(class_size == sizeof(IoCurveItem8), "Invalid class size.");
		}


		bool read(
			ul::StreamPtr stream_ptr)
		{
			if (!stream_ptr || !stream_ptr->is_readable())
			{
				return false;
			}

			if (stream_ptr->read(this, class_size) != class_size)
			{
				return false;
			}

			if (!ul::Endian::is_little())
			{
				ul::Endian::little_i(mt_start_);
				ul::Endian::little_i(mt_duration_);
				ul::Endian::little_i(mt_reset_duration_);
				ul::Endian::little_i(channel_);
				ul::Endian::little_i(offset_);
				ul::Endian::little_i(start_value_);
				ul::Endian::little_i(end_value_);
				ul::Endian::little_i(reset_value_);
				ul::Endian::little_i(type_);
				ul::Endian::little_i(curve_shape_);
				ul::Endian::little_i(cc_data_);
				ul::Endian::little_i(flags_);
				ul::Endian::little_i(param_type_);
				ul::Endian::little_i(merge_index_);
			}

			return true;
		}

		bool validate(
			std::string& error_message) const
		{
			if (mt_start_ != 0)
			{
				error_message = "Expected zero music time.";
				return false;
			}

			if (mt_duration_ <= 0)
			{
				error_message = "Expected positive music duration.";
				return false;
			}

			if (mt_reset_duration_ < 0)
			{
				error_message = "Negative music reset duration.";
				return false;
			}

			// Skip channel.

			if (offset_ != 0)
			{
				error_message = "Expected zero music offset.";
				return false;
			}

			if (start_value_ < 0)
			{
				error_message = "Negative start value.";
				return false;
			}

			if (end_value_ < 0)
			{
				error_message = "Negative end value.";
				return false;
			}

			if (reset_value_ < 0)
			{
				error_message = "Negative reset value.";
				return false;
			}

			if (type_ != Type::control_change)
			{
				error_message = "Expected continuous controller curve type.";
				return false;
			}

			if (!(curve_shape_ == Shape::instant ||
				curve_shape_ == Shape::sine ||
				curve_shape_ == Shape::exponential))
			{
				error_message = "Unsupported curve shape.";
				return false;
			}

			// Skip CC data.

			if (!(flags_ == Flags::none || flags_ == Flags::reset || flags_ == Flags::start_from_current))
			{
				error_message = "Unsupported flags.";
				return false;
			}

			if (param_type_ != 0)
			{
				error_message = "Expected zero param type.";
				return false;
			}

			if (merge_index_ != 0)
			{
				error_message = "Expected zero merge index.";
				return false;
			}

			return true;
		}
	}; // IoCurveItem8

	using IoCurveItems8 = std::vector<IoCurveItem8>;


	struct IoWaveTrackHeader8
	{
		static constexpr auto class_size = 8;


		struct Flags :
			ul::EnumFlagsT<std::uint32_t>
		{
			Flags(const Value flags = none)
				:
				EnumFlagsT<std::uint32_t>{flags}
			{
			}

			enum : Value
			{
				persist_control = 0B0010,
			}; // Value
		}; // Flags


		std::int32_t volume_;
		Flags flags_;


		IoWaveTrackHeader8()
			:
			volume_{},
			flags_{}
		{
			static_assert(class_size == sizeof(IoWaveTrackHeader8), "Invalid class size.");
		}

		bool read(
			ul::StreamPtr stream_ptr)
		{
			if (!stream_ptr || !stream_ptr->is_readable())
			{
				return false;
			}

			if (stream_ptr->read(this, class_size) != class_size)
			{
				return false;
			}

			if (!ul::Endian::is_little())
			{
				ul::Endian::little_i(volume_);
				ul::Endian::little_i(flags_);
			}

			return true;
		}

		bool validate(
			std::string& error_message) const
		{
			if (volume_ != 0)
			{
				error_message = "Expected zero volume.";
				return false;
			}

			if (flags_ != Flags::persist_control)
			{
				error_message = "Expected persistent variation control info.";
				return false;
			}

			return true;
		}
	}; // IoWaveTrackHeader8

	struct IoWavePartHeader8
	{
		static constexpr auto class_size = 24;


		enum class VariationType :
			std::uint32_t
		{
			random = 1,
			no_repeat = 3,
		}; // VariationType


		std::int32_t volume_;
		std::uint32_t variations_;
		std::uint32_t channel_;
		std::uint32_t lock_to_part_;
		VariationType flags_;
		std::uint32_t index_;


		IoWavePartHeader8()
			:
			volume_{},
			variations_{},
			channel_{},
			lock_to_part_{},
			flags_{},
			index_{}
		{
			static_assert(class_size == sizeof(IoWavePartHeader8), "Invalid class size.");
		}

		bool read(
			ul::StreamPtr stream_ptr)
		{
			if (!stream_ptr || !stream_ptr->is_readable())
			{
				return false;
			}

			if (stream_ptr->read(this, class_size) != class_size)
			{
				return false;
			}

			if (!ul::Endian::is_little())
			{
				ul::Endian::little_i(volume_);
				ul::Endian::little_i(variations_);
				ul::Endian::little_i(channel_);
				ul::Endian::little_i(lock_to_part_);
				ul::Endian::little_i(flags_);
				ul::Endian::little_i(index_);
			}

			return true;
		}

		bool validate(
			std::string& error_message) const
		{
			// In theory this must be negative. On practice it can be zero.
			if (volume_ != 0)
			{
				error_message = "Expected zero volume.";
				return false;
			}

			if (variations_ == 0)
			{
				error_message = "Expected at least one variation.";
				return false;
			}

			// Skip channel.

			if (lock_to_part_ != 0)
			{
				error_message = "Expected zero lock to part.";
				return false;
			}

			if (!(flags_ == VariationType::random || flags_ == VariationType::no_repeat))
			{
				error_message = "Unsupported variation type.";
				return false;
			}

			if (index_ != 0)
			{
				error_message = "Expected zero index.";
				return false;
			}

			return true;
		}
	}; // IoWavePartHeader8

	struct IoWaveItemHeader8
	{
		static constexpr auto class_size = 64;


		std::int32_t volume_;
		std::int32_t pitch_;
		std::uint32_t variations_;
		IoReferenceTime8 rt_time_;
		IoReferenceTime8 rt_start_offset_;
		IoReferenceTime8 rt_reserved_;
		IoReferenceTime8 rt_duration_;
		IoMusicTime8 mt_logical_time;
		std::uint32_t loop_start_;
		std::uint32_t loop_end_;
		std::uint32_t flags_;


		IoWaveItemHeader8()
		{
			static_assert(class_size == sizeof(IoWaveItemHeader8), "Invalid class size.");
		}

		bool read(
			ul::StreamPtr stream_ptr)
		{
			if (!stream_ptr || !stream_ptr->is_readable())
			{
				return false;
			}

			if (stream_ptr->read(this, class_size) != class_size)
			{
				return false;
			}

			if (!ul::Endian::is_little())
			{
				ul::Endian::little_i(volume_);
				ul::Endian::little_i(pitch_);
				ul::Endian::little_i(variations_);
				ul::Endian::little_i(rt_time_);
				ul::Endian::little_i(rt_start_offset_);
				ul::Endian::little_i(rt_reserved_);
				ul::Endian::little_i(rt_duration_);
				ul::Endian::little_i(mt_logical_time);
				ul::Endian::little_i(loop_start_);
				ul::Endian::little_i(loop_end_);
				ul::Endian::little_i(flags_);
			}

			return true;
		}

		bool validate(
			std::string& error_message) const
		{
			// In theory this must be negative. On practice it can be zero.
			if (volume_ != 0)
			{
				error_message = "Expected zero volume.";
				return false;
			}

			if (pitch_ != 0)
			{
				error_message = "Expected zero pitch.";
				return false;
			}

			if (variations_ == 0)
			{
				error_message = "Expected at least one variation.";
				return false;
			}

			if (rt_time_ < 0)
			{
				error_message = "Negative reference time.";
				return false;
			}

			if (rt_start_offset_ != 0)
			{
				error_message = "Expected zero reference start offset.";
				return false;
			}

			if (rt_reserved_ != 0)
			{
				error_message = "Expected zero reserved reference time.";
				return false;
			}

			if (rt_duration_ < 0)
			{
				error_message = "Negative reference duration.";
				return false;
			}

			if (mt_logical_time < 0)
			{
				error_message = "Negative logical music time.";
				return false;
			}

			if (loop_start_ != 0)
			{
				error_message = "Expected zero loop start.";
				return false;
			}

			if (loop_end_ != 0)
			{
				error_message = "Expected zero loop end.";
				return false;
			}

			if (flags_ != 0)
			{
				error_message = "Expected zero flags.";
				return false;
			}

			return true;
		}
	}; // IoWaveItemHeader8

	struct IoReference8
	{
		static constexpr auto class_size = 20;


		ul::Uuid clsid_;
		ValidFlags8 valid_data_;


		IoReference8()
		{
			static_assert(class_size == sizeof(IoReference8), "Invalid class size.");
		}

		bool read(
			ul::StreamPtr stream_ptr)
		{
			if (!stream_ptr || !stream_ptr->is_readable())
			{
				return false;
			}

			if (stream_ptr->read(this, class_size) != class_size)
			{
				return false;
			}

			clsid_.endian(ul::Uuid::EndianType::little_mixed);

			if (!ul::Endian::is_little())
			{
				ul::Endian::little_i(valid_data_);
			}

			return true;
		}

		bool validate(
			std::string& error_message) const
		{
			if (clsid_ != clsid_sound_wave)
			{
				error_message = "Expected DirectSound wave CLSID.";
				return false;
			}

			if ((valid_data_ & ValidFlags8::file_name) == 0)
			{
				error_message = "Expected file name flag.";
				return false;
			}

			return true;
		}
	}; // IoReference8

	struct IoTrackHeader8
	{
		static constexpr auto class_size = 32;


		ul::Uuid guid_;
		std::uint32_t position_;
		std::uint32_t group_;
		ul::FourCc chunk_id_;
		ul::FourCc list_type_;


		IoTrackHeader8()
		{
			static_assert(sizeof(IoTrackHeader8) == class_size, "Invalid class size.");
		}


		bool read(
			ul::Stream& stream)
		{
			if (stream.read(this, class_size) != class_size)
			{
				return false;
			}

			guid_.endian(ul::Uuid::EndianType::little_mixed);

			if (!ul::Endian::is_little())
			{
				ul::Endian::little_i(position_);
				ul::Endian::little_i(group_);
				ul::Endian::little_i(chunk_id_);
				ul::Endian::little_i(list_type_);
			}

			return true;
		}

		IoTrackType8 get_type() const
		{
			if (guid_ == clsid_tempo_track)
			{
				return IoTrackType8::tempo;
			}
			else if (guid_ == clsid_time_sig_track)
			{
				return IoTrackType8::time_signature;
			}
			else if (guid_ == clsid_sequence_track)
			{
				return IoTrackType8::sequence;
			}
			else if (guid_ == clsid_wave_track)
			{
				return IoTrackType8::wave;
			}
			else
			{
				return IoTrackType8::none;
			}
		}

		bool validate(
			std::string& error_message) const
		{
			if (!(guid_ == clsid_tempo_track ||
				guid_ == clsid_time_sig_track ||
				guid_ == clsid_sequence_track ||
				guid_ == clsid_wave_track))
			{
				error_message = "Unsupported track type.";
				return false;
			}

			if (position_ != 0)
			{
				error_message = "Expected zero position.";
				return false;
			}

			if (!(group_ == 1 || group_ == all_flags_on))
			{
				error_message = "Unsupported group value.";
				return false;
			}

			if (chunk_id_ == 0 && list_type_ == 0)
			{
				error_message = "Expected chunk id or list type.";
				return false;
			}

			return true;
		}
	}; // IoTrackHeader8


	using IoTempoItems = std::vector<IoTempoItem8>;
	using IoTimeSignatureItems = std::vector<IoTimeSignatureItem8>;

	struct IoSequenceItem
	{
		IoSequenceItems8 events_;
		IoCurveItems8 curves_;
	}; // IoSequenceItem

	using IoSequenceItems = std::vector<IoSequenceItem>;

	struct IoReference
	{
		IoReference8 header_;
		std::string file_name_u8_;
		ul::MemoryStream file_stream_;
		ltjs::AudioDecoder audio_decoder_;
	}; // IoReference

	using IoReferences = std::vector<IoReference>;

	struct IoWaveItem
	{
		IoWaveItemHeader8 header_;
		IoReferences references_;
	}; // IoWaveItem

	using IoWaveItems = std::vector<IoWaveItem>;

	struct IoWavePart
	{
		IoWavePartHeader8 header_;
		IoWaveItems items_;

	}; // IoWavePart

	using IoWaveParts = std::vector<IoWavePart>;

	struct IoWaveTrack
	{
		IoWaveTrackHeader8 header_;
		IoWaveParts parts_;
	}; // IoWaveTrack

	using IoWaveTracks = std::vector<IoWaveTrack>;

	struct IoTrack
	{
		IoTrackHeader8 header_;
		IoTempoItems tempos_;
		IoTimeSignatureItems times_;
		IoSequenceItems sequences_;
		IoWaveTracks waves_;
	}; // IoTrack

	using IoTracks = std::vector<IoTrack>;


	struct WaveItem
	{
		int length_; // bytes
		std::uint32_t variation_masks_;

		ul::MemoryStream stream_;
		ltjs::AudioDecoder decoder_;
	}; // WaveItem

	using WaveItems = std::vector<WaveItem>;

	struct Object
	{
		bool is_started_;
		bool is_finished_;
		int mix_offset_; // bytes
		int decode_offset_; // bytes
		WaveItem* item_ptr_;
	}; // Object

	using Objects = std::vector<Object>;

	using VariationMaskList = std::vector<std::uint32_t>;

	struct Waves
	{
		std::uint32_t channel_;
		std::uint32_t current_variation_mask_;
		std::uint32_t variation_count_;
		VariationMaskList variation_mask_list_;
		int last_variation_index_;
		bool is_variation_no_repeat_;

		WaveItems items_;
		Objects objects_template_;
		Objects active_objects_;
		Objects inactive_objects_;
	}; // Waves

	struct Segment
	{
		bool is_finished_;

		int mix_offset_; // bytes
		int length_; // bytes

		Waves waves_;
	}; // Segment


	// Tempo track CLSID.
	static const ul::Uuid clsid_tempo_track;

	// Time signature track CLSID.
	static const ul::Uuid clsid_time_sig_track;

	// Sequence track CLSID.
	static const ul::Uuid clsid_sequence_track;

	// Wave track CLSID.
	static const ul::Uuid clsid_wave_track;

	// Wave CLSID.
	static const ul::Uuid clsid_sound_wave;


	bool is_open_;
	std::string error_message_;
	std::string working_dir_;
	Buffer file_image_;
	ul::MemoryStream memory_stream_;
	ul::RiffReader riff_reader_;
	WaveCache wave_cache_;
	int sample_rate_;
	Segment segment_;


	bool read_io_segment_header(
		IoSegmentHeader8& io_segment_header)
	{
		if (!riff_reader_.find_and_descend(ul::FourCc{"segh"}))
		{
			error_message_ = "No header.";
			return false;
		}

		auto io_header_chunk = riff_reader_.get_current_chunk();

		if (!io_segment_header.read(&io_header_chunk.data_stream_))
		{
			error_message_ = "Failed to read a header.";
			return false;
		}

		if (!io_segment_header.validate(error_message_))
		{
			error_message_ = "Failed to validate a segment header: " + error_message_;
			return false;
		}

		if (!riff_reader_.ascend())
		{
			error_message_ = "RIFF error.";
			return false;
		}

		return true;
	}

	//
	// "tetr" - tempo track chunk
	//
	bool read_tempo_track(
		IoTrack& track)
	{
		if (!riff_reader_.find_and_descend(ul::FourCc{"tetr"}))
		{
			error_message_ = "No tempo track item.";
			return false;
		}

		auto chunk = riff_reader_.get_current_chunk();

		auto item_size = std::uint32_t{};

		if (chunk.data_stream_.read(&item_size, 4) != 4)
		{
			error_message_ = "Failed to read a size of a tempo item.";
			return false;
		}

		ul::Endian::little_i(item_size);

		if (item_size < IoTempoItem8::class_size)
		{
			error_message_ = "Invalid size of a tempo item.";
			return false;
		}

		const auto item_count = static_cast<int>((chunk.size_ - 4) / item_size);
		const auto item_remain_size = item_size - IoTempoItem8::class_size;

		for (auto i = 0; i < item_count; ++i)
		{
			track.tempos_.emplace_back();
			auto& item = track.tempos_.back();

			if (!item.read(&chunk.data_stream_))
			{
				error_message_ = "Failed to read a tempo track item.";
				return false;
			}

			if (!item.validate(error_message_))
			{
				error_message_ = "Failed to validate a tempo track item: " + error_message_;
				return false;
			}

			if (item_remain_size > 0)
			{
				if (chunk.data_stream_.skip(item_remain_size) < 0)
				{
					error_message_ = "Seek error.";
					return false;
				}
			}
		}

		// Ascend "tetr".
		//
		if (!riff_reader_.ascend())
		{
			error_message_ = "RIFF error.";
			return false;
		}

		return true;
	}

	//
	// "TIMS" (list) - time signature track list
	//     "tims" - time signature
	//
	bool read_time_signature_track(
		IoTrack& track)
	{
		if (!riff_reader_.find_and_descend(ul::RiffFourCcs::list, ul::FourCc{"TIMS"}))
		{
			error_message_ = "No time signature track list.";
			return false;
		}

		if (riff_reader_.find_and_descend(ul::FourCc{"tims"}))
		{
			auto chunk = riff_reader_.get_current_chunk();

			auto item_size = std::uint32_t{};

			if (chunk.data_stream_.read(&item_size, 4) != 4)
			{
				error_message_ = "Failed to read size of a time signature item.";
				return false;
			}

			ul::Endian::little_i(item_size);

			if (item_size < IoTimeSignatureItem8::class_size)
			{
				error_message_ = "Invalid size of a time signature item.";
				return false;
			}

			const auto item_count = static_cast<int>((chunk.size_ - 4) / item_size);
			const auto item_remain_size = item_size - IoTimeSignatureItem8::class_size;

			for (auto i = 0; i < item_count; ++i)
			{
				track.times_.emplace_back();
				auto& item = track.times_.back();

				if (!item.read(&chunk.data_stream_))
				{
					error_message_ = "Failed to read a time signature item.";
					return false;
				}

				if (!item.validate(error_message_))
				{
					error_message_ = "Failed to validate a time signature item: " + error_message_;
					return false;
				}

				if (item_remain_size > 0)
				{
					if (chunk.data_stream_.skip(item_remain_size) < 0)
					{
						error_message_ = "Seek error.";
						return false;
					}
				}
			}

			// Ascend "tims".
			//
			if (!riff_reader_.ascend())
			{
				error_message_ = "RIFF error.";
				return false;
			}
		}

		// Ascend "TIMS".
		//
		if (!riff_reader_.ascend())
		{
			error_message_ = "RIFF error.";
			return false;
		}

		return true;
	}

	//
	// "seqt" - sequence track
	//     "evtl" - sequence item
	//     "curl" - curve item
	//
	bool read_sequence_track(
		IoTrack& track)
	{
		// Sequence track.
		//
		if (!riff_reader_.find_and_descend(ul::FourCc{"seqt"}))
		{
			error_message_ = "No sequence track.";
			return false;
		}

		auto seqt_chunk = riff_reader_.get_current_chunk();

		if (seqt_chunk.is_empty())
		{
			return true;
		}

		track.sequences_.emplace_back();
		auto& sequence = track.sequences_.back();

		// Sequence item.
		//
		if (!riff_reader_.find_and_descend(ul::FourCc{"evtl"}))
		{
			error_message_ = "No sequence item.";
			return false;
		}

		auto evtl_chunk = riff_reader_.get_current_chunk();

		auto evtl_item_size = std::uint32_t{};

		if (evtl_chunk.data_stream_.read(&evtl_item_size, 4) != 4)
		{
			error_message_ = "Failed to read a size of a sequence item.";
			return false;
		}

		ul::Endian::little_i(evtl_item_size);

		if (evtl_item_size < IoSequenceItem8::class_size)
		{
			error_message_ = "Invalid size of a sequence item.";
			return false;
		}

		const auto evtl_item_count = static_cast<int>((evtl_chunk.size_ - 4) / evtl_item_size);
		const auto evtl_item_remain_size = evtl_item_size - IoSequenceItem8::class_size;

		for (auto i = 0; i < evtl_item_count; ++i)
		{
			sequence.events_.emplace_back();
			auto& event = sequence.events_.back();

			if (!event.read(&evtl_chunk.data_stream_))
			{
				error_message_ = "Failed to read a sequence item.";
				return false;
			}

			if (!event.validate(error_message_))
			{
				error_message_ = "Failed to validate a sequence item: " + error_message_;
				return false;
			}

			if (evtl_item_remain_size > 0)
			{
				if (evtl_chunk.data_stream_.skip(evtl_item_remain_size) < 0)
				{
					error_message_ = "Seek error.";
					return false;
				}
			}
		}

		// Ascend "evtl".
		//
		if (!riff_reader_.ascend())
		{
			error_message_ = "RIFF error.";
			return false;
		}


		// Curve item.
		//
		if (!riff_reader_.find_and_descend(ul::FourCc{"curl"}))
		{
			error_message_ = "No curve item.";
			return false;
		}

		auto curl_chunk = riff_reader_.get_current_chunk();

		auto curl_item_size = std::uint32_t{};

		if (curl_chunk.data_stream_.read(&curl_item_size, 4) != 4)
		{
			error_message_ = "Failed to read a size of a curve item.";
			return false;
		}

		ul::Endian::little_i(curl_item_size);

		if (curl_item_size < IoCurveItem8::class_size)
		{
			error_message_ = "Invalid size of a curve item.";
			return false;
		}

		const auto curl_item_count = static_cast<int>((curl_chunk.size_ - 4) / curl_item_size);
		const auto curl_item_remain_size = curl_item_size - IoCurveItem8::class_size;

		for (auto i = 0; i < curl_item_count; ++i)
		{
			sequence.curves_.emplace_back();
			auto& curve = sequence.curves_.back();

			if (!curve.read(&curl_chunk.data_stream_))
			{
				error_message_ = "Failed to read a curve item.";
				return false;
			}

			if (!curve.validate(error_message_))
			{
				error_message_ = "Failed to validate a curve item: " + error_message_;
				return false;
			}

			if (curl_item_remain_size > 0)
			{
				if (curl_chunk.data_stream_.skip(curl_item_remain_size) < 0)
				{
					error_message_ = "Seek error.";
					return false;
				}
			}
		}

		// Ascend "curl".
		//
		if (!riff_reader_.ascend())
		{
			error_message_ = "RIFF error.";
			return false;
		}


		// Ascend "seqt".
		//
		if (!riff_reader_.ascend())
		{
			error_message_ = "RIFF error.";
			return false;
		}

		return true;
	}

	bool validate_wave_track(
		const IoTrack& track)
	{
		if (track.waves_.empty())
		{
			error_message_ = "Expected at least one wave.";
			return false;
		}

		for (const auto& wave : track.waves_)
		{
			if (wave.parts_.size() != 1)
			{
				error_message_ = "Expected one wave part.";
				return false;
			}

			const auto& part = wave.parts_.front();

			for (const auto& item : part.items_)
			{
				if (item.references_.size() != 1)
				{
					error_message_ = "Expected one reference.";
					return false;
				}
			}
		}

		return true;
	}

	bool cache_waves(
		IoTrack& track)
	{
		struct CacheItem
		{
			int offset_;
			int size_;
			ILTStream* lt_stream_;
		}; // CacheItem

		using FileNameLtFileIdMap = std::unordered_map<std::string, CacheItem>;

		auto map = FileNameLtFileIdMap{};

		auto scope_guard = ul::ScopeGuard{
			[&]()
			{
				for (auto& map_item : map)
				{
					auto& cache_item = map_item.second;

					if (cache_item.lt_stream_)
					{
						cache_item.lt_stream_->Release();
						cache_item.lt_stream_ = nullptr;
					}
				}
			}
		};

		// Evaluate total wave size.
		//
		auto current_offset = 0;

		auto& items = track.waves_.front().parts_.front().items_;

		for (auto& item : items)
		{
			auto& reference = item.references_.front();

			const auto file_name_lc = ul::AsciiUtils::to_lower(reference.file_name_u8_);

			auto map_it = map.find(file_name_lc);

			if (map_it != map.end())
			{
				continue;
			}

			map_it = map.insert({file_name_lc, CacheItem{}}).first;

			auto& cache_item = map_it->second;

			const auto file_path = ul::PathUtils::append(working_dir_, reference.file_name_u8_);

			auto file_ref = FileRef{};
			file_ref.m_FileType = FILE_ANYFILE;
			file_ref.m_pFilename = file_path.c_str();

			cache_item.lt_stream_ = client_file_manager->OpenFile(&file_ref);

			if (!cache_item.lt_stream_)
			{
				// TODO warning
				continue;
			}

			const auto lt_file_size = cache_item.lt_stream_->GetLen();

			if (lt_file_size > max_lt_wave_size)
			{
				error_message_ = "Wave file is too big (max size: " + std::to_string(max_lt_wave_size) + " bytes).";
				return false;
			}

			cache_item.size_ = static_cast<int>(lt_file_size);
			cache_item.offset_ = current_offset;

			current_offset += cache_item.size_;
		}

		// Load all files into the cache.
		//
		wave_cache_.clear();
		wave_cache_.resize(current_offset);

		for (auto& map_item : map)
		{
			auto& cache_item = map_item.second;

			if (!cache_item.lt_stream_)
			{
				continue;
			}

			const auto lt_result = cache_item.lt_stream_->Read(&wave_cache_[cache_item.offset_], cache_item.size_);

			if (lt_result != LT_OK)
			{
				error_message_ = "Failed to load a wave file.";
				return false;
			}

			cache_item.lt_stream_->Release();
			cache_item.lt_stream_ = nullptr;
		}


		// Open memory stream and audio decoder.
		//
		for (auto& item : items)
		{
			auto& reference = item.references_.front();

			const auto file_name_lc = ul::AsciiUtils::to_lower(reference.file_name_u8_);

			auto map_it = map.find(file_name_lc);
			auto& cache_item = map_it->second;

			if (cache_item.size_ <= 0)
			{
				continue;
			}

			const auto open_result = reference.file_stream_.open(
				&wave_cache_[cache_item.offset_], cache_item.size_, ul::Stream::OpenMode::read);

			if (!open_result)
			{
				error_message_ = "Failed to open a memory stream for \"" + reference.file_name_u8_ + "\".";
				return false;
			}

			auto decoder_param = ltjs::AudioDecoder::OpenParameters{};
			decoder_param.dst_bit_depth_ = bit_depth;
			decoder_param.dst_channel_count_ = channel_count;
			decoder_param.dst_sample_rate_ = sample_rate_;
			decoder_param.stream_ptr_ = &reference.file_stream_;

			auto& audio_decoder = reference.audio_decoder_;

			if (!audio_decoder.open(decoder_param))
			{
				error_message_ = "Failed to open an audio decoder for \"" + reference.file_name_u8_ + "\".";
				return false;
			}

			const auto sample_rate = audio_decoder.get_dst_sample_rate();

			if (sample_rate > sample_rate_)
			{
				sample_rate_ = sample_rate;
			}
		}

		return true;
	}

	//
	// "wavt" (list) - wave track list
	//     "wath" - wave track header
	//     "wavp" (list) - wave parts
	//         "waph" - wave part header
	//         "wavi" (list) - wave items
	//             "wave" (list) - wave item
	//                 "waih" - wave item header
	//                 "DMRF" (list) - reference list
	//                     "refh" - reference header
	//                     "file" - file name
	//
	bool read_wave_track(
		IoTrack& track)
	{
		// Wave track list.
		//
		if (!riff_reader_.find_and_descend(ul::RiffFourCcs::list, ul::FourCc{"wavt"}))
		{
			error_message_ = "No wave track.";
			return false;
		}

		// Wave track header.
		//
		if (!riff_reader_.find_and_descend(ul::FourCc{"wath"}))
		{
			error_message_ = "No wave track header.";
			return false;
		}

		track.waves_.emplace_back();
		auto& wave = track.waves_.back();

		auto header_chunk = riff_reader_.get_current_chunk();

		if (!wave.header_.read(&header_chunk.data_stream_))
		{
			error_message_ = "Failed to read a wave track header.";
			return false;
		}

		if (!wave.header_.validate(error_message_))
		{
			error_message_ = "Failed to validate a wave track header: " + error_message_;
			return false;
		}

		// Ascend "wath".
		//
		if (!riff_reader_.ascend())
		{
			error_message_ = "RIFF error.";
			return false;
		}

		// Wave parts.
		//
		if (!riff_reader_.find_and_descend(ul::RiffFourCcs::list, ul::FourCc{"wavp"}))
		{
			error_message_ = "No wave part list.";
			return false;
		}

		while (true)
		{
			// Wave part header.
			//
			if (!riff_reader_.find_and_descend(ul::FourCc{"waph"}))
			{
				break;
			}

			auto part_header_chunk = riff_reader_.get_current_chunk();

			wave.parts_.emplace_back();
			auto& part_item = wave.parts_.back();

			if (!part_item.header_.read(&part_header_chunk.data_stream_))
			{
				error_message_ = "Failed to read a wave part header.";
				return false;
			}

			if (!part_item.header_.validate(error_message_))
			{
				error_message_ = "Failed to validate a wave part header: " + error_message_;
				return false;
			}

			// Ascend "waph".
			//
			if (!riff_reader_.ascend())
			{
				error_message_ = "RIFF error.";
				return false;
			}

			// Wave items.
			//
			if (!riff_reader_.find_and_descend(ul::RiffFourCcs::list, ul::FourCc{"wavi"}))
			{
				continue;
			}

			while (true)
			{
				// Wave item.
				//
				if (!riff_reader_.find_and_descend(ul::RiffFourCcs::list, ul::FourCc{"wave"}))
				{
					break;
				}

				// Wave item header.
				//
				if (!riff_reader_.find_and_descend(ul::FourCc{"waih"}))
				{
					error_message_ = "No wave item header.";
					return false;
				}

				auto waih_chunk = riff_reader_.get_current_chunk();

				part_item.items_.emplace_back();
				auto& wave_item = part_item.items_.back();

				if (!wave_item.header_.read(&waih_chunk.data_stream_))
				{
					error_message_ = "Failed to read wave item header.";
					return false;
				}

				if (!wave_item.header_.validate(error_message_))
				{
					error_message_ = "Failed to validate a wave item header: " + error_message_;
					return false;
				}

				// Ascend "waih".
				//
				if (!riff_reader_.ascend())
				{
					error_message_ = "RIFF error.";
					return false;
				}

				// Reference list.
				//
				if (!riff_reader_.find_and_descend(ul::RiffFourCcs::list, ul::FourCc{"DMRF"}))
				{
					error_message_ = "No reference list.";
					return false;
				}

				// Reference header.
				//
				if (!riff_reader_.find_and_descend(ul::FourCc{"refh"}))
				{
					error_message_ = "No reference item's header.";
					return false;
				}

				auto refh_chunk = riff_reader_.get_current_chunk();

				wave_item.references_.emplace_back();
				auto& reference = wave_item.references_.back();

				if (!reference.header_.read(&refh_chunk.data_stream_))
				{
					error_message_ = "Failed to read a reference header.";
					return false;
				}

				if (!reference.header_.validate(error_message_))
				{
					error_message_ = "Failed to validate a reference header: " + error_message_;
					return false;
				}

				// Ascend "refh".
				//
				if (!riff_reader_.ascend())
				{
					error_message_ = "RIFF error.";
					return false;
				}

				// File name.
				//
				if (!riff_reader_.find_and_descend(ul::FourCc{"file"}))
				{
					error_message_ = "No wave file name.";
					return false;
				}

				auto file_chunk = riff_reader_.get_current_chunk();

				if (file_chunk.size_ < 2 || (file_chunk.size_ % 2) != 0)
				{
					error_message_ = "Invalid wave file name size.";
					return false;
				}

				// Without '\0'.
				const auto file_name_size = static_cast<int>(file_chunk.size_ - 2);

				// Without '\0'.
				const auto file_name_length = file_name_size / 2;

				auto file_name_u16 = std::u16string(
					static_cast<std::u16string::size_type>(file_name_length),
					std::u16string::value_type{});

				const auto file_name_result = file_chunk.data_stream_.read(
					&file_name_u16[0], file_name_size);

				if (file_name_result != file_name_size)
				{
					error_message_ = "Failed to read a wave file name.";
					return false;
				}

				if (!ul::Endian::is_little())
				{
					std::for_each(
						file_name_u16.begin(),
						file_name_u16.end(),
						[](auto& c)
						{
							ul::Endian::swap_i(c);
						}
					);
				}

				reference.file_name_u8_ = ul::EncodingUtils::utf16_to_utf8(file_name_u16);

				// Ascend "file".
				//
				if (!riff_reader_.ascend())
				{
					error_message_ = "RIFF error.";
					return false;
				}

				// Ascend "DMRF".
				//
				if (!riff_reader_.ascend())
				{
					error_message_ = "RIFF error.";
					return false;
				}

				// Ascend "wave".
				//
				if (!riff_reader_.ascend())
				{
					error_message_ = "RIFF error.";
					return false;
				}
			}

			// Ascend "wavi".
			//
			if (!riff_reader_.ascend())
			{
				error_message_ = "RIFF error.";
				return false;
			}

		}

		// Ascend "wavp".
		//
		if (!riff_reader_.ascend())
		{
			error_message_ = "RIFF error.";
			return false;
		}

		// Ascend "wavt".
		//
		if (!riff_reader_.ascend())
		{
			error_message_ = "RIFF error.";
			return false;
		}

		if (!validate_wave_track(track))
		{
			return false;
		}

		if (!cache_waves(track))
		{
			return false;
		}

		return true;
	}

	bool read_track(
		IoTracks& io_tracks)
	{
		if (!riff_reader_.find_and_descend(ul::FourCc{"trkh"}))
		{
			error_message_ = "No track header.";
			return false;
		}

		auto header_chunk = riff_reader_.get_current_chunk();

		io_tracks.emplace_back();
		auto& track = io_tracks.back();

		auto& header = track.header_;

		if (!header.read(header_chunk.data_stream_))
		{
			error_message_ = "Failed to read track's header.";
			return false;
		}

		if (!header.validate(error_message_))
		{
			error_message_ = "Failed to validate track's header: " + error_message_;
			return false;
		}

		// Ascend "trkh".
		//
		if (!riff_reader_.ascend())
		{
			error_message_ = "RIFF error.";
			return false;
		}

		const auto track_type = header.get_type();

		switch (track_type)
		{
		case IoTrackType8::tempo:
			if (!read_tempo_track(track))
			{
				return false;
			}
			break;

		case IoTrackType8::time_signature:
			if (!read_time_signature_track(track))
			{
				return false;
			}
			break;

		case IoTrackType8::sequence:
			if (!read_sequence_track(track))
			{
				return false;
			}
			break;

		case IoTrackType8::wave:
			if (!read_wave_track(track))
			{
				return false;
			}
			break;

		default:
			error_message_ = "Unsupported track type.";
			return false;
		}

		return true;
	}

	bool read_tracks(
		const IoSegmentHeader8& io_segment_header,
		IoTracks& io_tracks)
	{
		if (!riff_reader_.find_and_descend(ul::RiffFourCcs::list, ul::FourCc{"trkl"}))
		{
			error_message_ = "No track list chunk.";
			return false;
		}

		while (true)
		{
			if (!riff_reader_.find_and_descend(ul::RiffFourCcs::riff, ul::FourCc{"DMTK"}))
			{
				break;
			}

			if (!read_track(io_tracks))
			{
				return false;
			}

			if (!riff_reader_.ascend())
			{
				error_message_ = "RIFF error.";
				return false;
			}
		}

		if (!riff_reader_.ascend())
		{
			error_message_ = "RIFF error.";
			return false;
		}

		convert_tracks_to_native(io_segment_header, io_tracks);

		return true;
	}

	bool read_file_image(
		const std::string& file_name)
	{
		auto file_ref = FileRef{};
		file_ref.m_FileType = FILE_ANYFILE;
		file_ref.m_pFilename = file_name.c_str();

		auto lt_stream_ptr = client_file_manager->OpenFile(&file_ref);

		if (!lt_stream_ptr)
		{
			error_message_ = "Failed to open a file.";
			return false;
		}

		auto guard_lt_stream = ul::ScopeGuard
		{
			[&]()
			{
				if (lt_stream_ptr)
				{
					lt_stream_ptr->Release();
				}
			}
		};

		auto file_size = std::uint32_t{};

		if (lt_stream_ptr->GetLen(&file_size) != LT_OK)
		{
			error_message_ = "Failed to get a file size.";
			return false;
		}

		file_image_.resize(file_size);

		if (lt_stream_ptr->Read(file_image_.data(), file_size) != LT_OK)
		{
			error_message_ = "Failed to read a file.";
			return false;
		}

		if (!memory_stream_.open(file_image_.data(), static_cast<int>(file_image_.size()), ul::Stream::OpenMode::read))
		{
			error_message_ = "Failed to open a memory stream.";
			return false;
		}

		return true;
	}

	void convert_tracks_to_native(
		const IoSegmentHeader8& io_segment_header,
		IoTracks& io_tracks)
	{
		// Get quarter beats per minute.
		//
		auto qbpm = default_qbpm;

		for (const auto& io_track : io_tracks)
		{
			if (io_track.tempos_.empty())
			{
				continue;
			}

			qbpm = static_cast<int>(io_track.tempos_.front().tempo_);

			break;
		}

		// Calculate bytes per one music time unit.
		//
		// qbps = quarter_beats_per_second = qbpm / 60
		// units_per_second = qbps * units_per_quarter_beat = qbps * 768
		// seconds_per_unit = 1 / units_per_second = 60 / (qbpm * 768)
		// bytes_per_unit = sample_rate * sample_size * seconds_per_unit
		// bytes_per_unit = (60 * sample_rate * sample_size) / (qbpm * 768)
		//
		const auto sample_size = channel_count * (bit_depth / 8);
		const auto bpu_num = 60LL * sample_rate_ * sample_size; // numerator
		const auto bpu_den = static_cast<long long>(qbpm * units_per_quarter_beat); // denominator


		segment_.is_finished_ = false;
		segment_.mix_offset_ = 0;

		// Calculate a segment length.
		//
		auto segment_length = (bpu_num * io_segment_header.mt_length_) / bpu_den;
		segment_length += sample_size - 1;
		segment_length /= sample_size;
		segment_length *= sample_size;

		segment_.length_ = static_cast<int>(segment_length);


		// Find the wave track.
		//
		auto wave_track_index = -1;
		const auto n_io_tracks = static_cast<int>(io_tracks.size());

		for (auto i = 0; i < n_io_tracks; ++i)
		{
			const auto& io_track = io_tracks[i];
			const auto& io_waves = io_track.waves_;

			if (!io_waves.empty() && !io_waves.front().parts_.empty())
			{
				wave_track_index = i;
				break;
			}
		}

		if (wave_track_index < 0)
		{
			io_tracks.clear();
			return;
		}

		// Add wave items.
		//
		auto& waves = segment_.waves_;
		auto& io_wave_part = io_tracks[wave_track_index].waves_.front().parts_.front();
		const auto& io_wave_part_header = io_wave_part.header_;

		waves.channel_ = io_wave_part_header.channel_;
		waves.is_variation_no_repeat_ = (io_wave_part_header.flags_ == IoWavePartHeader8::VariationType::no_repeat);
		waves.last_variation_index_ = -1;
		waves.current_variation_mask_ = 0;
		waves.variation_count_ = 0;

		// Initialize variations.
		//
		if (io_wave_part_header.variations_ == all_flags_on)
		{
			waves.variation_count_ = 1;

			waves.variation_mask_list_.reserve(waves.variation_count_);
			waves.variation_mask_list_.emplace_back(default_variation_mask);
		}
		else
		{
			for (auto i = 0; i < 32; ++i)
			{
				const auto mask = 1U << i;

				if ((io_wave_part_header.variations_ & mask) != 0)
				{
					waves.variation_count_ += 1;
				}
			}

			waves.variation_mask_list_.reserve(waves.variation_count_);

			for (auto i = 0; i < 32; ++i)
			{
				const auto mask = 1U << i;

				if ((io_wave_part_header.variations_ & mask) != 0)
				{
					waves.variation_mask_list_.emplace_back(mask);
				}
			}
		}

		// Initialize the items.
		//
		for (auto& io_wave_item : io_wave_part.items_)
		{
			waves.items_.emplace_back();
			auto& wave_item = waves.items_.back();

			const auto& io_wave_item_header = io_wave_item.header_;

			wave_item.variation_masks_ = io_wave_item_header.variations_;

			// Calculate the length.
			//
			auto item_length = (bpu_num * io_wave_item_header.rt_duration_) / bpu_den;
			item_length += sample_size - 1;
			item_length /= sample_size;
			item_length *= sample_size;

			wave_item.length_ = static_cast<int>(item_length);

			// Steal the stream and the decoder.
			//
			auto& io_wave_ref = io_wave_item.references_.front();
			wave_item.stream_ = std::move(io_wave_ref.file_stream_);
			wave_item.decoder_ = std::move(io_wave_ref.audio_decoder_);
		}

		// Setup reference active item list.
		//
		const auto item_count = static_cast<int>(waves.items_.size());

		waves.objects_template_.resize(item_count);
		waves.active_objects_.reserve(item_count);

		for (auto i = 0; i < item_count; ++i)
		{
			auto& active_item = waves.objects_template_[i];

			// Calculate mix offset.
			//
			auto mix_offset = (bpu_num * io_wave_part.items_[i].header_.mt_logical_time) / bpu_den;
			mix_offset += sample_size - 1;
			mix_offset /= sample_size;
			mix_offset *= sample_size;

			active_item.mix_offset_ = static_cast<int>(mix_offset);

			// Set the reset of the fields.
			//
			active_item.decode_offset_ = 0;
			active_item.item_ptr_ = &waves.items_[i];
		}

		// Sort the list (from latest to the earliest).
		//
		std::sort(
			waves.objects_template_.begin(),
			waves.objects_template_.end(),
			[](const auto& lhs, const auto& rhs)
			{
				return lhs.mix_offset_ >= rhs.mix_offset_;
			}
		);
	}

	bool open_internal(
		const std::string& file_name)
	{
		if (!read_file_image(file_name))
		{
			return false;
		}

		working_dir_ = ul::PathUtils::get_parent_path(file_name);

		if (!riff_reader_.open(&memory_stream_, ul::FourCc{"DMSG"}))
		{
			error_message_ = "Not a segment file.";
			return false;
		}

		auto io_segment_header = IoSegmentHeader8{};

		if (!read_io_segment_header(io_segment_header))
		{
			return false;
		}

		auto io_tracks = IoTracks{};

		if (!read_tracks(io_segment_header, io_tracks))
		{
			return false;
		}

		rewind();

		is_open_ = true;

#ifdef LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_STRUCTURE
		debug_dump_structure(file_name, io_segment_header, io_tracks);
#endif // LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_STRUCTURE

#ifdef LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_WAVES
		debug_dump_waves(file_name);
#endif // LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_WAVES

		return true;
	}

	void close_internal()
	{
		is_open_ = {};
		file_image_.clear();
		riff_reader_.close();
		memory_stream_.close();
		wave_cache_.clear();
		sample_rate_ = {};
		segment_ = {};
	}

	bool rewind()
	{
		segment_.is_finished_ = false;
		segment_.mix_offset_ = {};

		auto& waves = segment_.waves_;

		waves.active_objects_.clear();
		waves.inactive_objects_ = waves.objects_template_;

		for (auto& item : waves.items_)
		{
			if (!item.decoder_.rewind())
			{
				error_message_ = "Failed to rewind an audio decoder.";
				close_internal();
				return false;
			}
		}


		// Select a random variation.
		//
		if (waves.variation_count_ > 0)
		{
			auto variation_index = -1;

			if (waves.variation_count_ > 1)
			{
				while (true)
				{
					variation_index = get_random_value(waves.variation_count_ - 1);

					if (waves.is_variation_no_repeat_)
					{
						if (waves.last_variation_index_ >= 0)
						{
							if (variation_index != waves.last_variation_index_)
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
			}
			else
			{
				variation_index = 0;
			}

			waves.last_variation_index_ = variation_index;
			waves.current_variation_mask_ = waves.variation_mask_list_[variation_index];
		}

		return true;
	}

	int mix(
		const int src_decode_size,
		std::int16_t* dst_decode_buffer,
		float* dst_mix_buffer)
	{
		const auto is_overlapping = (segment_.mix_offset_ >= segment_.length_);

		if (is_overlapping)
		{
			if (segment_.waves_.active_objects_.empty())
			{
				segment_.is_finished_ = true;
				return 0;
			}
		}

		const auto sample_size_per_channel = bit_depth / 8;
		const auto sample_size = channel_count * sample_size_per_channel;
		auto decode_size = (src_decode_size / sample_size) * sample_size;

		if (!is_overlapping)
		{
			if ((segment_.mix_offset_ + decode_size) > segment_.length_)
			{
				decode_size = segment_.length_ - segment_.mix_offset_;
			}
		}

		if (segment_.waves_.items_.empty())
		{
			segment_.mix_offset_ += decode_size;
			return decode_size;
		}

		const auto variation_mask = segment_.waves_.current_variation_mask_;

		// Update lists.
		//
		auto& actives = segment_.waves_.active_objects_;
		auto& inactives = segment_.waves_.inactive_objects_;

		while (!inactives.empty())
		{
			const auto& inactive = inactives.back();

			if ((inactive.item_ptr_->variation_masks_ & variation_mask) == 0)
			{
				inactives.pop_back();
				continue;
			}

			if (inactive.mix_offset_ >= (segment_.mix_offset_ + decode_size))
			{
				break;
			}

			actives.emplace_back(inactive);
			inactives.pop_back();
		}

		// Process active objects.
		//
		for (auto& active : actives)
		{
			auto mix_offset = 0;
			auto to_decode_size = decode_size;

			if (!active.is_started_)
			{
				active.is_started_ = true;

				if (active.mix_offset_ > segment_.mix_offset_)
				{
					mix_offset = active.mix_offset_ - segment_.mix_offset_;
					to_decode_size -= mix_offset;
				}
			}

			const auto active_remain_size = active.item_ptr_->length_ - active.decode_offset_;

			if (to_decode_size > active_remain_size)
			{
				to_decode_size = active_remain_size;
			}

			const auto decoded_size = active.item_ptr_->decoder_.decode(dst_decode_buffer, to_decode_size);

			if (decoded_size > 0)
			{
				const auto mix_index = mix_offset / sample_size_per_channel;
				const auto sample_count = decoded_size / sample_size_per_channel;

				auto decode_buffer = static_cast<const std::int16_t*>(dst_decode_buffer);
				auto mix_buffer = static_cast<float*>(dst_mix_buffer) + mix_index;

				for (auto i = 0; i < sample_count; ++i)
				{
					mix_buffer[i] += decode_buffer[i] / 32768.0F;
				}

				active.decode_offset_ += decoded_size;

				if (active.decode_offset_ == active.item_ptr_->length_)
				{
					active.is_finished_ = true;
				}
			}
			else
			{
				active.is_finished_ = true;
			}
		}

		// Remove finished objects.
		//
		{
			auto begin_it = actives.begin();
			auto end_it = actives.end();

			auto new_end_it = std::remove_if(
				actives.begin(),
				actives.end(),
				[](const auto& item)
				{
					return item.is_finished_;
				}
			);

			if (new_end_it != end_it)
			{
				actives.resize(new_end_it - begin_it);
			}
		}

		segment_.mix_offset_ += decode_size;

		return decode_size;
	}

	static int get_random_value(
		const int max_value)
	{
		static std::random_device random_device{};
		static auto random_engine = std::mt19937{random_device()};
		static auto random_dist = std::uniform_int_distribution<int>{};

		const auto random_value = random_dist(random_engine);

		return random_value % max_value;
	}


	// ======================================================================
	// Debug stuff
	//

#ifdef LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_STRUCTURE
	std::filebuf debug_filebuf_;


	void debug_write_string(
		const std::string& string)
	{
		if (string.empty())
		{
			return;
		}

		static_cast<void>(debug_filebuf_.sputn(string.data(), string.length()));
	}

	void debug_write_line(
		const std::string& string)
	{
		debug_write_string(string);
		static_cast<void>(debug_filebuf_.sputc('\n'));
	}

	void debug_write_line()
	{
		static_cast<void>(debug_filebuf_.sputc('\n'));
	}

	template<int TBitCount>
	std::string debug_flags_to_string(
		const std::uint32_t flags)
	{
		auto result = std::string(std::string::size_type{TBitCount}, '0');

		if (flags != 0)
		{
			for (auto i = 0; i < TBitCount; ++i)
			{
				if ((flags & (1 << i)) != 0)
				{
					result[TBitCount - i - 1] = '1';
				}
			}
		}

		return result;
	}

	template<typename T>
	std::string debug_flags_to_string(
		const T& flags)
	{
		return debug_flags_to_string<static_cast<int>(sizeof(T) * 8)>(static_cast<std::uint32_t>(flags));
	}

	void debug_dump_structure(
		const std::string& file_name,
		const IoSegmentHeader8& io_segment_header,
		const IoTracks& io_tracks)
	{
		auto file_path = ul::PathUtils::get_parent_path(file_name);
		std::replace(file_path.begin(), file_path.end(), '\\', '_');
		std::replace(file_path.begin(), file_path.end(), '/', '_');
		ul::AsciiUtils::to_lower_i(file_path);
		file_path = "ltjs_dbg_dmus001_" + file_path + ".txt";

		static_cast<void>(debug_filebuf_.open(file_path, std::ios_base::out | std::ios_base::binary | std::ios_base::app));

		if (!debug_filebuf_.is_open())
		{
			return;
		}


		debug_write_line();
		debug_write_line("=============================================================================");
		debug_write_line(file_name);
		debug_write_line("-----------------------------------------------------------------------------");

		// Header.
		//
		debug_write_line("Header:");
		debug_write_line("\trepeat_count_: " + std::to_string(io_segment_header.repeat_count_));
		debug_write_line("\tmt_length_: " + std::to_string(io_segment_header.mt_length_));
		debug_write_line("\tmt_play_start_: " + std::to_string(io_segment_header.mt_play_start_));
		debug_write_line("\tmt_loop_start_: " + std::to_string(io_segment_header.mt_loop_start_));
		debug_write_line("\tmt_loop_end_: " + std::to_string(io_segment_header.mt_loop_end_));
		debug_write_line("\tresolution_: " + std::to_string(io_segment_header.resolution_));
		debug_write_line("\trt_length_: " + std::to_string(io_segment_header.rt_length_));

		debug_write_line("\tflags_: " + std::to_string(io_segment_header.flags_) + " (" +
			debug_flags_to_string(io_segment_header.flags_) + ")");

		// Tracks.
		//
		auto n_tracks = static_cast<int>(io_tracks.size());

		for (auto i_track = 0; i_track < n_tracks; ++i_track)
		{
			debug_write_line();
			debug_write_line("Track " + std::to_string(i_track) + ":");

			const auto& track = io_tracks[i_track];

			const auto& track_header = track.header_;
			debug_write_line("\tHeader:");
			debug_write_line("\t\tguid_: " + track_header.guid_.to_string());
			debug_write_line("\t\tposition_: " + std::to_string(track_header.position_));
			debug_write_line("\t\tgroup_: " + std::to_string(track_header.group_) + " (" + debug_flags_to_string(track_header.group_) + ")");
			debug_write_line("\t\tchunk_id_: " + std::to_string(track_header.chunk_id_) + " (\"" + track_header.chunk_id_.to_string() + "\")");
			debug_write_line("\t\tlist_type_: " + std::to_string(track_header.list_type_) + " (\"" + track_header.list_type_.to_string() + "\")");

			// Sequences.
			//
			const auto n_sequences = static_cast<int>(track.sequences_.size());

			for (auto i_sequence = 0; i_sequence < n_sequences; ++i_sequence)
			{
				debug_write_line();
				debug_write_line("\tSequence " + std::to_string(i_sequence) + ":");
				const auto& sequence = track.sequences_[i_sequence];

				// Event item.
				//
				const auto n_events = static_cast<int>(sequence.events_.size());

				for (auto i_event = 0; i_event < n_events; ++i_event)
				{
					const auto& event = sequence.events_[i_event];
					debug_write_line();
					debug_write_line("\t\tEvent " + std::to_string(i_event) + ":");
					debug_write_line("\t\t\tmt_time_: " + std::to_string(event.mt_time_));
					debug_write_line("\t\t\tmt_duration_: " + std::to_string(event.mt_duration_));
					debug_write_line("\t\t\tchannel_: " + std::to_string(event.channel_));
					debug_write_line("\t\t\toffset_: " + std::to_string(event.offset_));
					debug_write_line("\t\t\tstatus_: " + std::to_string(static_cast<int>(event.status_)));
					debug_write_line("\t\t\tbyte_1_: " + std::to_string(static_cast<int>(event.byte_1_)));
					debug_write_line("\t\t\tbyte_2_: " + std::to_string(static_cast<int>(event.byte_2_)));
				}

				// Curve item.
				//
				const auto n_curves = static_cast<int>(sequence.curves_.size());

				for (auto i_curve = 0; i_curve < n_curves; ++i_curve)
				{
					const auto& curve = sequence.curves_[i_curve];
					debug_write_line();
					debug_write_line("\t\tCurve " + std::to_string(i_curve) + ":");
					debug_write_line("\t\t\tmt_start_: " + std::to_string(curve.mt_start_));
					debug_write_line("\t\t\tmt_duration_: " + std::to_string(curve.mt_duration_));
					debug_write_line("\t\t\tmt_reset_duration_: " + std::to_string(curve.mt_reset_duration_));
					debug_write_line("\t\t\tchannel_: " + std::to_string(curve.channel_));
					debug_write_line("\t\t\toffset_: " + std::to_string(curve.offset_));
					debug_write_line("\t\t\tstart_value_: " + std::to_string(curve.start_value_));
					debug_write_line("\t\t\tend_value_: " + std::to_string(curve.end_value_));
					debug_write_line("\t\t\treset_value_: " + std::to_string(curve.reset_value_));
					debug_write_line("\t\t\ttype_: " + std::to_string(static_cast<int>(curve.type_)));
					debug_write_line("\t\t\tcurve_shape_: " + std::to_string(static_cast<int>(curve.curve_shape_)));
					debug_write_line("\t\t\tcc_data_: " + std::to_string(static_cast<int>(curve.cc_data_)));
					debug_write_line("\t\t\tflags_: " + std::to_string(static_cast<int>(curve.flags_)) + " (" + debug_flags_to_string(curve.flags_) + ")");
					debug_write_line("\t\t\tparam_type_: " + std::to_string(curve.param_type_));
					debug_write_line("\t\t\tmerge_index_: " + std::to_string(curve.merge_index_));
				}
			}

			// Tempos.
			//
			const auto n_tempos = static_cast<int>(track.tempos_.size());

			for (auto i_tempo = 0; i_tempo < n_tempos; ++i_tempo)
			{
				debug_write_line();
				debug_write_line("\tTempo " + std::to_string(i_tempo) + ":");

				const auto& tempo = track.tempos_[i_tempo];
				debug_write_line("\t\ttime_: " + std::to_string(tempo.time_));
				debug_write_line("\t\ttempo_: " + std::to_string(tempo.tempo_));
			}

			// Time signatures.
			//
			const auto n_times = static_cast<int>(track.times_.size());

			for (auto i_time = 0; i_time < n_times; ++i_time)
			{
				debug_write_line();
				debug_write_line("\tTime signature " + std::to_string(i_time) + ":");

				const auto& time = track.times_[i_time];
				debug_write_line("\t\ttime_: " + std::to_string(time.time_));
				debug_write_line("\t\tbeats_per_measure_: " + std::to_string(static_cast<int>(time.beats_per_measure_)));
				debug_write_line("\t\tbeat_: " + std::to_string(static_cast<int>(time.beat_)));
				debug_write_line("\t\tgrids_per_beat_: " + std::to_string(time.grids_per_beat_));
			}

			// Waves.
			//
			const auto n_waves = static_cast<int>(track.waves_.size());

			for (auto i_wave = 0; i_wave < n_waves; ++i_wave)
			{
				debug_write_line();
				debug_write_line("\tWave " + std::to_string(i_wave) + ":");

				const auto& wave = track.waves_[i_wave];

				// Header.
				//
				const auto& wave_header = wave.header_;
				debug_write_line("\t\tHeader:");
				debug_write_line("\t\t\tvolume_: " + std::to_string(wave_header.volume_));
				debug_write_line("\t\t\tflags_: " + std::to_string(wave_header.flags_) + " (" + debug_flags_to_string(wave_header.flags_) + ")");

				// Parts.
				//
				const auto n_parts = static_cast<int>(wave.parts_.size());

				for (auto i_part = 0; i_part < n_parts; ++i_part)
				{
					debug_write_line("\t\tPart " + std::to_string(i_part) + ":");
					const auto& part = wave.parts_[i_part];

					// Header.
					//
					debug_write_line("\t\t\tHeader:");
					const auto& part_header = part.header_;
					debug_write_line("\t\t\t\tvolume_: " + std::to_string(part_header.volume_));
					debug_write_line("\t\t\t\tvariations_: " + std::to_string(part_header.variations_) + " (" + debug_flags_to_string(part_header.variations_) + ")");
					debug_write_line("\t\t\t\tchannel_: " + std::to_string(part_header.channel_));
					debug_write_line("\t\t\t\tlock_to_part_: " + std::to_string(part_header.lock_to_part_));
					debug_write_line("\t\t\t\tflags_: " + std::to_string(static_cast<uint32_t>(part_header.flags_)) + " (" + debug_flags_to_string(part_header.flags_) + ")");
					debug_write_line("\t\t\t\tindex_: " + std::to_string(part_header.index_));


					// Parts.
					//
					const auto n_items = static_cast<int>(part.items_.size());

					for (auto i_item = 0; i_item < n_items; ++i_item)
					{
						debug_write_line();
						debug_write_line("\t\t\tItem " + std::to_string(i_item) + ":");
						const auto& item = part.items_[i_item];

						// Header.
						//
						debug_write_line("\t\t\t\tHeader:");
						const auto& item_header = item.header_;
						debug_write_line("\t\t\t\t\tvolume_: " + std::to_string(item_header.volume_));
						debug_write_line("\t\t\t\t\tpitch_: " + std::to_string(item_header.pitch_));
						debug_write_line("\t\t\t\t\tvariations_: " + std::to_string(item_header.variations_) + " (" + debug_flags_to_string(item_header.variations_) + ")");
						debug_write_line("\t\t\t\t\trt_time_: " + std::to_string(item_header.rt_time_));
						debug_write_line("\t\t\t\t\trt_start_offset_: " + std::to_string(item_header.rt_start_offset_));
						debug_write_line("\t\t\t\t\trt_reserved_: " + std::to_string(item_header.rt_reserved_));
						debug_write_line("\t\t\t\t\trt_duration_: " + std::to_string(item_header.rt_duration_));
						debug_write_line("\t\t\t\t\tmt_logical_time: " + std::to_string(item_header.mt_logical_time));
						debug_write_line("\t\t\t\t\tloop_start_: " + std::to_string(item_header.loop_start_));
						debug_write_line("\t\t\t\t\tloop_end_: " + std::to_string(item_header.loop_end_));
						debug_write_line("\t\t\t\t\tflags_: " + std::to_string(item_header.flags_) + " (" + debug_flags_to_string(item_header.flags_) + ")");

						// References.
						//
						const auto n_refs = static_cast<int>(item.references_.size());

						for (auto i_ref = 0; i_ref < n_refs; ++i_ref)
						{
							debug_write_line();
							debug_write_line("\t\t\t\tReference " + std::to_string(i_ref) + ":");
							const auto& ref = item.references_[i_ref];

							// Header.
							//
							debug_write_line("\t\t\t\t\tHeader: ");
							const auto& ref_header = ref.header_;
							debug_write_line("\t\t\t\t\t\tclsid_: " + ref_header.clsid_.to_string());
							debug_write_line("\t\t\t\t\t\tvalid_data_: " + std::to_string(ref_header.valid_data_) + " (" + debug_flags_to_string(ref_header.valid_data_) + ")");

							// File name.
							//
							debug_write_line("\t\t\t\t\tFile name:");
							debug_write_line("\t\t\t\t\t\t\"" + ref.file_name_u8_ + "\"");
						}
					}
				}
			}
		}

		debug_write_line("=============================================================================");

		debug_filebuf_.close();
	}
#endif // LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_STRUCTURE

#ifdef LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_WAVES
	// TODO Apply low-pass filter.
	void debug_dump_waves(
		const std::string& file_name)
	{
		const auto wave_prefix_size =
			4 + 4 + 4 + // "RIFF"<size>"WAVE"
			4 + 4 + ul::PcmWaveFormat::class_size + // "fmt "<size><data>
			4 + 4 + // "data"<size>
			0;

		auto file_path = file_name;
		std::replace(file_path.begin(), file_path.end(), '\\', '_');
		std::replace(file_path.begin(), file_path.end(), '/', '_');
		std::replace(file_path.begin(), file_path.end(), '.', '_');
		ul::AsciiUtils::to_lower_i(file_path);
		file_path = "ltjs_dbg_dmus002_" + file_path + ".wav";

		auto debug_file = ul::FileStream{file_path, ul::Stream::OpenMode::write};

		if (!debug_file.is_open())
		{
			return;
		}

		if (!debug_file.set_position(wave_prefix_size))
		{
			return;
		}

		const auto max_sample_count = 4096;
		const auto max_buffer_size = max_sample_count * channel_count;
		const auto max_decode_size = max_buffer_size * (bit_depth / 8);

		using BufferS16 = std::vector<std::int16_t>;
		using MixBufferF = std::vector<float>;

		auto decode_buffer = BufferS16{};
		decode_buffer.resize(max_buffer_size);

		auto write_buffer = BufferS16{};
		write_buffer.resize(max_buffer_size);

		auto mix_buffer = MixBufferF{};
		mix_buffer.resize(max_buffer_size);

		rewind();

		auto total_decoded_size = 0;

		while (true)
		{
			std::fill(mix_buffer.begin(), mix_buffer.end(), 0.0F);

			const auto decoded_size = mix(max_decode_size, decode_buffer.data(), mix_buffer.data());

			if (decoded_size <= 0)
			{
				break;
			}

			const auto sample_count = decoded_size / (bit_depth / 8);

			for (auto i = 0; i < sample_count; ++i)
			{
				write_buffer[i] = ul::Endian::little(static_cast<std::int16_t>(mix_buffer[i] * 32768.0F));
			}

			debug_file.write(write_buffer.data(), decoded_size);

			total_decoded_size += decoded_size;
		}

		static_cast<void>(debug_file.set_position(0));

		const auto riff_id_le = ul::Endian::little(static_cast<std::uint32_t>(ul::RiffFourCcs::riff));
		const auto riff_size_le = ul::Endian::little(static_cast<std::uint32_t>(wave_prefix_size + total_decoded_size - 8));
		const auto wave_id_le = ul::Endian::little(static_cast<std::uint32_t>(ul::WaveFourCcs::wave));
		const auto fmt_id_le = ul::Endian::little(static_cast<std::uint32_t>(ul::WaveFourCcs::fmt));
		const auto fmt_size_le = ul::Endian::little(static_cast<std::uint32_t>(ul::PcmWaveFormat::class_size));
		const auto data_id_le = ul::Endian::little(static_cast<std::uint32_t>(ul::WaveFourCcs::data));
		const auto data_size_le = ul::Endian::little(static_cast<std::uint32_t>(total_decoded_size));

		auto waveformat = ul::PcmWaveFormat{};
		waveformat.tag_ = ul::WaveFormatTag::pcm;
		waveformat.bit_depth_ = bit_depth;
		waveformat.channel_count_ = channel_count;
		waveformat.sample_rate_ = sample_rate_;
		waveformat.block_align_ = waveformat.channel_count_ * (waveformat.bit_depth_ / 8);
		waveformat.avg_bytes_per_sec_ = waveformat.block_align_ * waveformat.sample_rate_;

		static_cast<void>(debug_file.write(&riff_id_le, 4));
		static_cast<void>(debug_file.write(&riff_size_le, 4));
		static_cast<void>(debug_file.write(&wave_id_le, 4));
		static_cast<void>(debug_file.write(&fmt_id_le, 4));
		static_cast<void>(debug_file.write(&fmt_size_le, 4));
		static_cast<void>(ul::WaveformatUtils::write(waveformat, &debug_file));
		static_cast<void>(debug_file.write(&data_id_le, 4));
		static_cast<void>(debug_file.write(&data_size_le, 4));

		debug_file.close();
	}
#endif // LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_WAVES

	//
	// Debug stuff
	// ======================================================================
}; // DMusicSegment::Impl


const ul::Uuid DMusicSegment::Impl::clsid_tempo_track = ul::Uuid{"D2AC2885-B39B-11D1-8704-00600893B1BD"};
const ul::Uuid DMusicSegment::Impl::clsid_time_sig_track = ul::Uuid{"D2AC2888-B39B-11D1-8704-00600893B1BD"};
const ul::Uuid DMusicSegment::Impl::clsid_sequence_track = ul::Uuid{"D2AC2886-B39B-11D1-8704-00600893B1BD"};
const ul::Uuid DMusicSegment::Impl::clsid_wave_track = ul::Uuid{"EED36461-9EA5-11D3-9BD1-0080C7150A74"};
const ul::Uuid DMusicSegment::Impl::clsid_sound_wave = ul::Uuid{"8A667154-F9CB-11D2-AD8A-0060B0575ABC"};


DMusicSegment::DMusicSegment()
	:
	pimpl_{std::make_unique<Impl>()}
{
}

DMusicSegment::DMusicSegment(
	DMusicSegment&& that)
	:
	pimpl_{std::move(that.pimpl_)}
{
}

DMusicSegment& DMusicSegment::operator=(
	DMusicSegment&& that)
{
	if (this != std::addressof(that))
	{
		pimpl_ = std::move(that.pimpl_);
	}

	return *this;
}

DMusicSegment::~DMusicSegment()
{
}

bool DMusicSegment::open(
	const std::string& file_name,
	const int sample_rate)
{
	return pimpl_->api_open(file_name, sample_rate);
}

void DMusicSegment::close()
{
	pimpl_->api_close();
}

bool DMusicSegment::rewind()
{
	return pimpl_->api_rewind();
}

int DMusicSegment::mix(
	const int src_decode_size,
	std::int16_t* dst_decode_buffer,
	float* dst_mix_buffer)
{
	return pimpl_->api_mix(src_decode_size, dst_decode_buffer, dst_mix_buffer);
}

int DMusicSegment::get_length() const
{
	return pimpl_->api_get_length();
}

bool DMusicSegment::is_finished() const
{
	return pimpl_->api_is_finished();
}

bool DMusicSegment::is_silence() const
{
	return pimpl_->api_is_silence();
}

const std::string& DMusicSegment::get_error_message() const
{
	return pimpl_->api_get_error_message();
}


}; // ltjs
