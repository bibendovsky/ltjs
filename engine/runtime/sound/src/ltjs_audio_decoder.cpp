#include "ltjs_audio_decoder.h"
#include <algorithm>
#include <utility>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
} // extern "C"

#include "bibendovsky_spul_wave_format_tag.h"


namespace ltjs
{


struct AudioDecoder::Impl
{
	using AVInputFormatPtr = AVInputFormat*;


	enum class State
	{
		none,
		read_packet,
		send_packet,
		receive_frame,
		convert_samples,
		output_frame,
		rewind,
		is_failed,
		completed,
	}; // State


	static constexpr int dst_bit_depth = 16;
	static constexpr AVSampleFormat ff_dst_sample_format = AV_SAMPLE_FMT_S16;
	static constexpr int max_ff_io_buffer_size = 4096;


	Impl() :
		ff_io_context_{},
		ff_io_buffer_{},
		ff_format_context_{},
		ff_codec_{},
		ff_codec_context_{},
		ff_frame_{},
		ff_swr_context_{},
		ff_stream_index_{},
		state_{},
		stream_ptr_{},
		channel_count_{},
		sample_rate_{},
		sample_size_{},
		sample_count_{},
		frame_size_{},
		frame_offset_{},
		decoded_size_{},
		decoded_offset_{},
		is_draining_{},
		is_flushing_{}
	{
	}

	Impl(
		const Impl& that) = delete;

	Impl& operator=(
		const Impl& that) = delete;

	~Impl()
	{
		close();
	}

	bool open(
		ul::Stream* stream_ptr)
	{
		close();

		if (!stream_ptr || !stream_ptr->is_open() || !stream_ptr->is_readable() || !stream_ptr->is_seekable())
		{
			return false;
		}

		if (!initialize_io(stream_ptr))
		{
			close();
			return false;
		}

		if (!open())
		{
			close();
			return false;
		}

		return true;
	}

	void close()
	{
		::avcodec_free_context(&ff_codec_context_);
		::avformat_close_input(&ff_format_context_);
		::av_frame_free(&ff_frame_);

		const auto free_buffer = (ff_io_context_ == nullptr);

		::av_freep(&ff_io_context_);

		if (free_buffer)
		{
			::av_freep(&ff_io_buffer_);
		}
		else
		{
			ff_io_buffer_ = nullptr;
		}

		::swr_free(&ff_swr_context_);

		ff_codec_ = nullptr;
		ff_codec_context_ = nullptr;
		state_ = State::none;
		stream_ptr_ = nullptr;
		channel_count_ = 0;
		sample_size_ = 0;
		sample_count_ = 0;
		sample_rate_ = 0;
		is_draining_ = false;
		is_flushing_ = false;
	}

