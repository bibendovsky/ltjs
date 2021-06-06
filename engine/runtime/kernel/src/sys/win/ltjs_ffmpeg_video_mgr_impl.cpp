#include "bdefs.h"


#ifdef LTJS_USE_FFMPEG_VIDEO_MGR


#include "ltjs_ffmpeg_video_mgr_impl.h"
#include <array>
#include <memory>
#include <utility>
#include "bibendovsky_spul_file_stream.h"
#include "ltjs_fmv_player.h"
#include "render.h"
#include "soundmgr.h"


namespace ltjs
{


namespace ul = bibendovsky::spul;


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// FfmpegVideoInst::Impl
//

class FfmpegVideoInst::Impl
{
public:
	Impl(
		VideoMgr* video_mgr_ptr);

	~Impl();


	bool api_initialize(
		const char* const file_name);

	void api_release(
		VideoInst* video_inst_ptr);

	void api_on_render_init();

	void api_on_render_term();

	LTRESULT api_update();

	LTRESULT api_draw_video();

	LTRESULT api_get_video_status();


private:
	static constexpr auto audio_buffer_size_ms = 50;
	static constexpr auto default_format_string = "[FfmpegVideoInst] %s";


	struct Vertex
	{
		LTVector position_;
		float rhw_;
		float u_;
		float v_;


		void initialize(
			const float x,
			const float y,
			const float u,
			const float v)
		{
			position_.Init(x, y);

			rhw_ = 1.0f;
			u_ = u;
			v_ = v;
		}
	}; // Vertex

	using Vertices = std::array<Vertex, 4>;


	bool is_initialized_;

	VideoMgr* video_mgr_ptr_;
	FmvPlayer fmv_player_;
	ul::FileStream file_stream_;
	bool is_audio_stream_started_;
	LtjsLtSoundSysGenericStream* audio_stream_;
	ILTSoundSys* sound_sys_ptr_;
	int audio_max_buffer_count_;

	LPDIRECT3DTEXTURE9 d3d9_texture_;
	std::uint32_t texture_width_;
	std::uint32_t texture_height_;
	Vertices vertices_;


	bool initialize_internal(
		const char* const file_name);

	void uninitialize_internal();


	int io_read_func(
		std::uint8_t* buffer,
		const int buffer_size);

	static int io_read_func_proxy(
		void* user_data,
		std::uint8_t* buffer,
		const int buffer_size);

	std::int64_t io_seek_func(
		const std::int64_t offset,
		const FmvPlayer::SeekOrigin whence);

	static std::int64_t io_seek_func_proxy(
		void* user_data,
		const std::int64_t offset,
		const FmvPlayer::SeekOrigin whence);

	void audio_present_func(
		const void* data,
		const int data_size);

	static void audio_present_func_proxy(
		void* user_data,
		const void* data,
		const int data_size);

	int audio_get_free_buffer_count_func();

	static int audio_get_free_buffer_count_func_proxy(
		void* user_data);

	void video_present_func(
		const void* data,
		const int width,
		const int height);

	static void video_present_func_proxy(
		void* user_data,
		const void* data,
		const int width,
		const int height);

	// Calculates dimension for texture nearest to the two power of N.
	//
	static std::uint32_t calculate_texture_dimension(
		const std::uint32_t value);

	bool clear_surface();

	LTRESULT init_screen();

