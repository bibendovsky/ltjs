#include "bdefs.h"
#include "ltjs_dmusic_segment.h"
#include <cstdint>
#include <utility>
#include <vector>
#include "bibendovsky_spul_enum_flags.h"
#include "bibendovsky_spul_endian.h"
#include "bibendovsky_spul_four_cc.h"
#include "bibendovsky_spul_memory_stream.h"
#include "bibendovsky_spul_riff_four_ccs.h"
#include "bibendovsky_spul_riff_reader.h"
#include "bibendovsky_spul_scope_guard.h"
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
		io_header_{}
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
		io_header_{std::move(that.io_header_)}
	{
	}

	~Impl()
	{
		close_internal();
	}


	bool api_open(
		const std::string& file_name)
	{
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
	using Buffer = std::vector<std::uint8_t>;


	struct IoFlags8 :
		ul::EnumFlagsT<std::uint32_t>
	{
		IoFlags8(const Value flags = none) :
			EnumFlagsT<std::uint32_t>{flags}
		{
		}

		enum : Value
		{
			is_ref_time = 0B0001,
		}; // Value
	}; // IoFlags8

#pragma pack(push, 1)
	struct IoHeader8
	{
		static constexpr auto packed_size = 40;


		std::uint32_t repeat_count_;
		std::int32_t length_;
		std::int32_t play_start_;
		std::int32_t loop_start_;
		std::int32_t loop_end_;
		std::uint32_t resolution_;
		std::int64_t ref_length_;
		IoFlags8 flags_;
		std::uint32_t reserved_;


		IoHeader8()
		{
			static_assert(packed_size == sizeof(IoHeader8), "Invalid structure size.");
		}

		bool read(
			ul::Stream& stream)
		{
			if (stream.read(this, packed_size) != packed_size)
			{
				return false;
			}

			ul::Endian::little_i(repeat_count_);
			ul::Endian::little_i(length_);
			ul::Endian::little_i(play_start_);
			ul::Endian::little_i(loop_start_);
			ul::Endian::little_i(loop_end_);
			ul::Endian::little_i(resolution_);
			ul::Endian::little_i(ref_length_);
			ul::Endian::little_i(flags_);
			ul::Endian::little_i(reserved_);

			return true;
		}
	}; // IoHeader8
#pragma pack(pop)


	std::string error_message_;
	Buffer file_image_;
	ul::MemoryStream memory_stream_;
	ul::RiffReader riff_reader_;
	IoHeader8 io_header_;


	bool read_io_header()
	{
		if (!riff_reader_.find_and_descend(ul::FourCc{"segh"}))
		{
			error_message_ = "No header.";
			return false;
		}


		auto io_header_chunk = riff_reader_.get_current_chunk();

		if (io_header_chunk.size_ < IoHeader8::packed_size)
		{
			error_message_ = "Invalid header size.";
			return false;
		}

		auto io_header_stream = io_header_chunk.data_stream_;

		if (!io_header_.read(io_header_stream))
		{
			error_message_ = "Failed to read a header.";
			return false;
		}

		if (!riff_reader_.ascend())
		{
			error_message_ = "RIFF error.";
			return false;
		}

		return true;
	}

	bool read_track(
		ul::RiffReader& riff_reader)
	{
		if (!riff_reader.find_and_descend(ul::FourCc{"trkh"}))
		{
			error_message_ = "No track header.";
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

			if (!riff_reader_.find_and_descend(ul::FourCc{"trkh"}))
			{
				error_message_ = "No track header.";
				return false;
			}

			if (!riff_reader_.ascend())
			{
				error_message_ = "RIFF error.";
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

		if (!read_io_header())
		{
			return false;
		}

		if (!read_tracks())
		{
			return false;
		}

		return false;
	}

	void close_internal()
	{
		error_message_.clear();
		file_image_.clear();
		riff_reader_.close();
		memory_stream_.close();
		io_header_ = {};
	}
}; // DMusicSegment::Impl


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
