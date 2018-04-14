#ifndef LTJS_S_OAL_INCLUDED
#define LTJS_S_OAL_INCLUDED


#include <memory>
#include "bibendovsky_spul_wave_format.h"
#include "iltsound.h"


namespace ltjs
{
namespace ul = bibendovsky::spul;
} // ltjs


class OalSoundSys :
	public ILTSoundSys
{
public:
	OalSoundSys();

	OalSoundSys(
		OalSoundSys&& that);

	~OalSoundSys() override;


	bool Init() override;

	void Term() override;


	void* GetDDInterface(
		const uint dd_interface_id) override;


	// system wide functions
	//
	void Lock() override;

	void Unlock() override;

	sint32 Startup() override;

	void Shutdown() override;

	uint32 MsCount() override;

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


	// digital sound driver functions
	//
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


	// 3D sound provider functions
	//
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


	// 3D listener_ptr functions
	//
	LH3DPOBJECT Open3DListener(
		LHPROVIDER provider_id) override;

	void Close3DListener(
		LH3DPOBJECT listener_ptr) override;

	void SetListenerDoppler(
		LH3DPOBJECT listener_ptr,
		const float doppler_factor) override;

	void CommitDeferred() override;


	// 3D sound object functions
	//
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
		const sint32 value) override;

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

	sint32 Get3DUserData(
		LH3DPOBJECT object_ptr,
		const uint32 index) override;


	// 3D sound sample functions
	//
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
		const void* ptr,
		const uint32 length,
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
		const LTSOUNDINFO& sound_info_ptr) override;

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


	// 2D sound sample functions
	//
	LHSAMPLE AllocateSampleHandle(
		LHDIGDRIVER driver_ptr) override;

	void ReleaseSampleHandle(
		LHSAMPLE sample_handle) override;

	void InitSample(
		LHSAMPLE sample_handle) override;

	void StopSample(
		LHSAMPLE sample_handle) override;

	void StartSample(
		LHSAMPLE sample_handle) override;

	void ResumeSample(
		LHSAMPLE sample_handle) override;

	void EndSample(
		LHSAMPLE sample_handle) override;

	void SetSampleVolume(
		LHSAMPLE sample_handle,
		const sint32 volume) override;

	void SetSamplePan(
		LHSAMPLE sample_handle,
		const sint32 pan) override;

	sint32 GetSampleVolume(
		LHSAMPLE sample_handle) override;

	sint32 GetSamplePan(
		LHSAMPLE sample_handle) override;

	void SetSampleUserData(
		LHSAMPLE sample_handle,
		const uint32 index,
		const sint32 value) override;

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
		const void* ptr,
		const uint32 length,
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

	sint32 GetSampleUserData(
		LHSAMPLE sample_handle,
		const uint32 index) override;

	uint32 GetSampleStatus(
		LHSAMPLE sample_handle) override;


	// old 2D sound stream functions
	//
	LHSTREAM OpenStream(
		const char* file_name,
		const uint32 file_offset,
		LHDIGDRIVER driver_ptr,
		const char* file_image,
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
		const sint32 value) override;

	sint32 GetStreamUserData(
		LHSTREAM stream_ptr,
		const uint32 index) override;


	// new 2D sound stream functions
	//
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
		const bool is_clear_data_queue = true) override;


	// wave file decompression functons
	//
	sint32 DecompressADPCM(
		const LTSOUNDINFO& sound_info,
		void*& output_data,
		uint32& output_size) override;

	sint32 DecompressASI(
		const void* src_data_ptr,
		const uint32 src_data_size,
		const char* file_name_ext,
		void*& dst_wav_ptr,
		uint32& dst_wav_size,
		LTLENGTHYCB callback) override;


	// Gets the ticks spent in sound thread.
	uint32 GetThreadedSoundTicks() override;


	bool HasOnBoardMemory() override;

	void handle_focus_lost(
		const bool is_focus_lost) override;


	static OalSoundSys& get_singleton();


private:
	struct Impl;

	using ImplUPtr = std::unique_ptr<Impl>;

	ImplUPtr pimpl_;
}; // OalSoundSys


#endif // LTJS_S_OAL_INCLUDED
