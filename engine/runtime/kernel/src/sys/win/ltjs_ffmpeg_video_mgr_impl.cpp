#include "bdefs.h"


#ifdef LTJS_USE_FFMPEG_VIDEO_MGR


#include "ltjs_ffmpeg_video_mgr_impl.h"
#include <utility>


namespace ltjs
{


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
	return LT_ERROR;
}

LTRESULT FfmpegVideoMgr::Impl::api_create_screen_video(
	const char* const pFilename,
	const uint32 flags,
	VideoInst*& pVideo)
{
	return LT_ERROR;
}

//
// FfmpegVideoMgr::Impl
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// FfmpegVideoInst::Impl
//

class FfmpegVideoInst::Impl
{
public:
	Impl();

	~Impl();


	void api_release();

	void api_on_render_init();

	void api_on_render_term();

	LTRESULT api_update();

	LTRESULT api_draw_video();

	LTRESULT api_get_video_status();
}; // FfmpegVideoInst::Impl


FfmpegVideoInst::Impl::Impl()
{
}

FfmpegVideoInst::Impl::~Impl()
{
}

void FfmpegVideoInst::Impl::api_release()
{
}

void FfmpegVideoInst::Impl::api_on_render_init()
{
}

void FfmpegVideoInst::Impl::api_on_render_term()
{
}

LTRESULT FfmpegVideoInst::Impl::api_update()
{
	return LT_ERROR;
}

LTRESULT FfmpegVideoInst::Impl::api_draw_video()
{
	return LT_ERROR;
}

LTRESULT FfmpegVideoInst::Impl::api_get_video_status()
{
	return LT_ERROR;
}

//
// FfmpegVideoInst::Impl
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
	const char* const pFilename,
	const uint32 flags,
	VideoInst*& pVideo)
{
	return impl_->api_create_screen_video(pFilename, flags, pVideo);
}

//
// FfmpegVideoMgr
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// FfmpegVideoInst
//

FfmpegVideoInst::FfmpegVideoInst(
	FfmpegVideoMgr* pMgr)
	:
	impl_{std::make_unique<Impl>()}
{
	static_cast<void>(pMgr);
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

void FfmpegVideoInst::Release()
{
	impl_->api_release();
}

void FfmpegVideoInst::OnRenderInit()
{
	impl_->api_on_render_init();
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
	return impl_->api_draw_video();;
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