	LTRESULT update_on_screen(
		const void* const data_ptr,
		const int width,
		const int height);
}; // FfmpegVideoInst::Impl


FfmpegVideoInst::Impl::Impl(
	VideoMgr* video_mgr_ptr)
	:
	is_initialized_{},
	video_mgr_ptr_{video_mgr_ptr},
	fmv_player_{},
	file_stream_{},
	is_audio_stream_started_{},
	audio_stream_{},
	sound_sys_ptr_{},
	audio_max_buffer_count_{},
	d3d9_texture_{},
	texture_width_{},
	texture_height_{},
	vertices_{}
{
}

FfmpegVideoInst::Impl::~Impl()
{
}

bool FfmpegVideoInst::Impl::api_initialize(
	const char* const file_name)
{
	uninitialize_internal();

	if (!initialize_internal(file_name))
	{
		uninitialize_internal();
		return false;
	}

	return true;
}

void FfmpegVideoInst::Impl::api_release(
	VideoInst* video_inst_ptr)
{
	if (video_mgr_ptr_->m_Videos.FindElement(video_inst_ptr) != BAD_INDEX)
	{
		video_mgr_ptr_->m_Videos.RemoveAt(&video_inst_ptr->m_Link);
	}

	uninitialize_internal();
	api_on_render_term();

	delete this;
}

void FfmpegVideoInst::Impl::api_on_render_init()
{
	if( init_screen() != LT_OK )
		api_on_render_term();
}

void FfmpegVideoInst::Impl::api_on_render_term()
{
	// release any textures if present
	if(d3d9_texture_)
	{
		d3d9_texture_->Release();
		d3d9_texture_ = NULL;
	}

	texture_width_ = 0;
	texture_height_ = 0;
}

LTRESULT FfmpegVideoInst::Impl::api_update()
{
	return LT_FINISHED;
}

LTRESULT FfmpegVideoInst::Impl::api_draw_video()
{
	if (!fmv_player_.is_initialized())
	{
		return LT_FINISHED;
	}

	if (!fmv_player_.present_frame())
	{
		return LT_FINISHED;
	}

	if (sound_sys_ptr_ && !is_audio_stream_started_)
	{
		is_audio_stream_started_ = true;
		audio_stream_->set_pause(false);
	}

	return LT_OK;
}

LTRESULT FfmpegVideoInst::Impl::api_get_video_status()
{
	if (!fmv_player_.is_initialized() || fmv_player_.is_presentation_finished())
	{
		return LT_FINISHED;
	}

	return LT_OK;
}

bool FfmpegVideoInst::Impl::initialize_internal(
	const char* const file_name)
{
	if (!file_name)
	{
		::dsi_ConsolePrint(default_format_string, "No file name.");
		return false;
	}

	const auto file_stream_result = file_stream_.open(file_name, ul::Stream::OpenMode::read);

	if (!file_stream_result)
	{
		const auto error_message = "Failed to open \"" + std::string{file_name} + "\"";
		::dsi_ConsolePrint(default_format_string, error_message.c_str());
		return false;
	}


	sound_sys_ptr_ = GetSoundSys();

	if (sound_sys_ptr_)
	{
		audio_max_buffer_count_ = sound_sys_ptr_->ltjs_get_generic_stream_queue_size();

		if (audio_max_buffer_count_ <= 0)
		{
			::dsi_ConsolePrint(default_format_string, "Failed to get generic stream queue size.");
			return false;
		}
	}

	const auto is_ignore_audio = (sound_sys_ptr_ == nullptr);

	auto initialize_param = FmvPlayer::InitializeParam{};
	initialize_param.is_ignore_audio_ = is_ignore_audio;

	if (!is_ignore_audio)
	{
		initialize_param.dst_sample_rate_ = 0;
		initialize_param.audio_buffer_size_ms_ = audio_buffer_size_ms;
		initialize_param.audio_max_buffer_count_ = audio_max_buffer_count_;
	}

	initialize_param.user_data_ = this;

	initialize_param.io_read_func_ = io_read_func_proxy;
	initialize_param.io_seek_func_ = io_seek_func_proxy;
	initialize_param.is_cancelled_func_ = nullptr;
	initialize_param.video_present_func_ = video_present_func_proxy;
	initialize_param.audio_present_func_ = audio_present_func_proxy;
	initialize_param.audio_get_free_buffer_count_func_ = audio_get_free_buffer_count_func_proxy;

	if (!fmv_player_.initialize(initialize_param))
	{
		::dsi_ConsolePrint(default_format_string, "Failed to initialize.");
		return false;
	}

	if (!is_ignore_audio)
	{
		const auto sample_rate = fmv_player_.get_sample_rate();
		const auto buffer_size = fmv_player_.get_audio_buffer_size();

		audio_stream_ = sound_sys_ptr_->ltjs_open_generic_stream(sample_rate, buffer_size);

		if (!audio_stream_)
		{
			::dsi_ConsolePrint(default_format_string, "Failed to open generic audio stream.");
			return false;
		}
	}

	api_on_render_init();

	is_initialized_ = true;

	return true;
}

void FfmpegVideoInst::Impl::uninitialize_internal()
{
	is_initialized_ = false;

	fmv_player_.uninitialize();
	file_stream_.close();

	if (sound_sys_ptr_)
	{
		is_audio_stream_started_ = false;

		if (audio_stream_)
		{
			sound_sys_ptr_->ltjs_close_generic_stream(audio_stream_);
			audio_stream_ = nullptr;
		}

		sound_sys_ptr_ = nullptr;
	}

	audio_max_buffer_count_ = 0;
}

int FfmpegVideoInst::Impl::io_read_func(
	std::uint8_t* buffer,
	const int buffer_size)
{
	return file_stream_.read(buffer, buffer_size);
}

int FfmpegVideoInst::Impl::io_read_func_proxy(
	void* user_data,
	std::uint8_t* buffer,
	const int buffer_size)
{
	auto instance = static_cast<FfmpegVideoInst::Impl*>(user_data);

	return instance->io_read_func(buffer, buffer_size);
}

std::int64_t FfmpegVideoInst::Impl::io_seek_func(
	const std::int64_t offset,
	const FmvPlayer::SeekOrigin whence)
{
	auto origin = ul::Stream::Origin{};

	switch (whence)
	{
	case ltjs::FmvPlayer::SeekOrigin::begin:
		origin = ul::Stream::Origin::begin;
		break;

	case ltjs::FmvPlayer::SeekOrigin::current:
		origin = ul::Stream::Origin::current;
		break;

	case ltjs::FmvPlayer::SeekOrigin::end:
		origin = ul::Stream::Origin::end;
		break;

	case ltjs::FmvPlayer::SeekOrigin::size:
		return file_stream_.get_size();

	case ltjs::FmvPlayer::SeekOrigin::none:
	default:
		return -1;
	}

	return file_stream_.set_position(offset, origin);
}

std::int64_t FfmpegVideoInst::Impl::io_seek_func_proxy(
	void* user_data,
	const std::int64_t offset,
	const FmvPlayer::SeekOrigin whence)
{
	auto instance = static_cast<FfmpegVideoInst::Impl*>(user_data);

	return instance->io_seek_func(offset, whence);
}

void FfmpegVideoInst::Impl::audio_present_func(
	const void* data,
	const int data_size)
{
	static_cast<void>(data_size);

	audio_stream_->enqueue_buffer(data);
}

void FfmpegVideoInst::Impl::audio_present_func_proxy(
	void* user_data,
	const void* data,
	const int data_size)
{
	auto instance = static_cast<FfmpegVideoInst::Impl*>(user_data);

	return instance->audio_present_func(data, data_size);
}

int FfmpegVideoInst::Impl::audio_get_free_buffer_count_func()
{
	return audio_stream_->get_free_buffer_count();
}

int FfmpegVideoInst::Impl::audio_get_free_buffer_count_func_proxy(
	void* user_data)
{
	auto instance = static_cast<FfmpegVideoInst::Impl*>(user_data);

	return instance->audio_get_free_buffer_count_func();
}

void FfmpegVideoInst::Impl::video_present_func(
	const void* data,
	const int width,
	const int height)
{
	static_cast<void>(update_on_screen(data, width, height));
}

void FfmpegVideoInst::Impl::video_present_func_proxy(
	void* user_data,
	const void* data,
	const int width,
	const int height)
{
	auto instance = static_cast<FfmpegVideoInst::Impl*>(user_data);

	return instance->video_present_func(data, width, height);
}

std::uint32_t FfmpegVideoInst::Impl::calculate_texture_dimension(
	const std::uint32_t value)
{
	auto dimension = std::uint32_t{1};

	while (dimension < value)
	{
		dimension <<= 1;
	}

	return dimension;
}

bool FfmpegVideoInst::Impl::clear_surface()
{
	if (!d3d9_texture_)
	{
		return false;
	}

	auto d3d9_surface = LPDIRECT3DSURFACE9{};

	if (FAILED(d3d9_texture_->GetSurfaceLevel(0, &d3d9_surface)))
	{
		return false;
	}

	D3DSURFACE_DESC	d3d9_surface_desc;

	if (FAILED(d3d9_surface->GetDesc(&d3d9_surface_desc)))
	{
		static_cast<void>(d3d9_surface->Release());
		return false;
	}

	D3DLOCKED_RECT locked_rect;

	if (FAILED(d3d9_surface->LockRect(&locked_rect, nullptr, 0)))
	{
		static_cast<void>(d3d9_surface->Release());
		return false;
	}

	auto dst_data_ptr = static_cast<uint8*>(locked_rect.pBits);

	for (auto i_line = 0U; i_line < d3d9_surface_desc.Height; ++i_line)
	{
		std::uninitialized_fill_n(dst_data_ptr, d3d9_surface_desc.Width * 4, std::uint8_t{});

		dst_data_ptr += locked_rect.Pitch;
	}

	static_cast<void>(d3d9_surface->UnlockRect());
	static_cast<void>(d3d9_surface->Release());

	return true;
}

LTRESULT FfmpegVideoInst::Impl::init_screen()
{
	api_on_render_term();

	if (!::r_IsRenderInitted())
	{
		RETURN_ERROR(1, FfmpegVideoInst::Impl::init_screen, LT_NOTINITIALIZED);
	}

	auto d3d9_device = ::r_GetRenderStruct()->GetD3DDevice();

	if (!d3d9_device)
	{
		RETURN_ERROR(1, FfmpegVideoInst::Impl::init_screen, LT_NOTINITIALIZED);
	}

	const auto fmv_width = fmv_player_.get_width();
	const auto fmv_height = fmv_player_.get_height();

	const auto texture_width = calculate_texture_dimension(fmv_width);
	const auto texture_height = calculate_texture_dimension(fmv_height);

	if (FAILED(d3d9_device->CreateTexture(
		texture_width,
		texture_height,
		1,
		0,
		D3DFMT_A8R8G8B8,
		D3DPOOL_MANAGED,
		&d3d9_texture_,
		nullptr)))
	{
		RETURN_ERROR_PARAM(1, FfmpegVideoInst::Impl::init_screen, LT_ERROR, "Failed to create texture.");
	}

	if (!clear_surface())
	{
		static_cast<void>(d3d9_texture_->Release());
		d3d9_texture_ = nullptr;

		RETURN_ERROR_PARAM(1, FfmpegVideoInst::Impl::init_screen, LT_ERROR, "Failed to clear texture.");
	}

	texture_width_ = texture_width;
	texture_height_ = texture_height;

	return LT_OK;
}

LTRESULT FfmpegVideoInst::Impl::update_on_screen(
	const void* const data_ptr,
	const int width,
	const int height)
{
	if (!::r_IsRenderInitted() || !d3d9_texture_)
	{
		RETURN_ERROR(1, FfmpegVideoInst::Impl::update_on_screen, LT_NOTINITIALIZED);
	}

	auto d3d9_device = ::r_GetRenderStruct()->GetD3DDevice();

	if (!d3d9_device)
	{
		RETURN_ERROR(1, FfmpegVideoInst::Impl::update_on_screen, LT_NOTINITIALIZED);
	}

	auto hr = HRESULT{};
	auto d3d9_surface = LPDIRECT3DSURFACE9{};

	hr = d3d9_texture_->GetSurfaceLevel(0, &d3d9_surface);

	if (FAILED(hr))
	{
		RETURN_ERROR_PARAM(1, FfmpegVideoInst::Impl::update_on_screen, LT_ERROR, "Failed to get texture's surface.");
	}

	D3DLOCKED_RECT locked_rect;

	hr = d3d9_surface->LockRect(&locked_rect, nullptr, 0);

	if (FAILED(hr))
	{
		static_cast<void>(d3d9_surface->Release());
		RETURN_ERROR_PARAM(1, FfmpegVideoInst::Impl::update_on_screen, LT_ERROR, "Failed to lock texture's surface.");
	}

	const auto src_pitch = 4 * width;

	for (auto i = 0; i < height; ++i)
	{
		const auto src_data_ptr = static_cast<const std::uint8_t*>(data_ptr) + (i * src_pitch);
		const auto dst_data_ptr = static_cast<std::uint8_t*>(locked_rect.pBits) + (i * locked_rect.Pitch);

		std::uninitialized_copy_n(src_data_ptr, src_pitch, dst_data_ptr);
	}

	hr = d3d9_surface->UnlockRect();
	static_cast<void>(d3d9_surface->Release());

	bool is_set_3d_state = !::r_GetRenderStruct()->IsIn3D();

	if (is_set_3d_state)
	{
		::r_GetRenderStruct()->Start3D();

		D3DVIEWPORT9 vp;
		vp.X = 0;
		vp.Y = 0;
		vp.Width = ::r_GetRenderStruct()->m_Width;
		vp.Height = ::r_GetRenderStruct()->m_Height;
		vp.MinZ = 0.0F;
		vp.MaxZ = 1.0F;
		hr = d3d9_device->SetViewport(&vp);

		LTRGBColor color;
		color.dwordVal = 0x00000000;

		::r_GetRenderStruct()->Clear(nullptr, CLEARSCREEN_SCREEN, color);
	}

	hr = d3d9_device->SetTexture(0, d3d9_texture_);
	hr = d3d9_device->SetVertexShader(nullptr);
	hr = d3d9_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

	// We now need to figure out the position and dimensions of the video since we want to
	// maintain the aspect ratio.
	//
	auto screen_width = ::r_GetRenderStruct()->m_Width;
	auto screen_height = ::r_GetRenderStruct()->m_Height;

	// We can basically fit either the width or height to the screen width or height and maintain
	//aspect ratio, so we will simply try both.
	//
	auto image_width = screen_width;
	auto image_height = (image_width * height) / width;

	if (image_height > screen_height)
	{
		image_height = screen_height;
		image_width = (image_height * width) / height;
	}

	// Sanity check.
	//
	assert(image_width <= screen_width);
	assert(image_height <= screen_height);

	// Now figure out the offsets.
	//
	const auto x_offset = (screen_width - image_width) / 2;
	const auto y_offset = (screen_height - image_height) / 2;

	// And the rectangle for the actual video to be rendered to.
	//
	const auto image_left = static_cast<float>(x_offset);
	const auto image_top = static_cast<float>(y_offset);
	const auto image_right = static_cast<float>(x_offset + image_width);
	const auto image_bottom = static_cast<float>(y_offset + image_height);

	// Figure out our bilinear offset.
	//
	const auto filtering_offset_x = 0.5F / static_cast<float>(texture_width_);
	const auto filtering_offset_y = 0.5F / static_cast<float>(texture_height_);

	// The texture locations since the video didn't necessarily fill the entire texture.
	//
	const auto texture_left = filtering_offset_x;
	const auto texture_top = filtering_offset_y;
	const auto texture_right = static_cast<float>(width) / static_cast<float>(texture_width_) - filtering_offset_x;
	const auto texture_bottom = static_cast<float>(height) / static_cast<float>(texture_height_) - filtering_offset_y;

	// Setup our vertices (static to avoid possible memory transfer issues).
	//
	vertices_[0].initialize(image_left, image_top, texture_left, texture_top);
	vertices_[1].initialize(image_right, image_top, texture_right, texture_top);
	vertices_[2].initialize(image_right, image_bottom, texture_right, texture_bottom);
	vertices_[3].initialize(image_left, image_bottom, texture_left, texture_bottom);

	// Now setup the texture and the texture stage states.
	//
	hr = d3d9_device->SetTexture(0, d3d9_texture_);
	hr = d3d9_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	hr = d3d9_device->SetRenderState(D3DRS_FOGENABLE, FALSE);
	hr = d3d9_device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	hr = d3d9_device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	hr = d3d9_device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	hr = d3d9_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

	hr = d3d9_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	hr = d3d9_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

	hr = d3d9_device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	hr = d3d9_device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	// And we are ready to render.
	//
	hr = d3d9_device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices_.data(), static_cast<UINT>(sizeof(Vertex)));

