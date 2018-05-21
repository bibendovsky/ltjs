#include "bdefs.h"


#ifdef _DEBUG
#define LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_STRUCTURE
#endif // _DEBUG


#include "ltjs_dmusic_segment.h"
#include <cstdint>
#include <array>
#include <fstream>
#include <string>
#include <utility>
#include <vector>
#include "bibendovsky_spul_ascii_utils.h"
#include "bibendovsky_spul_enum_flags.h"
#include "bibendovsky_spul_endian.h"
#include "bibendovsky_spul_four_cc.h"
#include "bibendovsky_spul_memory_stream.h"
#include "bibendovsky_spul_path_utils.h"
#include "bibendovsky_spul_riff_four_ccs.h"
#include "bibendovsky_spul_riff_reader.h"
#include "bibendovsky_spul_scope_guard.h"
#include "bibendovsky_spul_uuid.h"
#include "bibendovsky_spul_encoding_utils.h"
#include "client_filemgr.h"


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
		error_message_{},
		file_image_{},
		memory_stream_{},
		riff_reader_{},
		io_segment_header_{},
		io_tracks_{}
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
		error_message_{std::move(that.error_message_)},
		file_image_{std::move(that.file_image_)},
		memory_stream_{std::move(that.memory_stream_)},
		riff_reader_{std::move(that.riff_reader_)},
		io_segment_header_{std::move(that.io_segment_header_)},
		io_tracks_{std::move(that.io_tracks_)}
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
		const std::string& file_name)
	{
		error_message_.clear();

		close_internal();

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

	const std::string& api_get_error_message() const
	{
		return error_message_;
	}


private:
	static constexpr auto all_flags_on = std::uint32_t{0xFFFFFFFF};


	using Buffer = std::vector<std::uint8_t>;


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
				is_reference_length = 0B0001,
			}; // Value
		}; // Flags


		std::uint32_t repeat_count_;
		IoMusicTime8 mt_length_;
		IoMusicTime8 mt_play_start_;
		IoMusicTime8 mt_loop_start_;
		IoMusicTime8 mt_loop_end_;
		std::uint32_t resolution_;
		IoReferenceTime8 rt_length_;
		Flags flags_;
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

			if (mt_length_ < 0)
			{
				error_message = "Negative music length.";
				return false;
			}

			if (mt_play_start_ < 0)
			{
				error_message = "Negative music play start.";
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

			// Skip resolution.

			if (rt_length_ < 0)
			{
				error_message = "Negative reference length.";
				return false;
			}

			if (!(flags_ == 0 || flags_ == Flags::is_reference_length))
			{
				error_message = "Unexpected set of flags.";
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

			if (tempo_ < 0.0)
			{
				error_message = "Negative tempo.";
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

			// Skip beats per measure.

			// Beat.
			//
			const auto beat = (beat_ > 0 ? beat_ : 256);

			switch (beat)
			{
			case 1:
			case 2:
			case 4:
			case 8:
			case 16:
			case 32:
			case 64:
			case 128:
			case 256:
				break;

			default:
				error_message = "Invalid beat value.";
				return false;
			}

			// Skip grids per beat.

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

			// Skip channel.

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

	struct IoCurveItem8
	{
		static constexpr auto class_size = 32;


		IoMusicTime8 mt_start_;
		IoMusicTime8 mt_duration_;
		IoMusicTime8 mt_reset_duration_;
		std::uint32_t channel_;
		std::int16_t offset_;
		std::int16_t start_value_;
		std::int16_t end_value_;
		std::int16_t reset_value_;
		std::uint8_t type_;
		std::uint8_t curve_shape_;
		std::uint8_t cc_data_;
		std::uint8_t flags_;
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
			if (mt_start_ < 0)
			{
				error_message = "Negative music time.";
				return false;
			}

			if (mt_duration_ != 0)
			{
				error_message = "Expected zero music duration.";
				return false;
			}

			if (mt_reset_duration_ < 0)
			{
				error_message = "Negative music reset duration.";
				return false;
			}

			// Skip channel.

			if (offset_ < 0)
			{
				error_message = "Negative music offset.";
				return false;
			}

			if (start_value_ != 0)
			{
				error_message = "Expected zero start value.";
				return false;
			}

			if (end_value_ != 0)
			{
				error_message = "Expected zero end value.";
				return false;
			}

			if (reset_value_ < 0)
			{
				error_message = "Negative zero reset value.";
				return false;
			}

			// Skip type.

			if (curve_shape_ != 0)
			{
				error_message = "Expected linear curve shape.";
				return false;
			}

			if (cc_data_ != 0)
			{
				error_message = "Expected zero CC data.";
				return false;
			}

			if (flags_ != 0)
			{
				error_message = "Expected zero flags.";
				return false;
			}

			// Skip param type.

			// Skip merge index_.

			return true;
		}
	}; // IoCurveItem8

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
			if (volume_ > 0)
			{
				error_message = "Positive volume.";
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


		std::int32_t volume_;
		std::uint32_t variations_;
		std::uint32_t channel_;
		std::uint32_t lock_to_part_;
		std::uint32_t flags_;
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
			if (volume_ > 0)
			{
				error_message = "Positive volume.";
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

			// Skip flags.

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
			if (volume_ > 0)
			{
				error_message = "Positive volume.";
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

			// Skip reserved.

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

			// Skip position.

			// Skip group.

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
		IoSequenceItem8 event_;
		IoCurveItem8 curve_;
	}; // IoSequenceItem

	using IoSequenceItems = std::vector<IoSequenceItem>;

	struct IoReference
	{
		IoReference8 header_;
		std::u16string u16_file_name_;
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


	std::string error_message_;
	Buffer file_image_;
	ul::MemoryStream memory_stream_;
	ul::RiffReader riff_reader_;
	IoSegmentHeader8 io_segment_header_;
	IoTracks io_tracks_;


	bool read_io_segment_header()
	{
		if (!riff_reader_.find_and_descend(ul::FourCc{"segh"}))
		{
			error_message_ = "No header.";
			return false;
		}

		auto io_header_chunk = riff_reader_.get_current_chunk();

		if (!io_segment_header_.read(&io_header_chunk.data_stream_))
		{
			error_message_ = "Failed to read a header.";
			return false;
		}

		if (!io_segment_header_.validate(error_message_))
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

		track.tempos_.emplace_back();
		auto& item = track.tempos_.back();

		auto chunk = riff_reader_.get_current_chunk();

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

		while (true)
		{
			if (!riff_reader_.find_and_descend(ul::FourCc{"tims"}))
			{
				break;
			}

			track.times_.emplace_back();
			auto& item = track.times_.back();

			auto chunk = riff_reader_.get_current_chunk();

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

		if (!sequence.event_.read(&evtl_chunk.data_stream_))
		{
			error_message_ = "Failed to read a sequence item.";
			return false;
		}

		if (!sequence.event_.validate(error_message_))
		{
			error_message_ = "Failed to validate a sequence item: " + error_message_;
			return false;
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

		if (!sequence.curve_.read(&curl_chunk.data_stream_))
		{
			error_message_ = "Failed to read a curve item.";
			return false;
		}

		if (!sequence.curve_.validate(error_message_))
		{
			error_message_ = "Failed to validate a curve item: " + error_message_;
			return false;
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

				reference.u16_file_name_.resize(file_name_length);

				const auto file_name_result = file_chunk.data_stream_.read(
					&reference.u16_file_name_[0], file_name_size);

				if (file_name_result != file_name_size)
				{
					error_message_ = "Failed to read a wave file name.";
					return false;
				}

				if (!ul::Endian::is_little())
				{
					std::for_each(
						reference.u16_file_name_.begin(),
						reference.u16_file_name_.end(),
						[](auto& c)
						{
							ul::Endian::swap_i(c);
						}
					);
				}

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

		return true;
	}

	bool read_track()
	{
		if (!riff_reader_.find_and_descend(ul::FourCc{"trkh"}))
		{
			error_message_ = "No track header.";
			return false;
		}

		auto header_chunk = riff_reader_.get_current_chunk();

		io_tracks_.emplace_back();
		auto& track = io_tracks_.back();

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

	bool read_tracks()
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

			if (!read_track())
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

	bool open_internal(
		const std::string& file_name)
	{
		if (!read_file_image(file_name))
		{
			return false;
		}

		if (!riff_reader_.open(&memory_stream_, ul::FourCc{"DMSG"}))
		{
			error_message_ = "Not a segment file.";
			return false;
		}

		if (!read_io_segment_header())
		{
			return false;
		}

		if (!read_tracks())
		{
			return false;
		}

#ifdef LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_STRUCTURE
		debug_dump_structure(file_name);
#endif // LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_STRUCTURE

		return true;
	}

	void close_internal()
	{
		file_image_.clear();
		riff_reader_.close();
		memory_stream_.close();
		io_segment_header_ = {};
		io_tracks_.clear();
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

	void debug_dump_structure(
		const std::string& file_name)
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

		debug_write_line("Header:");
		debug_write_line("\tmt_length_: " + std::to_string(io_segment_header_.mt_length_));
		debug_write_line("\tmt_play_start_: " + std::to_string(io_segment_header_.mt_play_start_));
		debug_write_line("\trt_length_: " + std::to_string(io_segment_header_.rt_length_));

		debug_write_line("=============================================================================");

		debug_filebuf_.close();
	}
#endif // LTJS_DEBUG_DMUSIC_SEGMENT_DUMP_STRUCTURE

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

DMusicSegment::~DMusicSegment()
{
}

bool DMusicSegment::open(
	const std::string& file_name)
{
	return pimpl_->api_open(file_name);
}

void DMusicSegment::close()
{
	pimpl_->api_close();
}

const std::string& DMusicSegment::get_error_message() const
{
	return pimpl_->api_get_error_message();
}


}; // ltjs
