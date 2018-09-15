#include "bdefs.h"

#include "ltjs_fmv_player.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <iterator>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
} // extern "C"


namespace ltjs
{


namespace
{


struct FmvPlayerDetail
{
	static constexpr auto audio_dst_channel_count = 2;

	static constexpr auto audio_dst_bit_depth = 16;
	static constexpr auto audio_dst_byte_depth = audio_dst_bit_depth / 8;

	static constexpr auto audio_dst_sample_size = audio_dst_channel_count * audio_dst_byte_depth;

	static constexpr auto default_sleep_delay_ms = 5;
}; // FmvPlayerDetail


} // namespace


class FmvPlayer::Impl
{
public:
	using IsCancelledFunc = bool (*)();

	using AudioPresentFunc = void (*)(
		const void* data,
		const int data_size);

	using VideoPresentFunc = void (*)(
		const void* data,
		const int width,
		const int height);


	Impl();

	Impl(
		const Impl& that) = delete;

	Impl& operator=(
		const Impl& that) = delete;

	~Impl();


	bool initialize(
		const InitializeParam& param);

	void uninitialize();

	bool is_initialized() const;


	int get_sample_rate() const;

	int get_audio_buffer_size() const;

	int get_width() const;

	int get_height() const;

	bool has_audio() const;

	bool present();

	bool present_frame();

	bool is_presentation_finished() const;

	static void initialize_current_thread();


private:
	static constexpr int max_ff_io_buffer_size = 4096;
	static constexpr int max_ff_video_planes = 4;
	static constexpr int pixel_size = 4; // ARGB/BGRA


	using AVInputFormatPtr = AVInputFormat*;
	using FfPointers = std::array<std::uint8_t*, max_ff_video_planes>;
	using FfStrides = std::array<int, max_ff_video_planes>;


	enum class FfState
	{
		none,
		read,
		send,
		receive,
		decode,
		flush,
		failed,
		finished,
	}; // FfState


	struct Frame
	{
		using Data = std::vector<std::uint8_t>;


		std::int64_t pts_ms_;
		int data_size_;
		Data data_;


		Data& initialize_data(
			const int max_data_size);


		static void swap(
			Frame& a,
			Frame& b);
	}; // Frame

	struct PresentFrameContext
	{
		bool is_initialized_;
		int frame_count_to_remove_;
		std::int64_t frame_rate_ms_;
	}; // PresentFrameContext


	using Mutex = std::mutex;
	using MutexGuard = std::lock_guard<Mutex>;
	using UniqueLock = std::unique_lock<Mutex>;
	using Thread = std::thread;
	using CondVar = std::condition_variable;
	using Frames = std::deque<Frame>;
	using AudioBuffer = std::vector<std::uint8_t>;
	using MtPtsMs = std::atomic<std::int64_t>;


	AVIOContext* ff_io_context_;
	std::uint8_t* ff_io_buffer_;
	AVFormatContext* ff_format_context_;
	AVCodec* ff_audio_codec_;
	AVCodec* ff_video_codec_;
	AVCodecContext* ff_codec_context_;
	AVCodecContext* ff_audio_codec_context_;
	AVCodecContext* ff_video_codec_context_;
	AVPacket ff_packet_;
	AVFrame* ff_frame_;
	AVFrame* ff_audio_frame_;
	AVFrame* ff_video_frame_;
	SwrContext* ff_swr_context_;
	SwsContext* ff_sws_context_;
	int ff_audio_stream_index_;
	int ff_video_stream_index_;
	bool ff_is_audio_frame_;
	void* ff_io_user_data_;
	IoReadFunction ff_io_read_function_;
	IoSeekFunction ff_io_seek_function_;
	FfState ff_state_;
	bool ff_is_await_;
	bool ff_is_finished_;
	int ff_decoded_sample_count_;
	int ff_dst_sample_rate_;


	bool is_initialized_;
	bool is_presentation_finished_;
	bool is_stop_threads_;
	bool is_decoder_worker_finished_;
	bool is_audio_worker_finished_;
	bool is_present_all_frames_;
	bool is_present_one_frame_;
	InitializeParam initialize_param_;
	MtPtsMs mt_pts_ms_;
	Mutex mt_audio_mutex_;
	Mutex mt_video_mutex_;
	Mutex mt_preload_mutex_;
	CondVar mt_preload_cv_;
	bool mt_preload_flag_;
	Thread mt_clock_thread_;
	Thread mt_decoder_thread_;
	Thread mt_audio_thread_;
	Frames mt_audio_frames_;
	Frames mt_video_frames_;
	Frame audio_frame_buffer_;
	int audio_buffer_size_;
	PresentFrameContext present_frame_context_;


	bool initialize_internal(
		const InitializeParam& param);

	void handle_audio_frame_buffer();

	void handle_audio_buffer_flush();

	void handle_read_state();

	void handle_send_state();

	void handle_receive_state();

	void handle_decode_audio_state();

	void handle_decode_video_state();

	void handle_decode_state();

	void handle_flush_state();

	bool decode();

	void get_frame_rate_internal(
		int& numerator,
		int& denominator) const;

	bool get_frame_rate(
		int& numerator,
		int& denominator) const;

	static int io_read_function_proxy(
		void* user_data,
		std::uint8_t* buffer,
		const int buffer_size);

	static std::int64_t io_seek_function_proxy(
		void* user_data,
		const std::int64_t offset,
		const int whence);

	bool initialize_io(
		const InitializeParam& param);

	bool initialize_decoder(
		const InitializeParam& param);

	static std::int64_t normalize_pts(
		const std::int64_t pts);

	static AVInputFormatPtr& get_ff_bik_input_format();


	void clock_worker();

	void decoder_worker();

	void audio_worker();

	void stop_workers();

	void wait_for_preload();

	void notify_preload(
		bool& flag);

	std::int64_t ms_to_size(
		const std::int64_t milliseconds);

	std::int64_t size_to_ms(
		const std::int64_t size);