	// Disable that texture.
	//
	hr = d3d9_device->SetTexture(0, nullptr);

	if (is_set_3d_state)
	{
		::r_GetRenderStruct()->End3D();
	}

	return LT_OK;
}

//
// FfmpegVideoInst::Impl
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// FfmpegVideoMgr::Impl
//

class FfmpegVideoMgr::Impl
{
public:
	Impl();

	~Impl();


	LTRESULT api_initialize();

	LTRESULT api_create_screen_video(
		VideoMgr* video_mgr_ptr,
		const char* const pFilename,
		const uint32 flags,
		VideoInst*& pVideo);
}; // FfmpegVideoMgr::Impl


FfmpegVideoMgr::Impl::Impl()
{
}

FfmpegVideoMgr::Impl::~Impl()
{
}

LTRESULT FfmpegVideoMgr::Impl::api_initialize()
{
	ltjs::FmvPlayer::initialize_current_thread();

	return LT_OK;
}

LTRESULT FfmpegVideoMgr::Impl::api_create_screen_video(
	VideoMgr* video_mgr_ptr,
	const char* const file_name,
	const uint32 flags,
	VideoInst*& video_inst_ptr)
{
	static_cast<void>(flags);

	video_inst_ptr = nullptr;

	auto ffmpeg_video_inst_ptr = std::make_unique<FfmpegVideoInst>(video_mgr_ptr);

	if (!ffmpeg_video_inst_ptr)
	{
		return LT_ERROR;
	}

	if (!ffmpeg_video_inst_ptr->initialize(file_name))
	{
		return LT_ERROR;
	}

	video_inst_ptr = ffmpeg_video_inst_ptr.release();

	video_mgr_ptr->m_Videos.AddTail(video_inst_ptr, &video_inst_ptr->m_Link);

	return LT_OK;
}