	int decode(
		void* buffer,
		const int buffer_size)
	{
		if (!is_open() || is_failed() || !buffer || buffer_size < 0)
		{
			return -1;
		}

		if (buffer_size == 0)
		{
			return 0;
		}

		auto is_buffer_filled = false;
		auto decoded_size = 0;
		auto dst_buffer = static_cast<std::uint8_t*>(buffer);

		auto ff_packet = AVPacket{};
		::av_init_packet(&ff_packet);

		while (!is_buffer_filled)
		{
			switch (state_)
			{
			case State::read_packet:
			{
				const auto ff_result = ::av_read_frame(ff_format_context_, &ff_packet);

				if (ff_result == 0)
				{
					if (ff_packet.stream_index == ff_stream_index_)
					{
						state_ = State::send_packet;
					}
				}
				else if (ff_result == AVERROR_EOF)
				{
					if (ff_packet.stream_index == ff_stream_index_)
					{
						is_draining_ = true;
						state_ = State::send_packet;
					}
				}
				else
				{
					state_ = State::is_failed;
				}

				break;
			}

			case State::send_packet:
			{
				const auto ff_result = ::avcodec_send_packet(ff_codec_context_, is_draining_ ? nullptr : &ff_packet);

				if (ff_result == 0)
				{
					state_ = State::receive_frame;
				}
				else if (ff_result == AVERROR(EAGAIN))
				{
					state_ = (is_draining_ ? State::is_failed : State::read_packet);
				}
				else if (ff_result == AVERROR_EOF)
				{
					state_ = State::completed;
				}
				else
				{
					state_ = State::is_failed;
				}

				break;
			}

			case State::receive_frame:
			{
				const auto ff_result = ::avcodec_receive_frame(ff_codec_context_, ff_frame_);

				if (ff_result == 0)
				{
					if (has_converter())
					{
						state_ = State::convert_samples;
					}
					else
					{
						frame_offset_ = 0;
						frame_size_ = ff_frame_->nb_samples * sample_size_;
						state_ = State::output_frame;
					}
				}
				else if (ff_result ==  AVERROR(EAGAIN))
				{
					state_ = State::read_packet;
				}
				else if (ff_result == AVERROR_EOF)
				{
					if (has_converter())
					{
						is_flushing_ = true;
						state_ = State::convert_samples;
					}
					else
					{
						state_ = State::completed;
					}
				}
				else
				{
					state_ = State::is_failed;
				}

				break;
			}

			case State::convert_samples:
			{
				auto max_src_count = is_flushing_ ? 0 : ff_frame_->nb_samples;

				const auto max_dst_count = (buffer_size - decoded_size) / sample_size_;

				if (max_dst_count == 0)
				{
					is_buffer_filled = true;

					if (decoded_offset_ == decoded_size_)
					{
						state_ = State::completed;
					}

					break;
				}

				std::uint8_t* dst_buffers[] = { &dst_buffer[decoded_size] };

				const auto converted_count = ::swr_convert(
					ff_swr_context_,
					dst_buffers,
					max_dst_count,
					is_flushing_ ? nullptr : const_cast<const std::uint8_t**>(ff_frame_->data),
					max_src_count);

				if (converted_count > 0)
				{
					const auto converted_size = converted_count * sample_size_;

					max_src_count = 0;
					decoded_size += converted_size;
					decoded_offset_ += converted_size;

					if (!is_flushing_)
					{
						state_ = State::receive_frame;
					}
				}
				else if (converted_count == 0)
				{
					state_ = (is_flushing_ ? State::completed : State::receive_frame);
				}
				else
				{
					state_ = State::is_failed;
				}

				break;
			}

			case State::output_frame:
			{
				if (frame_offset_ != frame_size_)
				{
					const auto copy_size = std::min(frame_size_ - frame_offset_, buffer_size - decoded_size);

					if (copy_size > 0)
					{
						std::uninitialized_copy_n(
							&ff_frame_->data[0][frame_offset_],
							copy_size,
							&dst_buffer[decoded_size]);

						decoded_size += copy_size;
						frame_offset_ += copy_size;
						decoded_offset_ += copy_size;
					}
					else
					{
						is_buffer_filled = true;

						if (decoded_offset_ == decoded_size_)
						{
							state_ = State::completed;
						}
					}
				}
				else
				{
					state_ = State::receive_frame;
				}

				break;
			}


			case State::rewind:
				static_cast<void>(rewind_internal());
				break;


			case State::is_failed:
			case State::completed:
				is_buffer_filled = true;
				break;
			}
		}

		::av_packet_unref(&ff_packet);

		return decoded_size;
	}

	bool rewind()
	{
		if (!is_open() || is_failed())
		{
			return false;
		}

		return rewind_internal();
	}

	bool set_position(
		const int sample_offset)
	{
		if (!is_open() || is_failed() || sample_offset < 0 || sample_offset >= sample_count_)
		{
			return false;
		}

		if (sample_count_ == 0)
		{
			return true;
		}

		if (sample_offset == 0)
		{
			return rewind_internal();
		}

		::avcodec_flush_buffers(ff_codec_context_);

		const auto start_timestamp = normalize_pts(ff_format_context_->start_time);
		const auto time_base = ff_format_context_->streams[ff_stream_index_]->time_base;
		const auto sample_timestamp = (sample_offset * time_base.den) / (sample_rate_ * time_base.num);
		const auto timestamp = start_timestamp + sample_timestamp;

		const auto ff_result = ::av_seek_frame(
			ff_format_context_,
			ff_stream_index_,
			timestamp,
			AVSEEK_FLAG_FRAME);

		if (ff_result < 0)
		{
			state_ = State::is_failed;
			return false;
		}

		state_ = State::read_packet;
		is_draining_ = false;
		is_flushing_ = false;
		decoded_offset_ = sample_offset * sample_size_;

		return true;
	}

