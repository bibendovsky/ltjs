#ifndef LTJS_OAL_LT_SOUND_SYS_INCLUDED
#define LTJS_OAL_LT_SOUND_SYS_INCLUDED


#include <chrono>
#include <string>
#include <list>
#include <mutex>
#include <thread>
#include <vector>

#include "spdlog/spdlog.h"

#include "iltsound.h"

#include "ltjs_audio_utils.h"
#include "ltjs_oal_lt_sound_sys_generic_stream.h"
#include "ltjs_oal_lt_sound_sys_streaming_source.h"
#include "ltjs_oal_lt_filter.h"
#include "ltjs_oal_system.h"
#include "ltjs_oal_utils.h"


namespace ltjs
{


class OalLtSoundSys final :
	public ILTSoundSys
{
public:
	OalLtSoundSys();

	~OalLtSoundSys();


	// -------------------------------------------------------------------------
	// ILTSoundSys

	bool Init() override;

	void Term() override;

	void* GetDDInterface(
		const uint dd_interface_id) override;

	void Lock() override;

	void Unlock() override;

	sint32 Startup() override;

	void Shutdown() override;

	std::uint32_t MsCount() override;

	sint32 SetPreference(
		const uint32 index,
		const sint32 value) override;

	sint32 GetPreference(
		const uint32 index) override;

	void MemFreeLock(
		void* storage_ptr) override;

	void* MemAllocLock(
		const uint32 storage_size) override;

	const char* LastError() override;

	sint32 WaveOutOpen(
		LHDIGDRIVER& driver,
		PHWAVEOUT& wave_out,
		const sint32 device_id,
		const ul::WaveFormatEx& wave_format) override;

	void WaveOutClose(
		LHDIGDRIVER driver_ptr) override;

	void SetDigitalMasterVolume(
		LHDIGDRIVER driver_ptr,
		const sint32 master_volume) override;

	sint32 GetDigitalMasterVolume(
		LHDIGDRIVER driver_ptr) override;

	sint32 DigitalHandleRelease(
		LHDIGDRIVER driver_ptr) override;

	sint32 DigitalHandleReacquire(
		LHDIGDRIVER driver_ptr) override;

#ifdef USE_EAX20_HARDWARE_FILTERS
	bool SetEAX20Filter(
		const bool is_enable,
		const LTSOUNDFILTERDATA& filter_data) override;

	bool SupportsEAX20Filter() override;

	bool SetEAX20BufferSettings(
		LHSAMPLE sample_handle,
		const LTSOUNDFILTERDATA& filter_data) override;
#endif // USE_EAX20_HARDWARE_FILTERS

	void Set3DProviderMinBuffers(
		const uint32 buffer_count) override;

	sint32 Open3DProvider(
		LHPROVIDER provider_id) override;

	void Close3DProvider(
		LHPROVIDER provider_id) override;

	void Set3DProviderPreference(
		LHPROVIDER provider_id,
		const char* name,
		const void* value) override;

	void Get3DProviderAttribute(
		LHPROVIDER provider_id,
		const char* name,
		void* value) override;

	sint32 Enumerate3DProviders(
		LHPROENUM& index,
		LHPROVIDER& id,
		const char*& name) override;

	LH3DPOBJECT Open3DListener(
		LHPROVIDER provider_id) override;

	void Close3DListener(
		LH3DPOBJECT listener_ptr) override;

	void SetListenerDoppler(
		LH3DPOBJECT listener_ptr,
		const float doppler_factor) override;

	void CommitDeferred() override;

	void Set3DPosition(
		LH3DPOBJECT object_ptr,
		const float x,
		const float y,
		const float z) override;

	void Set3DVelocityVector(
		LH3DPOBJECT object_ptr,
		const float dx_per_s,
		const float dy_per_s,
		const float dz_per_s) override;

	void Set3DOrientation(
		LH3DPOBJECT object_ptr,
		const float x_face,
		const float y_face,
		const float z_face,
		const float x_up,
		const float y_up,
		const float z_up) override;

	void Set3DUserData(
		LH3DPOBJECT object_ptr,
		const uint32 index,
		const std::intptr_t value) override;

	void Get3DPosition(
		LH3DPOBJECT object_ptr,
		float& x,
		float& y,
		float& z) override;

	void Get3DVelocity(
		LH3DPOBJECT object_ptr,
		float& dx_per_ms,
		float& dy_per_ms,
		float& dz_per_ms) override;

	void Get3DOrientation(
		LH3DPOBJECT object_ptr,
		float& x_face,
		float& y_face,
		float& z_face,
		float& x_up,
		float& y_up,
		float& z_up) override;

	std::intptr_t Get3DUserData(
		LH3DPOBJECT object_ptr,
		const uint32 index) override;

	LH3DSAMPLE Allocate3DSampleHandle(
		LHPROVIDER provider_id) override;

	void Release3DSampleHandle(
		LH3DSAMPLE sample_handle) override;

	void Stop3DSample(
		LH3DSAMPLE sample_handle) override;

	void Start3DSample(
		LH3DSAMPLE sample_handle) override;

	void Resume3DSample(
		LH3DSAMPLE sample_handle) override;

	void End3DSample(
		LH3DSAMPLE sample_handle) override;

	sint32 Init3DSampleFromAddress(
		LH3DSAMPLE sample_handle,
		const void* storage_ptr,
		const uint32 storage_size,
		const ul::WaveFormatEx& wave_format,
		const sint32 playback_rate,
		const LTSOUNDFILTERDATA* filter_data_ptr) override;

	sint32 Init3DSampleFromFile(
		LH3DSAMPLE sample_handle,
		const void* storage_ptr,
		const sint32 block,
		const sint32 playback_rate,
		const LTSOUNDFILTERDATA* filter_data_ptr) override;

	sint32 Get3DSampleVolume(
		LH3DSAMPLE sample_handle) override;

	void Set3DSampleVolume(
		LH3DSAMPLE sample_handle,
		const sint32 volume) override;

	uint32 Get3DSampleStatus(
		LH3DSAMPLE sample_handle) override;

	void Set3DSampleMsPosition(
		LHSAMPLE sample_handle,
		const sint32 milliseconds) override;

	sint32 Set3DSampleInfo(
		LH3DSAMPLE sample_handle,
		const LTSOUNDINFO& sound_info) override;

	void Set3DSampleDistances(
		LH3DSAMPLE sample_handle,
		const float max_distance,
		const float min_distance) override;

	void Set3DSamplePreference(
		LH3DSAMPLE sample_handle,
		const char* name,
		const void* value) override;

	void Set3DSampleLoopBlock(
		LH3DSAMPLE sample_handle,
		const sint32 loop_begin_offset,
		const sint32 loop_end_offset,
		const bool is_enable) override;

	void Set3DSampleLoop(
		LH3DSAMPLE sample_handle,
		const bool is_enable) override;

	void Set3DSampleObstruction(
		LH3DSAMPLE sample_handle,
		const float obstruction) override;

	float Get3DSampleObstruction(
		LH3DSAMPLE sample_handle) override;

	void Set3DSampleOcclusion(
		LH3DSAMPLE sample_handle,
		const float occlusion) override;

	float Get3DSampleOcclusion(
		LH3DSAMPLE sample_handle) override;

	LHSAMPLE AllocateSampleHandle(
		LHDIGDRIVER driver_ptr) override;

	void ReleaseSampleHandle(
		LHSAMPLE sample_ptr) override;

	void InitSample(
		LHSAMPLE sample_handle) override;

	void StopSample(
		LHSAMPLE sample_ptr) override;

	void StartSample(
		LHSAMPLE sample_ptr) override;

	void ResumeSample(
		LHSAMPLE sample_ptr) override;

	void EndSample(
		LHSAMPLE sample_ptr) override;

	void SetSampleVolume(
		LHSAMPLE sample_ptr,
		const sint32 volume) override;

	void SetSamplePan(
		LHSAMPLE sample_ptr,
		const sint32 pan) override;

	sint32 GetSampleVolume(
		LHSAMPLE sample_ptr) override;

	sint32 GetSamplePan(
		LHSAMPLE sample_ptr) override;

	void SetSampleUserData(
		LHSAMPLE sample_handle,
		const uint32 index,
		const std::intptr_t value) override;

	void GetDirectSoundInfo(
		LHSAMPLE sample_handle,
		PTDIRECTSOUND& ds_instance,
		PTDIRECTSOUNDBUFFER& ds_buffer) override;

	void SetSampleReverb(
		LHSAMPLE sample_handle,
		const float reverb_level,
		const float reverb_reflect_time,
		const float reverb_decay_time) override;

	sint32 InitSampleFromAddress(
		LHSAMPLE sample_handle,
		const void* storage_ptr,
		const uint32 storage_size,
		const ul::WaveFormatEx& wave_format,
		const sint32 playback_rate,
		const LTSOUNDFILTERDATA* filter_data_ptr) override;

	sint32 InitSampleFromFile(
		LHSAMPLE sample_handle,
		const void* storage_ptr,
		const sint32 block,
		const sint32 playback_rate,
		const LTSOUNDFILTERDATA* filter_data_ptr) override;

	void SetSampleLoopBlock(
		LHSAMPLE sample_handle,
		const sint32 loop_begin_offset,
		const sint32 loop_end_offset,
		const bool is_enable) override;

	void SetSampleLoop(
		LHSAMPLE sample_handle,
		const bool is_enable) override;

	void SetSampleMsPosition(
		LHSAMPLE sample_handle,
		const sint32 milliseconds) override;

	std::intptr_t GetSampleUserData(
		LHSAMPLE sample_handle,
		const uint32 index) override;

	uint32 GetSampleStatus(
		LHSAMPLE sample_handle) override;

	LHSTREAM OpenStream(
		const char* file_name,
		const uint32 file_offset,
		LHDIGDRIVER driver_ptr,
		const char* storage_ptr,
		const sint32 storage_size) override;

	void SetStreamLoop(
		LHSTREAM stream_ptr,
		const bool is_enable) override;

	void SetStreamPlaybackRate(
		LHSTREAM stream_ptr,
		const sint32 rate) override;

	void SetStreamMsPosition(
		LHSTREAM stream_ptr,
		const sint32 milliseconds) override;

	void SetStreamUserData(
		LHSTREAM stream_ptr,
		const uint32 index,
		const std::intptr_t value) override;

	std::intptr_t GetStreamUserData(
		LHSTREAM stream_ptr,
		const uint32 index) override;

	void CloseStream(
		LHSTREAM stream_ptr) override;

	void StartStream(
		LHSTREAM stream_ptr) override;

	void PauseStream(
		LHSTREAM stream_ptr,
		const sint32 is_enable) override;

	void ResetStream(
		LHSTREAM stream_ptr) override;

	void SetStreamVolume(
		LHSTREAM stream_ptr,
		const sint32 volume) override;

	void SetStreamPan(
		LHSTREAM stream_ptr,
		const sint32 pan) override;

	sint32 GetStreamVolume(
		LHSTREAM stream_ptr) override;

	sint32 GetStreamPan(
		LHSTREAM stream_ptr) override;

	uint32 GetStreamStatus(
		LHSTREAM stream_ptr) override;

	sint32 GetStreamBufferParam(
		LHSTREAM stream_ptr,
		const uint32 index) override;

	void ClearStreamBuffer(
		LHSTREAM stream_ptr,
		const bool is_clear_data_queue) override;

	sint32 DecompressADPCM(
		const LTSOUNDINFO& sound_info,
		void*& dst_data,
		uint32& dst_size) override;

	sint32 DecompressASI(
		const void* src_data_ptr,
		const uint32 src_data_size,
		const char* file_name_ext,
		void*& ds_wav_ptr,
		uint32& ds_wav_size,
		LTLENGTHYCB callback) override;

	uint32 GetThreadedSoundTicks() override;

	bool HasOnBoardMemory() override;

	void ltjs_handle_focus_lost(
		bool is_focus_lost) override;

	int ltjs_get_generic_stream_queue_size() noexcept override;

	LtjsLtSoundSysGenericStream* ltjs_open_generic_stream(
		int sample_rate,
		int buffer_size) noexcept override;

	void ltjs_close_generic_stream(
		LtjsLtSoundSysGenericStream* generic_stream) noexcept override;

	// ILTSoundSys
	// -------------------------------------------------------------------------


private:
	using String = std::string;
	using ExtensionsStrings = std::vector<String>;

	using Clock = std::chrono::system_clock;
	using ClockTs = Clock::time_point;

	using MtThread = std::thread;
	using MtMutex = std::mutex;
	using MtMutexGuard = std::lock_guard<MtMutex>;
	using MtUniqueLock = std::unique_lock<MtMutex>;
	using MtCondVar = std::condition_variable;

	using GenericStreams = std::list<OalLtSoundSysGenericStream>;
	using StreamingSourceUPtr = std::unique_ptr<OalLtSoundSysStreamingSource>;
	using Sources = std::list<OalLtSoundSysStreamingSource>;
	using OpenSources = std::list<OalLtSoundSysStreamingSource*>;


	std::shared_ptr<spdlog::logger> logger_{};

	String error_message_;
	oal::SystemUPtr system_;
	oal::LtFilterUPtr lt_filter_;

	AudioDecoder audio_decoder_;

	ClockTs clock_base_;

	Sources samples_;
	MtMutex mt_samples_mutex_;
	OpenSources mt_open_samples_;

	StreamingSourceUPtr listener_3d_uptr_;
	Sources objects_3d_;
	MtMutex mt_3d_objects_mutex_;
	OpenSources mt_open_3d_objects_;

	Sources streams_;
	MtMutex mt_streams_mutex_;
	OpenSources mt_open_streams_;

	GenericStreams generic_streams_;
	MtMutex mt_generic_streams_mutex_;

	MtThread mt_sound_thread_;
	MtCondVar mt_sound_cv_;
	MtMutex mt_sound_cv_mutex_;

	bool mt_is_stop_sound_worker_;
	bool mt_sound_cv_flag_;


	// -------------------------------------------------------------------------
	// Utils

	void uninitialize_streaming();

	void initialize_streaming();

	void mt_notify_sound();

	void mt_wait_for_sound_cv();

	void sound_worker();

	void initialize_lt_filter_for_source(
		OalLtSoundSysStreamingSource& source,
		const LTSOUNDFILTERDATA* lt_filter_data);

	// Utils
	// -------------------------------------------------------------------------


	// -------------------------------------------------------------------------
	// API utils

	bool wave_out_open_internal();

	void wave_out_close_internal();

	// API utils
	// -------------------------------------------------------------------------
}; // OalLtSoundSys


} // ltjs


#endif // !LTJS_OAL_LT_SOUND_SYS_INCLUDED