	bool present_frame_initialize();
}; // FmvPlayer::Impl


FmvPlayer::Impl::Frame::Data& FmvPlayer::Impl::Frame::initialize_data(
	const int max_data_size)
{
	data_.resize(max_data_size);
	return data_;
}

void FmvPlayer::Impl::Frame::swap(
	Frame& a,
	Frame& b)
{
	std::swap(a.pts_ms_, b.pts_ms_);
	std::swap(a.data_size_, b.data_size_);
	std::swap(a.data_, b.data_);
}

bool FmvPlayer::InitializeParam::validate() const
{
	if (!io_read_func_ || !io_seek_func_)
	{
		return false;
	}

	if (!video_present_func_)
	{
		return false;
	}

	if (!is_ignore_audio_)
	{
		if (dst_sample_rate_ < 0)
		{
			return false;
		}

		const auto sample_size = FmvPlayerDetail::audio_dst_sample_size;

		if (audio_buffer_size_ms_ <= 0)
		{
			return false;
		}

		if (audio_max_buffer_count_ <= 0)
		{
			return false;
		}

		if (!audio_present_func_ || !audio_get_free_buffer_count_func_)
		{
			return false;
		}
	}

	return true;
}

FmvPlayer::Impl::Impl()
	:
	ff_io_context_{},
	ff_io_buffer_{},
	ff_format_context_{},
	ff_audio_codec_{},
	ff_video_codec_{},
	ff_codec_context_{},
	ff_audio_codec_context_{},
	ff_video_codec_context_{},
	ff_packet_{},
	ff_frame_{},
	ff_audio_frame_{},
	ff_video_frame_{},
	ff_swr_context_{},
	ff_sws_context_{},
	ff_audio_stream_index_{},
	ff_video_stream_index_{},
	ff_is_audio_frame_{},
	ff_io_user_data_{},
	ff_io_read_function_{},
	ff_io_seek_function_{},
	ff_state_{},
	ff_is_await_{},
	ff_is_finished_{},
	ff_decoded_sample_count_{},
	ff_dst_sample_rate_{},
	is_initialized_{},
	is_presentation_finished_{},
	is_stop_threads_{},
	is_decoder_worker_finished_{},
	is_audio_worker_finished_{},
	is_present_all_frames_{},
	is_present_one_frame_{},
	initialize_param_{},
	mt_pts_ms_{},
	mt_audio_mutex_{},
	mt_video_mutex_{},
	mt_preload_mutex_{},
	mt_preload_cv_{},
	mt_preload_flag_{},
	mt_decoder_thread_{},
	mt_audio_thread_{},
	mt_audio_frames_{},
	mt_video_frames_{},
	audio_frame_buffer_{},
	audio_buffer_size_{},
	present_frame_context_{}
{
}

FmvPlayer::Impl::~Impl()
{
	uninitialize();
}

bool FmvPlayer::Impl::initialize(
	const InitializeParam& param)
{
	uninitialize();

	if (!initialize_internal(param))
	{
		uninitialize();
		return false;
	}

	is_initialized_ = true;

	return true;
}

void FmvPlayer::Impl::uninitialize()
{
	::av_packet_unref(&ff_packet_);
	ff_packet_ = {};

	::avcodec_free_context(&ff_audio_codec_context_);
	::avcodec_free_context(&ff_video_codec_context_);
	::avformat_close_input(&ff_format_context_);
	::av_frame_free(&ff_audio_frame_);
	::av_frame_free(&ff_video_frame_);

	auto is_free_io_buffer = true;

	if (ff_io_context_ && ff_io_context_->buffer)
	{
		if (ff_io_context_->buffer != ff_io_buffer_)
		{
			is_free_io_buffer = false;
			::av_freep(&ff_io_context_->buffer);
		}
	}

	::avio_context_free(&ff_io_context_);

	if (is_free_io_buffer)
	{
		::av_freep(&ff_io_buffer_);
	}
	else
	{
		ff_io_buffer_ = nullptr;
	}

	::swr_free(&ff_swr_context_);

	::sws_freeContext(ff_sws_context_);
	ff_sws_context_ = nullptr;

	ff_frame_ = nullptr;
	ff_audio_codec_ = nullptr;
	ff_video_codec_ = nullptr;
	ff_codec_context_ = nullptr;
	ff_audio_codec_context_ = nullptr;
	ff_video_codec_context_ = nullptr;
	ff_is_audio_frame_ = false;
	ff_io_user_data_ = nullptr;
	ff_io_read_function_ = nullptr;
	ff_io_seek_function_ = nullptr;
	ff_state_ = FfState::none;
	ff_is_await_ = false;
	ff_is_finished_ = false;
	ff_decoded_sample_count_ = {};
	ff_dst_sample_rate_ = {};

	stop_workers();

	is_initialized_ = false;
	is_presentation_finished_ = false;
	is_stop_threads_ = false;
	is_decoder_worker_finished_ = false;
	is_audio_worker_finished_ = false;
	is_present_all_frames_ = false;
	is_present_one_frame_ = false;
	initialize_param_ = {};
	mt_pts_ms_.store(0);
	mt_preload_flag_ = {};
	mt_decoder_thread_ = {};
	mt_audio_thread_ = {};
	mt_audio_frames_ = {};
	mt_video_frames_ = {};
	audio_frame_buffer_ = {};
	audio_buffer_size_ = {};
	present_frame_context_ = {};
}

bool FmvPlayer::Impl::is_initialized() const
{
	return is_initialized_;
}

bool FmvPlayer::Impl::initialize_internal(
	const InitializeParam& param)
{
	if (!param.validate())
	{
		return false;
	}

	if (!initialize_io(param))
	{
		uninitialize();
		return false;
	}

	if (!initialize_decoder(param))
	{
		uninitialize();
		return false;
	}

	return true;
}

void FmvPlayer::Impl::handle_audio_frame_buffer()
{
	const auto buffer_count = audio_frame_buffer_.data_size_ / audio_buffer_size_;

	if (buffer_count == 0)
	{
		return;
	}

	const auto buffer_sample_count = audio_buffer_size_ / FmvPlayerDetail::audio_dst_sample_size;

	auto data_offset = 0;

	for (auto i = 0; i < buffer_count; ++i)
	{
		const auto pts_ms = (1000i64 * ff_decoded_sample_count_) / ff_dst_sample_rate_;

		auto frame = Frame{};

		frame.pts_ms_ = pts_ms;
		frame.initialize_data(audio_buffer_size_);
		frame.data_size_ = audio_buffer_size_;

		std::uninitialized_copy_n(
			audio_frame_buffer_.data_.cbegin() + data_offset,
			audio_buffer_size_,
			frame.data_.begin());

		data_offset += audio_buffer_size_;

		ff_decoded_sample_count_ += buffer_sample_count;

		{
			MutexGuard mutex_guard{mt_audio_mutex_};

			mt_audio_frames_.emplace_back();
			Frame::swap(mt_audio_frames_.back(), frame);
		}
	}

	const auto buffer_remain = audio_frame_buffer_.data_size_ - data_offset;

	if (buffer_remain == 0)
	{
		audio_frame_buffer_.data_size_ = 0;
		return;
	}

	const auto new_offset = audio_frame_buffer_.data_size_ - buffer_remain;

	std::uninitialized_copy_n(
		&audio_frame_buffer_.data_[new_offset],
		buffer_remain,
		audio_frame_buffer_.data_.data());

	audio_frame_buffer_.data_size_ = buffer_remain;
}

void FmvPlayer::Impl::handle_audio_buffer_flush()
{
	handle_audio_frame_buffer();

	if (audio_frame_buffer_.data_size_ == 0)
	{
		return;
	}

	const auto fill_count = audio_buffer_size_ - audio_frame_buffer_.data_size_;

	if (fill_count > 0)
	{
		std::uninitialized_fill_n(
			&audio_frame_buffer_.data_[audio_frame_buffer_.data_size_],
			fill_count,
			std::uint8_t{});

		audio_frame_buffer_.data_size_ = audio_buffer_size_;
	}

	handle_audio_frame_buffer();
}

void FmvPlayer::Impl::handle_read_state()
{
	::av_init_packet(&ff_packet_);

	const auto ff_result = ::av_read_frame(ff_format_context_, &ff_packet_);

	if (ff_result == 0)
	{
		if (ff_packet_.stream_index == ff_audio_stream_index_)
		{
			ff_is_audio_frame_ = true;
			ff_frame_ = ff_audio_frame_;
			ff_codec_context_ = ff_audio_codec_context_;
		}
		else if (ff_packet_.stream_index == ff_video_stream_index_)
		{
			ff_is_audio_frame_ = false;
			ff_frame_ = ff_video_frame_;
			ff_codec_context_ = ff_video_codec_context_;
		}
		else
		{
			::av_packet_unref(&ff_packet_);
			return;
		}

		ff_state_ = FfState::send;
	}
	else if (ff_result == AVERROR_EOF)
	{
		ff_state_ = FfState::flush;
	}
	else
	{
		ff_state_ = FfState::failed;
	}
}

void FmvPlayer::Impl::handle_send_state()
{
	const auto ff_result = ::avcodec_send_packet(ff_codec_context_, &ff_packet_);

	if (ff_result == 0)
	{
		ff_state_ = FfState::receive;
	}
	else if (ff_result == AVERROR(EAGAIN))
	{
		ff_state_ = FfState::receive;
	}
	else if (ff_result == AVERROR_EOF)
	{
		ff_state_ = FfState::flush;
	}
	else
	{
		ff_state_ = FfState::failed;
	}
}

void FmvPlayer::Impl::handle_receive_state()
{
	const auto ff_result = ::avcodec_receive_frame(ff_codec_context_, ff_frame_);

	if (ff_result == 0)
	{
		ff_state_ = FfState::decode;
	}
	else if (ff_result == AVERROR(EAGAIN))
	{
		ff_state_ = FfState::read;
		::av_packet_unref(&ff_packet_);
	}
	else if (ff_result == AVERROR_EOF)
	{
		ff_state_ = FfState::flush;
	}
	else
	{
		ff_state_ = FfState::failed;
	}
}

void FmvPlayer::Impl::handle_decode_audio_state()
{
	if (ff_swr_context_)
	{
		auto max_dst_sample_count = ::swr_get_out_samples(ff_swr_context_, ff_frame_->nb_samples);

		if (max_dst_sample_count < 0)
		{
			ff_state_ = FfState::failed;
			return;
		}

		if (max_dst_sample_count < ff_frame_->nb_samples)
		{
			max_dst_sample_count = ff_frame_->nb_samples;
		}

		const auto max_dst_size = max_dst_sample_count * FmvPlayerDetail::audio_dst_sample_size;

		const auto new_audio_buffer_size = max_dst_size + audio_frame_buffer_.data_size_;

		if (static_cast<int>(audio_frame_buffer_.data_.size()) < new_audio_buffer_size)
		{
			audio_frame_buffer_.data_.resize(new_audio_buffer_size);
		}

		std::uint8_t* dst_samples[] = {&audio_frame_buffer_.data_[audio_frame_buffer_.data_size_]};

		const auto dst_sample_count = ::swr_convert(
			ff_swr_context_,
			dst_samples,
			max_dst_sample_count,
			const_cast<const std::uint8_t**>(ff_frame_->data),
			ff_frame_->nb_samples);

		if (dst_sample_count >= 0)
		{
			const auto dst_size = FmvPlayerDetail::audio_dst_sample_size * dst_sample_count;

			audio_frame_buffer_.data_size_ += dst_size;

			handle_audio_frame_buffer();
		}
		else
		{
			ff_state_ = FfState::failed;
			return;
		}
	}
	else
	{
		const auto dst_size = FmvPlayerDetail::audio_dst_sample_size * ff_frame_->nb_samples;

		if (dst_size > 0)
		{
			if (static_cast<int>(audio_frame_buffer_.data_.size()) < dst_size)
			{
				audio_frame_buffer_.data_.resize(dst_size);
			}

			std::uninitialized_copy_n(
				ff_frame_->data[0],
				dst_size,
				&audio_frame_buffer_.data_[audio_frame_buffer_.data_size_]);

			audio_frame_buffer_.data_size_ += dst_size;

			handle_audio_frame_buffer();
		}
	}

	ff_is_await_ = true;
	ff_state_ = FfState::receive;
}

void FmvPlayer::Impl::handle_decode_video_state()
{
	const auto dst_stride = ff_video_codec_context_->width * pixel_size;
	const auto dst_size = ff_video_codec_context_->height * dst_stride;

	if (ff_sws_context_)
	{
		auto frame = Frame{};
		auto& frame_data = frame.initialize_data(dst_size);

		auto dst_pointers = FfPointers{};
		dst_pointers[0] = frame_data.data();

		auto dst_strides = FfStrides{};
		dst_strides[0] = dst_stride;

		const auto ff_result = ::sws_scale(
			ff_sws_context_,
			ff_frame_->data,
			ff_frame_->linesize,
			0,
			ff_video_codec_context_->height,
			dst_pointers.data(),
			dst_strides.data());

		if (ff_result != ff_video_codec_context_->height)
		{
			ff_state_ = FfState::failed;
			return;
		}

		mt_video_frames_.emplace_back();
		Frame::swap(mt_video_frames_.back(), frame);
	}
	else
	{
		mt_video_frames_.emplace_back();
		auto& frame = mt_video_frames_.back();
		auto& frame_data = frame.initialize_data(dst_size);

		std::uninitialized_copy_n(ff_frame_->data[0], dst_size, frame_data.begin());
	}

	auto& frame = mt_video_frames_.back();

	auto fr_num = 0;
	auto fr_den = 0;
	get_frame_rate_internal(fr_num, fr_den);

	const auto pts = normalize_pts(ff_frame_->pts);
	const auto pts_ms = (1000i64 * fr_den * pts) / fr_num;
	frame.pts_ms_ = pts_ms;

	frame.data_size_ = dst_size;

	ff_is_await_ = true;
	ff_state_ = FfState::receive;
}

void FmvPlayer::Impl::handle_decode_state()
{
	if (ff_is_audio_frame_)
	{
		handle_decode_audio_state();
	}
	else
	{
		handle_decode_video_state();
	}
}

void FmvPlayer::Impl::handle_flush_state()
{
	ff_state_ = FfState::finished;

	if (!ff_swr_context_)
	{
		return;
	}

	auto max_dst_sample_count = ::swr_get_out_samples(ff_swr_context_, 0);

	if (max_dst_sample_count < 0)
	{
		ff_state_ = FfState::failed;
		return;
	}

	if (max_dst_sample_count == 0)
	{
		ff_state_ = FfState::finished;
		handle_audio_buffer_flush();
		return;
	}

	const auto max_dst_size = max_dst_sample_count * FmvPlayerDetail::audio_dst_sample_size;

	if (static_cast<int>(audio_frame_buffer_.data_.size()) < max_dst_size)
	{
		audio_frame_buffer_.data_.resize(max_dst_size);
	}

	std::uint8_t* dst_samples[] = {&audio_frame_buffer_.data_[audio_frame_buffer_.data_size_]};

	const auto dst_sample_count = ::swr_convert(
		ff_swr_context_,
		dst_samples,
		max_dst_sample_count,
		nullptr,
		0);

	if (dst_sample_count < 0)
	{
		ff_state_ = FfState::failed;
		return;
	}

	if (dst_sample_count == 0)
	{
		handle_audio_buffer_flush();
		return;
	}

	const auto dst_size = dst_sample_count * FmvPlayerDetail::audio_dst_sample_size;
	audio_frame_buffer_.data_size_ += dst_size;

	handle_audio_buffer_flush();
}

bool FmvPlayer::Impl::decode()
{
	if (ff_is_finished_)
	{
		return false;
	}

	ff_is_await_ = false;

	while (!ff_is_await_)
	{
		switch (ff_state_)
		{
		case FfState::read:
			handle_read_state();
			break;

		case FfState::send:
			handle_send_state();
			break;

		case FfState::receive:
			handle_receive_state();
			break;

		case FfState::decode:
			handle_decode_state();
			break;

		case FfState::flush:
			handle_flush_state();
			break;

		case FfState::failed:
		case FfState::finished:
			ff_is_await_ = true;
			ff_is_finished_ = true;
			break;

		case FfState::none:
		default:
			throw "Invalid state.";
		}
	}

	return ff_state_ != FfState::failed;
}

void FmvPlayer::Impl::get_frame_rate_internal(
	int& numerator,
	int& denominator) const
{
	const auto& time_base = ff_format_context_->streams[ff_video_stream_index_]->r_frame_rate;

	numerator = time_base.num;
	denominator = time_base.den;
}

bool FmvPlayer::Impl::get_frame_rate(
	int& numerator,
	int& denominator) const
{
	if (!is_initialized_)
	{
		numerator = 0;
		denominator = 1;

		return false;
	}

	get_frame_rate_internal(numerator, denominator);

	return true;
}

int FmvPlayer::Impl::io_read_function_proxy(
	void* user_data,
	std::uint8_t* buffer,
	const int buffer_size)
{
	auto& pimpl = *static_cast<Impl*>(user_data);

	const auto read_result = pimpl.ff_io_read_function_(pimpl.ff_io_user_data_, buffer, buffer_size);

	if (read_result < 0)
	{
		return AVERROR_UNKNOWN;
	}

	return read_result;
}

std::int64_t FmvPlayer::Impl::io_seek_function_proxy(
	void* user_data,
	const std::int64_t offset,
	const int whence)
{
	auto api_whence = SeekOrigin::none;

	if ((whence & AVSEEK_SIZE) != 0)
	{
		api_whence = SeekOrigin::size;
	}
	else
	{
		switch (whence)
		{
		case SEEK_SET:
			api_whence = SeekOrigin::begin;
			break;

		case SEEK_CUR:
			api_whence = SeekOrigin::current;
			break;

		case SEEK_END:
			api_whence = SeekOrigin::end;
			break;

		default:
			return -1;
		}
	}

	auto& pimpl = *static_cast<Impl*>(user_data);

	return pimpl.ff_io_seek_function_(pimpl.ff_io_user_data_, offset, api_whence);
}

bool FmvPlayer::Impl::initialize_io(
	const InitializeParam& param)
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