	bool is_open() const
	{
		return state_ != State::none;
	}

	bool is_failed() const
	{
		return state_ == State::is_failed;
	}

	bool is_pcm() const
	{
		return
			is_open() &&
			(
				ff_codec_context_->codec_id == AV_CODEC_ID_PCM_S16LE ||
				ff_codec_context_->codec_id == AV_CODEC_ID_PCM_U8
			);
	}

	bool is_ima_adpcm() const
	{
		return is_open() && ff_codec_context_->codec_id == AV_CODEC_ID_ADPCM_IMA_WAV;
	}

	bool is_mp3() const
	{
		return is_open() && ff_codec_context_->codec_id == AV_CODEC_ID_MP3;
	}

	int get_channel_count() const
	{
		return channel_count_;
	}

	int get_bit_depth() const
	{
		return dst_bit_depth;
	}

	int get_sample_rate() const
	{
		return sample_rate_;
	}

	ul::WaveFormatEx get_wave_format_ex() const
	{
		if (!is_open())
		{
			return {};
		}


		auto result = ul::WaveFormatEx{};
		result.tag_ = ul::WaveFormatTag::pcm;
		result.channel_count_ = static_cast<std::uint16_t>(channel_count_);
		result.bit_depth_ = static_cast<std::uint16_t>(dst_bit_depth);
		result.sample_rate_ = static_cast<std::uint32_t>(sample_rate_);
		result.block_align_ = static_cast<std::uint16_t>(sample_size_);
		result.avg_bytes_per_sec_ = result.block_align_ * result.sample_rate_;

		return result;
	}

	int get_sample_size() const
	{
		return sample_size_;
	}

	int get_sample_count() const
	{
		return sample_count_;
	}

	int get_data_size() const
	{
		return sample_size_ * sample_count_;
	}

	int get_decoded_size() const
	{
		return decoded_offset_;
	}

	static void initialize_current_thread()
	{
		::av_log_set_level(AV_LOG_QUIET);
		::av_register_all();

		get_ff_wav_input_format() = ::av_find_input_format("wav");
	}

	static int io_read_function_proxy(
		void* user_data,
		std::uint8_t* buffer,
		const int buffer_size)
	{
		auto& stream = *static_cast<Impl*>(user_data)->stream_ptr_;

		const auto read_result = stream.read(buffer, buffer_size);

		if (read_result < 0)
		{
			return AVERROR_UNKNOWN;
		}

		return read_result;
	}

	static std::int64_t io_seek_function_proxy(
		void* user_data,
		const std::int64_t offset,
		const int whence)
	{
		auto& stream = *static_cast<Impl*>(user_data)->stream_ptr_;

		if ((whence & AVSEEK_SIZE) != 0)
		{
			return stream.get_size();
		}

		auto api_whence = ul::Stream::Origin::none;

		switch (whence)
		{
		case SEEK_SET:
			api_whence = ul::Stream::Origin::begin;
			break;

		case SEEK_CUR:
			api_whence = ul::Stream::Origin::current;
			break;

		case SEEK_END:
			api_whence = ul::Stream::Origin::end;
			break;

		default:
			return -1;
		}


		return stream.set_position(offset, api_whence);
	}

	bool initialize_io(
		ul::Stream* stream_ptr)
	{
		ff_io_buffer_ = static_cast<std::uint8_t*>(::av_malloc(max_ff_io_buffer_size));

		if (!ff_io_buffer_)
		{
			return false;
		}

		ff_io_context_ = ::avio_alloc_context(
			ff_io_buffer_,
			max_ff_io_buffer_size,
			0,
			this,
			io_read_function_proxy,
			nullptr,
			io_seek_function_proxy);

		if (!ff_io_context_)
		{
			return false;
		}

		stream_ptr_ = stream_ptr;

		return true;
	}