//
// FfmpegVideoMgr::Impl
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// FfmpegVideoMgr
//

FfmpegVideoMgr::FfmpegVideoMgr()
	:
	impl_{std::make_unique<FfmpegVideoMgr::Impl>()}
{
}

FfmpegVideoMgr::FfmpegVideoMgr(
	FfmpegVideoMgr&& that)
	:
	impl_{std::move(that.impl_)}
{
}

FfmpegVideoMgr::~FfmpegVideoMgr()
{
}

LTRESULT FfmpegVideoMgr::initialize()
{
	return impl_->api_initialize();
}

LTRESULT FfmpegVideoMgr::CreateScreenVideo(
	const char* const file_name,
	const uint32 flags,
	VideoInst*& video_inst_ptr)
{
	return impl_->api_create_screen_video(this, file_name, flags, video_inst_ptr);
}

//
// FfmpegVideoMgr
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// FfmpegVideoInst
//

FfmpegVideoInst::FfmpegVideoInst(
	VideoMgr* video_mgr_ptr)
	:
	impl_{std::make_unique<Impl>(video_mgr_ptr)}
{
}

FfmpegVideoInst::FfmpegVideoInst(
	FfmpegVideoInst&& that)
	:
	impl_{std::move(that.impl_)}
{
}

FfmpegVideoInst::~FfmpegVideoInst()
{
}

bool FfmpegVideoInst::initialize(
	const char* const file_name)
{
	return impl_->api_initialize(file_name);
}

void FfmpegVideoInst::Release()
{
	impl_->api_release(this);
}

void FfmpegVideoInst::OnRenderInit()
{
	//impl_->api_on_render_init();
}

void FfmpegVideoInst::OnRenderTerm()
{
	impl_->api_on_render_term();
}

LTRESULT FfmpegVideoInst::Update()
{
	return impl_->api_update();
}

LTRESULT FfmpegVideoInst::DrawVideo()
{
	return impl_->api_draw_video();
}

LTRESULT FfmpegVideoInst::GetVideoStatus()
{
	return impl_->api_get_video_status();
}

//
// FfmpegVideoInst
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // ltjs


#endif // LTJS_USE_FFMPEG_VIDEO_MGR