	ff_io_user_data_ = param.user_data_;
	ff_io_read_function_ = param.io_read_func_;
	ff_io_seek_function_ = param.io_seek_func_;

	return true;
}

bool FmvPlayer::Impl::initialize_decoder(
	const InitializeParam& param)
{
	auto ff_result = 0;

	ff_format_context_ = ::avformat_alloc_context();

	if (!ff_format_context_)
	{
		return false;
	}

	ff_format_context_->pb = ff_io_context_;

	ff_result = ::avformat_open_input(
		&ff_format_context_,
		nullptr,
		get_ff_bik_input_format(),
		nullptr);

	if (ff_result != 0)
	{
		return false;
	}

	const auto stream_count = static_cast<int>(ff_format_context_->nb_streams);

	using AVStreamPtr = AVStream*;

	if (param.is_ignore_audio_)
	{
		ff_audio_stream_index_ = -1;
	}
	else
	{
		ff_audio_stream_index_ = ::av_find_best_stream(
			ff_format_context_, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	}

	ff_video_stream_index_ = ::av_find_best_stream(
		ff_format_context_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);

	if (stream_count > 0)
	{
		// Check for stream parameters.
		//
		for (auto i = 0; i < stream_count; ++i)
		{
			const auto stream = ff_format_context_->streams[i];
			const auto& codec_params = *stream->codecpar;

			if (codec_params.codec_type == AVMEDIA_TYPE_AUDIO)
			{
				if (!param.is_ignore_audio_ && ff_audio_stream_index_ < 0)
				{
					ff_audio_stream_index_ = i;
				}
			}
			else if (codec_params.codec_type == AVMEDIA_TYPE_VIDEO && codec_params.codec_id == AV_CODEC_ID_BINKVIDEO)
			{
				if (ff_video_stream_index_ < 0)
				{
					ff_video_stream_index_ = i;
				}
			}
		}

		// Discard all unused stream.
		//
		for (auto i = 0; i < stream_count; ++i)
		{
			const auto stream = ff_format_context_->streams[i];

			if (i != ff_audio_stream_index_ && i != ff_video_stream_index_)
			{
				stream->discard = AVDISCARD_ALL;
			}
		}
	}

	auto has_audio_data = (ff_audio_stream_index_ >= 0);
	auto has_video_data = (ff_video_stream_index_ >= 0);

	if (!has_video_data)
	{
		return false;
	}

	ff_result = ::avformat_find_stream_info(ff_format_context_, nullptr);

	if (ff_result < 0)
	{
		return false;
	}

	if (has_audio_data)
	{
		const auto stream = ff_format_context_->streams[ff_audio_stream_index_];

		ff_audio_codec_context_ = ::avcodec_alloc_context3(nullptr);

		if (!ff_audio_codec_context_)
		{
			return false;
		}

		ff_result = ::avcodec_parameters_to_context(ff_audio_codec_context_, stream->codecpar);

		if (ff_result < 0)
		{
			return false;
		}
	}

	if (has_video_data)
	{
		const auto stream = ff_format_context_->streams[ff_video_stream_index_];

		ff_video_codec_context_ = ::avcodec_alloc_context3(nullptr);

		if (!ff_video_codec_context_)
		{
			return false;
		}

		ff_result = ::avcodec_parameters_to_context(ff_video_codec_context_, stream->codecpar);

		if (ff_result < 0)
		{
			return false;
		}
	}

	if (has_audio_data)
	{
		ff_audio_codec_ = ::avcodec_find_decoder(ff_audio_codec_context_->codec_id);

		if (!ff_audio_codec_)
		{
			return false;
		}

		ff_result = ::avcodec_open2(ff_audio_codec_context_, ff_audio_codec_, nullptr);

		if (ff_result != 0)
		{
			return false;
		}
	}

	if (has_video_data)
	{
		ff_video_codec_ = ::avcodec_find_decoder(ff_video_codec_context_->codec_id);

		if (!ff_video_codec_)
		{
			return false;
		}

		ff_result = ::avcodec_open2(ff_video_codec_context_, ff_video_codec_, nullptr);

		if (ff_result != 0)
		{
			return false;
		}
	}

	auto src_sample_format = AV_SAMPLE_FMT_NONE;
	auto src_channel_layout = std::uint64_t{};
	auto src_sample_rate = 0;

	auto dst_sample_format = AV_SAMPLE_FMT_NONE;
	auto dst_channel_count = 0;
	auto dst_channel_layout = std::uint64_t{};
	auto dst_sample_rate = 0;

	auto need_audio_converter = false;

	if (has_audio_data)
	{
		src_sample_format = ff_audio_codec_context_->sample_fmt;
		src_channel_layout = ff_audio_codec_context_->channel_layout;
		src_sample_rate = ff_audio_codec_context_->sample_rate;

		dst_sample_format = AV_SAMPLE_FMT_S16;
		dst_channel_count = FmvPlayerDetail::audio_dst_channel_count;
		dst_channel_layout = static_cast<std::uint64_t>(::av_get_default_channel_layout(dst_channel_count));
		dst_sample_rate = param.dst_sample_rate_;

		if (dst_sample_rate == 0)
		{
			dst_sample_rate = src_sample_rate;
		}

		need_audio_converter = (
			src_sample_format != dst_sample_format ||
			src_channel_layout != dst_channel_layout ||
			src_sample_rate != ff_dst_sample_rate_);
	}

	if (has_audio_data && need_audio_converter)
	{
		ff_swr_context_ = ::swr_alloc_set_opts(
			ff_swr_context_,
			dst_channel_layout,
			dst_sample_format,
			dst_sample_rate,
			src_channel_layout,
			src_sample_format,
			src_sample_rate,
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

	auto need_video_converter = false;

	if (has_video_data)
	{
		need_video_converter = (ff_video_codec_context_->pix_fmt != AV_PIX_FMT_RGB32);
	}

	if (has_video_data && need_video_converter)
	{
		ff_sws_context_ = ::sws_getCachedContext(
			ff_sws_context_,
			ff_video_codec_context_->width,
			ff_video_codec_context_->height,
			ff_video_codec_context_->pix_fmt,
			ff_video_codec_context_->width,
			ff_video_codec_context_->height,
			AV_PIX_FMT_RGB32,
			SWS_POINT,
			nullptr,
			nullptr,
			nullptr);

		if (!ff_sws_context_)
		{
			return false;
		}
	}

	if (has_audio_data)
	{
		ff_audio_frame_ = ::av_frame_alloc();

		if (!ff_audio_frame_)
		{
			return false;
		}

		audio_buffer_size_ = static_cast<int>((
			param.audio_buffer_size_ms_ *
				dst_sample_rate * FmvPlayerDetail::audio_dst_sample_size) / 1000LL);

		audio_buffer_size_ += FmvPlayerDetail::audio_dst_sample_size - 1;
		audio_buffer_size_ /= FmvPlayerDetail::audio_dst_sample_size;
		audio_buffer_size_ *= FmvPlayerDetail::audio_dst_sample_size;
	}

	if (has_video_data)
	{
		ff_video_frame_ = ::av_frame_alloc();

		if (!ff_video_frame_)
		{
			return false;
		}
	}

	ff_state_ = FfState::read;
	ff_dst_sample_rate_ = dst_sample_rate;
	initialize_param_ = param;

	return true;
}

std::int64_t FmvPlayer::Impl::normalize_pts(
	const std::int64_t pts)
{
	return pts != AV_NOPTS_VALUE ? pts : 0i64;
}

FmvPlayer::Impl::AVInputFormatPtr& FmvPlayer::Impl::get_ff_bik_input_format()
{
	static AVInputFormatPtr result;
	return result;
}

int FmvPlayer::Impl::get_sample_rate() const
{
	if (!is_initialized_)
	{
		return 0;
	}

	return ff_dst_sample_rate_;
}

int FmvPlayer::Impl::get_audio_buffer_size() const
{
	if (!is_initialized_)
	{
		return 0;
	}

	return audio_buffer_size_;
}

int FmvPlayer::Impl::get_width() const
{
	if (!is_initialized_)
	{
		return 0;
	}

	return ff_video_codec_context_->width;
}

int FmvPlayer::Impl::get_height() const
{
	if (!is_initialized_)
	{
		return 0;
	}

	return ff_video_codec_context_->height;
}

bool FmvPlayer::Impl::has_audio() const
{
	if (!is_initialized_)
	{
		return false;
	}

	return ff_audio_codec_context_ != nullptr;
}

bool FmvPlayer::Impl::present()
{
	if (!is_initialized_ || is_presentation_finished_)
	{
		return false;
	}

	if (!is_present_all_frames_ && !is_present_one_frame_)
	{
		is_present_all_frames_ = true;
	}

	if (is_present_one_frame_)
	{
		is_presentation_finished_ = true;
		return false;
	}

	if (initialize_param_.is_cancelled_func_ &&
		initialize_param_.is_cancelled_func_(initialize_param_.user_data_))
	{
		return true;
	}

	is_presentation_finished_ = true;

	auto fr_num = 0;
	auto fr_den = 0;

	if (!get_frame_rate(fr_num, fr_den))
	{
		return false;
	}

	const auto fr_ms = (1000i64 * fr_den) / fr_num;

	mt_pts_ms_.store(0, std::memory_order_release);

	mt_clock_thread_ = std::thread{std::bind(&Impl::clock_worker, this)};

	mt_decoder_thread_ = std::thread{std::bind(&Impl::decoder_worker, this)};

	if (has_audio())
	{
		mt_audio_thread_ = std::thread{std::bind(&Impl::audio_worker, this)};
	}


	constexpr auto max_video_size = 2;

	const auto width = get_width();
	const auto height = get_height();

	auto frame_count_to_remove = 0;

	wait_for_preload();

	while (true)
	{
		if (initialize_param_.is_cancelled_func_ &&
			initialize_param_.is_cancelled_func_(initialize_param_.user_data_))
		{
			break;
		}

		auto frame_count = 0;
		auto frame_index = 0;
		auto frame_it = Frames::const_iterator{};

		if (frame_count_to_remove > 0)
		{
			MutexGuard mutex_guard{mt_video_mutex_};

			auto begin_it = mt_video_frames_.cbegin();
			auto end_it = begin_it + frame_count_to_remove;

			mt_video_frames_.erase(begin_it, end_it);

			frame_count = static_cast<int>(mt_video_frames_.size());
			frame_it = mt_video_frames_.cbegin();

			frame_count_to_remove = 0;
		}
		else
		{
			MutexGuard mutex_guard{mt_video_mutex_};

			frame_count = static_cast<int>(mt_video_frames_.size());
			frame_it = mt_video_frames_.cbegin();
		}

		const auto pts_ms = mt_pts_ms_.load(std::memory_order_acquire);

		const auto begin_time_point = std::chrono::system_clock::now();

		auto is_presented = false;
		auto delay_ms = 0i64;

		while (frame_index < frame_count)
		{
			auto& frame = *frame_it;

			const auto pts_ms_delta = frame.pts_ms_ - pts_ms;

			if (pts_ms_delta < -10)
			{
				frame_count_to_remove += 1;
				frame_index += 1;
				++frame_it;

				is_presented = false;
			}
			else if (pts_ms_delta <= 10)
			{
				initialize_param_.video_present_func_(
					initialize_param_.user_data_,
					frame_it->data_.data(),
					width, height);

				frame_count_to_remove += 1;
				frame_index += 1;
				++frame_it;

				is_presented = true;

				const auto end_time_point = std::chrono::system_clock::now();
				const auto delta_time_point = end_time_point - begin_time_point;

				const auto delta_ms = static_cast<int>(
					std::chrono::duration_cast<std::chrono::milliseconds>(delta_time_point).count());

				delay_ms = fr_ms - delta_ms;

				if (delay_ms < 0)
				{
					delay_ms = 0;
				}

				break;
			}
			else
			{
				delay_ms = 0;
				break;
			}
		}

		if (is_decoder_worker_finished_)
		{
			if (frame_count == 0 && frame_count_to_remove == 0)
			{
				break;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds{delay_ms});
	}

	stop_workers();

	return true;
}

bool FmvPlayer::Impl::present_frame()
{
	if (!is_initialized_ || is_presentation_finished_)
	{
		return false;
	}

	if (!is_present_all_frames_ && !is_present_one_frame_)
	{
		is_present_one_frame_ = true;
	}

	if (is_present_all_frames_)
	{
		is_presentation_finished_ = true;
		return false;
	}

	if (!present_frame_initialize())
	{
		return false;
	}

	const auto width = get_width();
	const auto height = get_height();

	auto frame_count = 0;
	auto frame_index = 0;
	auto frame_it = Frames::const_iterator{};

	if (present_frame_context_.frame_count_to_remove_ > 0)
	{
		MutexGuard mutex_guard{mt_video_mutex_};

		auto begin_it = mt_video_frames_.cbegin();
		auto end_it = begin_it + present_frame_context_.frame_count_to_remove_;

		mt_video_frames_.erase(begin_it, end_it);

		frame_count = static_cast<int>(mt_video_frames_.size());
		frame_it = mt_video_frames_.cbegin();

		present_frame_context_.frame_count_to_remove_ = 0;
	}
	else
	{
		MutexGuard mutex_guard{mt_video_mutex_};

		frame_count = static_cast<int>(mt_video_frames_.size());
		frame_it = mt_video_frames_.cbegin();
	}

	const auto pts_ms = mt_pts_ms_.load(std::memory_order_acquire);

	const auto begin_time_point = std::chrono::system_clock::now();

	auto is_presented = false;
	auto delay_ms = 0i64;

	while (frame_index < frame_count)
	{
		auto& frame = *frame_it;

		const auto pts_ms_delta = frame.pts_ms_ - pts_ms;

		if (pts_ms_delta < -20)
		{
			present_frame_context_.frame_count_to_remove_ += 1;
			frame_index += 1;
			++frame_it;

			is_presented = false;
		}
		else if (pts_ms_delta <= 20)
		{
			initialize_param_.video_present_func_(
				initialize_param_.user_data_,
				frame_it->data_.data(),
				width,
				height);

			present_frame_context_.frame_count_to_remove_ += 1;
			frame_index += 1;
			++frame_it;

			is_presented = true;

			const auto end_time_point = std::chrono::system_clock::now();
			const auto delta_time_point = end_time_point - begin_time_point;

			const auto delta_ms = static_cast<int>(
				std::chrono::duration_cast<std::chrono::milliseconds>(delta_time_point).count());

			delay_ms = present_frame_context_.frame_rate_ms_ - delta_ms;

			break;
		}
		else
		{
			delay_ms = 0;
			break;
		}
	}

	if (is_decoder_worker_finished_)
	{
		if (frame_count == 0 && present_frame_context_.frame_count_to_remove_ == 0)
		{
			is_presentation_finished_ = true;
			stop_workers();
			return true;
		}
	}

	if (delay_ms > 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds{delay_ms});
	}

	return true;
}

bool FmvPlayer::Impl::is_presentation_finished() const
{
	return is_presentation_finished_;
}

void FmvPlayer::Impl::initialize_current_thread()
{
	::av_log_set_level(AV_LOG_QUIET);
	::av_register_all();

	get_ff_bik_input_format() = ::av_find_input_format("bik");
}

void FmvPlayer::Impl::clock_worker()
{
	using Clock = std::chrono::system_clock;

	const auto sleep_delay_ms = std::chrono::milliseconds{FmvPlayerDetail::default_sleep_delay_ms};

	wait_for_preload();

	const auto begin_time_point = Clock::now();

	while (!is_stop_threads_)
	{
		std::this_thread::sleep_for(sleep_delay_ms);

		const auto end_time_point = Clock::now();
		const auto duration_diff = end_time_point - begin_time_point;
		const auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration_diff).count();

		mt_pts_ms_.store(duration_ms, std::memory_order_release);
	}
}

void FmvPlayer::Impl::decoder_worker()
{
	const auto min_video_frame_count = 3;
	const auto has_audio = (ff_audio_codec_context_ != nullptr);

	const auto sleep_delay_ms = std::chrono::milliseconds{FmvPlayerDetail::default_sleep_delay_ms};

	auto is_decoded = false;
	auto is_preload_notified = false;

	while (!is_stop_threads_ && !is_decoder_worker_finished_)
	{
		if (!is_decoded)
		{
			while (true)
			{
				auto is_video_enough = false;
				auto video_frame_count = 0;

				{
					MutexGuard mutex_guard{mt_video_mutex_};

					video_frame_count = static_cast<int>(mt_video_frames_.size());
				}

				if (video_frame_count >= min_video_frame_count)
				{
					is_video_enough = true;
				}

				auto is_audio_enough = false;
				auto audio_frame_count = 0;

				if (initialize_param_.is_ignore_audio_)
				{
					is_audio_enough = true;
				}
				else
				{
					{
						MutexGuard mutex_guard{mt_audio_mutex_};

						audio_frame_count = static_cast<int>(mt_audio_frames_.size());
					}

					if (audio_frame_count >= initialize_param_.audio_max_buffer_count_)
					{
						is_audio_enough = true;
					}
				}

				if (is_video_enough && is_audio_enough)
				{
					notify_preload(is_preload_notified);
					break;
				}

				if (!decode())
				{
					is_decoded = true;
					notify_preload(is_preload_notified);
					break;
				}
			}
		}
		else
		{
			auto video_frame_count = 0;

			{
				MutexGuard mutex_guard{mt_video_mutex_};

				video_frame_count = static_cast<int>(mt_video_frames_.size());
			}

			auto audio_frame_count = 0;

			{
				MutexGuard mutex_guard{mt_audio_mutex_};

				audio_frame_count = static_cast<int>(mt_audio_frames_.size());
			}

			if (video_frame_count == 0 &&
				audio_frame_buffer_.data_size_ == 0 &&
				audio_frame_count == 0)
			{
				is_decoder_worker_finished_ = true;
			}
		}

		std::this_thread::sleep_for(sleep_delay_ms);
	}

	notify_preload(is_preload_notified);
}

void FmvPlayer::Impl::audio_worker()
{
	const auto sleep_delay_ms = std::chrono::milliseconds{FmvPlayerDetail::default_sleep_delay_ms};

	const auto threshold_ms = 20i64;

	auto frame_count_to_remove = 0;

	wait_for_preload();

	while (!is_stop_threads_ && !is_audio_worker_finished_)
	{
		auto frame_count = 0;
		auto frame_it = Frames::const_iterator{};

		if (frame_count_to_remove > 0)
		{
			MutexGuard mutex_guard{mt_audio_mutex_};

			auto begin_it = mt_audio_frames_.cbegin();
			auto end_it = begin_it + frame_count_to_remove;

			mt_audio_frames_.erase(begin_it, end_it);

			frame_count = static_cast<int>(mt_audio_frames_.size());
			frame_it = mt_audio_frames_.cbegin();

			frame_count_to_remove = 0;
		}
		else
		{
			MutexGuard mutex_guard{mt_audio_mutex_};

			frame_count = static_cast<int>(mt_audio_frames_.size());
			frame_it = mt_audio_frames_.cbegin();
		}

		auto is_filled = false;

		const auto pts_ms = mt_pts_ms_.load(std::memory_order_acquire);
		const auto free_buffer_count = initialize_param_.audio_get_free_buffer_count_func_(initialize_param_.user_data_);
		const auto already_queued_count = initialize_param_.audio_max_buffer_count_ - free_buffer_count;

		auto buffer_count = 0;
		auto frame_index = 0;

		while (frame_index < frame_count && buffer_count < free_buffer_count)
		{
			const auto& frame = *frame_it;

			const auto frame_ms = size_to_ms(frame.data_size_);
			const auto frame_end_ms = frame.pts_ms_ + frame_ms;

			if (pts_ms <= frame_end_ms)
			{
				is_filled = true;

				buffer_count += 1;
				initialize_param_.audio_present_func_(
					initialize_param_.user_data_,
					frame.data_.data(),
					frame.data_size_);

				frame_index += 1;
				frame_count_to_remove += 1;
				++frame_it;
			}
			else
			{
				frame_index += 1;
				frame_count_to_remove += 1;
				++frame_it;

				for (auto i = 0; i < already_queued_count - 1; ++i)
				{
					if (frame_index == (frame_count - 1))
					{
						break;
					}

					frame_index += 1;
					frame_count_to_remove += 1;
					++frame_it;
				}
			}
		}

		if (is_decoder_worker_finished_)
		{
			if (!is_filled && frame_count == 0 && frame_count_to_remove == 0)
			{
				break;
			}
		}

		std::this_thread::sleep_for(sleep_delay_ms);
	}
}

void FmvPlayer::Impl::stop_workers()
{
	is_stop_threads_ = true;

	if (mt_decoder_thread_.joinable())
	{
		mt_decoder_thread_.join();
	}

	if (mt_audio_thread_.joinable())
	{
		mt_audio_thread_.join();
	}

	if (mt_clock_thread_.joinable())
	{
		mt_clock_thread_.join();
	}
}

void FmvPlayer::Impl::wait_for_preload()
{
	UniqueLock lock{mt_preload_mutex_};

	mt_preload_cv_.wait(lock, [&](){ return mt_preload_flag_; });
}

void FmvPlayer::Impl::notify_preload(
	bool& flag)
{
	if (flag)
	{
		return;
	}

	flag = true;

	UniqueLock lock{mt_preload_mutex_};

	mt_preload_flag_ = true;
	mt_preload_cv_.notify_all();
}

std::int64_t FmvPlayer::Impl::ms_to_size(
	const std::int64_t milliseconds)
{
	const auto sample_size = static_cast<std::int64_t>(FmvPlayerDetail::audio_dst_sample_size);
	const auto sample_rate = static_cast<std::int64_t>(ff_dst_sample_rate_);

	const auto size = (milliseconds * sample_size * sample_rate) / 1000i64;

	return size;
}

std::int64_t FmvPlayer::Impl::size_to_ms(
	const std::int64_t size)
{
	const auto sample_size = static_cast<std::int64_t>(FmvPlayerDetail::audio_dst_sample_size);
	const auto sample_rate = static_cast<std::int64_t>(ff_dst_sample_rate_);

	const auto ms = (1000i64 * size) / (sample_size * sample_rate);

	return ms;
}

bool FmvPlayer::Impl::present_frame_initialize()
{
	if (present_frame_context_.is_initialized_)
	{
		return true;
	}

	present_frame_context_.is_initialized_ = true;

	auto fr_num = 0;
	auto fr_den = 0;

	if (!get_frame_rate(fr_num, fr_den))
	{
		is_presentation_finished_ = true;
		return false;
	}

	present_frame_context_.frame_rate_ms_ = (1000i64 * fr_den) / fr_num;

	mt_pts_ms_.store(0, std::memory_order_release);

	mt_clock_thread_ = std::thread{std::bind(&Impl::clock_worker, this)};

	mt_decoder_thread_ = std::thread{std::bind(&Impl::decoder_worker, this)};

	if (has_audio())
	{
		mt_audio_thread_ = std::thread{std::bind(&Impl::audio_worker, this)};
	}

	present_frame_context_.frame_count_to_remove_ = 0;

	wait_for_preload();

	return true;
}

FmvPlayer::FmvPlayer()
	:
	impl_{std::make_unique<Impl>()}
{
}

FmvPlayer::FmvPlayer(
	FmvPlayer&& that)
	:
	impl_{std::move(that.impl_)}
{
}

FmvPlayer::~FmvPlayer()
{
}

void FmvPlayer::initialize_current_thread()
{
	Impl::initialize_current_thread();
}

bool FmvPlayer::initialize(
	const InitializeParam& param)
{
	return impl_->initialize(param);
}

void FmvPlayer::uninitialize()
{
	impl_->uninitialize();
}

bool FmvPlayer::is_initialized() const
{
	return impl_->is_initialized();
}

int FmvPlayer::get_channel_count()
{
	return FmvPlayerDetail::audio_dst_channel_count;
}

int FmvPlayer::get_bit_depth()
{
	return FmvPlayerDetail::audio_dst_bit_depth;
}

int FmvPlayer::get_sample_size()
{
	return FmvPlayerDetail::audio_dst_sample_size;
}

int FmvPlayer::get_sample_rate() const
{
	return impl_->get_sample_rate();
}

int FmvPlayer::get_audio_buffer_size() const
{
	return impl_->get_audio_buffer_size();
}

int FmvPlayer::get_width() const
{
	return impl_->get_width();
}

int FmvPlayer::get_height() const
{
	return impl_->get_height();
}

bool FmvPlayer::has_audio() const
{
	return impl_->has_audio();
}

bool FmvPlayer::present()
{
	return impl_->present();
}

bool FmvPlayer::present_frame()
{
	return impl_->present_frame();
}

bool FmvPlayer::is_presentation_finished() const
{
	return impl_->is_presentation_finished();
}


} // ltjs