	bool open()
	{
		auto ff_result = 0;
		auto has_data = false;
		auto is_succeed = false;

		ff_format_context_ = ::avformat_alloc_context();

		if (!ff_format_context_)
		{
			return false;
		}

		ff_format_context_->pb = ff_io_context_;

		ff_result = ::avformat_open_input(
			&ff_format_context_,
			nullptr,
			get_ff_wav_input_format(),
			nullptr);

		if (ff_result != 0)
		{
			return false;
		}

		const auto stream_count = static_cast<int>(ff_format_context_->nb_streams);

		using AVStreamPtr = AVStream*;
		auto stream = AVStreamPtr{};

		ff_stream_index_ = -1;

		if (stream_count > 0)
		{
			for (auto i = 0; i < stream_count; ++i)
			{
				stream = ff_format_context_->streams[i];
				const auto& codecpar = *stream->codecpar;

				if (codecpar.codec_type == AVMEDIA_TYPE_AUDIO && is_codec_id_valid(codecpar.codec_id))
				{
					ff_stream_index_ = i;
					break;
				}
				else
				{
					stream->discard = AVDISCARD_ALL;
				}
			}
		}

		if (ff_stream_index_ >= 0)
		{
			auto duration = normalize_pts(stream->duration);
			auto start_time = normalize_pts(stream->start_time);

			has_data = ((duration - start_time) > 0);

			if (!has_data)
			{
				ff_result = ::avformat_find_stream_info(ff_format_context_, nullptr);

				if (ff_result >= 0)
				{
					duration = normalize_pts(stream->duration);
					start_time = normalize_pts(stream->start_time);

					has_data = ((duration - start_time) > 0);
				}
				else
				{
					return false;
				}
			}

			ff_codec_context_ = ::avcodec_alloc_context3(nullptr);

			if (!ff_codec_context_)
			{
				return false;
			}

			ff_result = ::avcodec_parameters_to_context(ff_codec_context_, stream->codecpar);

			if (ff_result < 0)
			{
				return false;
			}
		}

		if (has_data)
		{
			ff_codec_ = ::avcodec_find_decoder(ff_codec_context_->codec_id);

			if (!ff_codec_)
			{
				return false;
			}

			ff_result = ::avcodec_open2(ff_codec_context_, ff_codec_, nullptr);

			if (ff_result != 0)
			{
				return false;
			}
		}

		channel_count_ = ff_codec_context_->channels;
		sample_size_ = channel_count_ * (dst_bit_depth / 8);

		sample_rate_ = ff_codec_context_->sample_rate;

		const auto ff_src_sample_format = ff_codec_context_->sample_fmt;

		auto need_converter = false;

		decoded_size_ = 0;
		decoded_offset_ = 0;

		if (has_data)
		{
			auto sample_count = stream->duration - normalize_pts(stream->start_time);
			sample_count *= sample_rate_;
			sample_count /= stream->time_base.den;
			sample_count += sample_size_ - 1;
			sample_count /= sample_size_;
			sample_count *= sample_size_;
			sample_count_ = static_cast<int>(sample_count);

			decoded_size_ = sample_count_ * sample_size_;

			need_converter = (ff_src_sample_format != ff_dst_sample_format);
		}

		if (need_converter)
		{
			const auto channel_layout = ::av_get_default_channel_layout(channel_count_);

			ff_swr_context_ = ::swr_alloc_set_opts(
				ff_swr_context_,
				channel_layout,
				ff_dst_sample_format,
				sample_rate_,
				channel_layout,
				ff_src_sample_format,
				sample_rate_,
				0,
				nullptr);

			if (!ff_swr_context_)
			{
				return false;
			}

			//
			ff_result = ::swr_init(ff_swr_context_);

			if (ff_result != 0)
			{
				return false;
			}
		}

		if (has_data)
		{
			ff_frame_ = ::av_frame_alloc();

			if (!ff_frame_)
			{
				return false;
			}
		}

		state_ = State::read_packet;

		is_succeed = true;

		return true;
	}

	bool has_converter() const
	{
		return ff_swr_context_ != nullptr;
	}

	bool rewind_internal()
	{
		const auto start_timestamp = normalize_pts(ff_format_context_->start_time);

		::avcodec_flush_buffers(ff_codec_context_);

		const auto ff_result = ::av_seek_frame(ff_format_context_, ff_stream_index_, start_timestamp, AVSEEK_FLAG_FRAME);

		if (ff_result < 0)
		{
			state_ = State::is_failed;
			return false;
		}

		state_ = State::read_packet;
		is_draining_ = false;
		is_flushing_ = false;
		decoded_offset_ = 0;

		return true;
	}

