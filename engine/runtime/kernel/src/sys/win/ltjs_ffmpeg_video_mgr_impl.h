#ifndef LTJS_FFMPEG_VIDEO_NGR_IMPL_INCLUDED
#define LTJS_FFMPEG_VIDEO_NGR_IMPL_INCLUDED


#ifdef LTJS_USE_FFMPEG_VIDEO_MGR


#include <memory>
#include "ltbasetypes.h"
#include "videomgr.h"


namespace ltjs
{


class FfmpegVideoMgr final :
	public VideoMgr
{
public:
	FfmpegVideoMgr();

	FfmpegVideoMgr(
		FfmpegVideoMgr&& that);

	virtual ~FfmpegVideoMgr();


	LTRESULT initialize();

	LTRESULT CreateScreenVideo(
		const char* const file_name,
		const uint32 flags,
		VideoInst*& video_inst_ptr) override;


private:
	class Impl;

	using ImplUPtr = std::unique_ptr<Impl>;

	ImplUPtr impl_;
};


class FfmpegVideoInst final :
	public VideoInst
{
public:
	FfmpegVideoInst(
		VideoMgr* video_mgr_ptr);

	FfmpegVideoInst(
		FfmpegVideoInst&& that);

	virtual ~FfmpegVideoInst();


	bool initialize(
		const char* const file_name);


	void Release() override;

	void OnRenderInit() override;

	void OnRenderTerm() override;

	LTRESULT Update() override;

	LTRESULT DrawVideo() override;

	LTRESULT GetVideoStatus() override;


private:
	class Impl;

	using ImplUPtr = std::unique_ptr<Impl>;

	ImplUPtr impl_;
};


} // ltjs


#endif // LTJS_USE_FFMPEG_VIDEO_MGR


#endif // LTJS_FFMPEG_VIDEO_NGR_IMPL_INCLUDED