	static bool is_codec_id_valid(
		const AVCodecID codec_id)
	{
		switch (codec_id)
		{
		case AV_CODEC_ID_PCM_U8:
		case AV_CODEC_ID_PCM_S16LE:
		case AV_CODEC_ID_ADPCM_IMA_WAV:
		case AV_CODEC_ID_MP3:
			return true;

		default:
			return false;
		}
	}

	static std::int64_t normalize_pts(
		const std::int64_t pts)
	{
		return pts != AV_NOPTS_VALUE ? pts : 0;
	}

	static AVInputFormatPtr& get_ff_wav_input_format()
	{
		static AVInputFormatPtr result;
		return result;
	}


	AVIOContext* ff_io_context_;
	std::uint8_t* ff_io_buffer_;
	AVFormatContext* ff_format_context_;
	AVCodec* ff_codec_;
	AVCodecContext* ff_codec_context_;
	AVFrame* ff_frame_;
	SwrContext* ff_swr_context_;
	int ff_stream_index_;
	State state_;
	ul::Stream* stream_ptr_;
	int channel_count_;
	int sample_rate_;
	int sample_size_;
	int sample_count_;
	int frame_size_;
	int frame_offset_;
	int decoded_size_;
	int decoded_offset_;
	bool is_draining_;
	bool is_flushing_;
}; // AudioDecoder::Impl


constexpr int AudioDecoder::Impl::dst_bit_depth;
constexpr AVSampleFormat AudioDecoder::Impl::ff_dst_sample_format;
constexpr int AudioDecoder::Impl::max_ff_io_buffer_size;


AudioDecoder::AudioDecoder() :
	pimpl_{new Impl{}}
{
}

AudioDecoder::AudioDecoder(
	ul::Stream* stream_ptr)
	:
	AudioDecoder{}
{
	static_cast<void>(open(stream_ptr));
}

AudioDecoder::AudioDecoder(
	AudioDecoder&& that)
	:
	pimpl_{std::move(that.pimpl_)}
{
}

AudioDecoder::~AudioDecoder()
{
}

bool AudioDecoder::open(
	ul::Stream* stream_ptr)
{
	return pimpl_->open(stream_ptr);
}

void AudioDecoder::close()
{
	pimpl_->close();
}

int AudioDecoder::decode(
	void* buffer,
	const int buffer_size)
{
	return pimpl_->decode(buffer, buffer_size);
}

bool AudioDecoder::rewind()
{
	return pimpl_->rewind();
}

bool AudioDecoder::set_position(
	const int sample_offset)
{
	return pimpl_->set_position(sample_offset);
}

bool AudioDecoder::is_open() const
{
	return pimpl_->is_open();
}

bool AudioDecoder::is_failed() const
{
	return pimpl_->is_failed();
}

bool AudioDecoder::is_pcm() const
{
	return pimpl_->is_pcm();
}

bool AudioDecoder::is_ima_adpcm() const
{
	return pimpl_->is_ima_adpcm();
}

bool AudioDecoder::is_mp3() const
{
	return pimpl_->is_mp3();
}

int AudioDecoder::get_channel_count() const
{
	return pimpl_->get_channel_count();
}

int AudioDecoder::get_bit_depth() const
{
	return pimpl_->get_bit_depth();
}

int AudioDecoder::get_sample_rate() const
{
	return pimpl_->get_sample_rate();
}

int AudioDecoder::get_sample_size() const
{
	return pimpl_->get_sample_size();
}

ul::WaveFormatEx AudioDecoder::get_wave_format_ex() const
{
	return pimpl_->get_wave_format_ex();
}

int AudioDecoder::get_sample_count() const
{
	return pimpl_->get_sample_count();
}

int AudioDecoder::get_data_size() const
{
	return pimpl_->get_data_size();
}

int AudioDecoder::get_decoded_size() const
{
	return pimpl_->get_decoded_size();
}

void AudioDecoder::initialize_current_thread()
{
	Impl::initialize_current_thread();
}


} // ltjs
