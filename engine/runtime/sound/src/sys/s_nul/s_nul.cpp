#include "s_nul.h"


NulSoundSys::NulSoundSys()
{
}

NulSoundSys::~NulSoundSys()
{
}

bool NulSoundSys::Init()
{
	return true;
}

void NulSoundSys::Term()
{
}

void* NulSoundSys::GetDDInterface(
	const uint dd_interface_id)
{
	static_cast<void>(dd_interface_id);

	return {};
}

void NulSoundSys::Lock()
{
}

void NulSoundSys::Unlock()
{
}

sint32 NulSoundSys::Startup()
{
	return {};
}

void NulSoundSys::Shutdown()
{
}

uint32 NulSoundSys::MsCount()
{
	return {};
}

sint32 NulSoundSys::SetPreference(
	const uint32 number,
	const sint32 value)
{
	static_cast<void>(number);
	static_cast<void>(value);

	return {};
}

sint32 NulSoundSys::GetPreference(
	const uint32 number)
{
	static_cast<void>(number);

	return {};
}

void NulSoundSys::MemFreeLock(
	void* ptr)
{
	::LTMemFree(ptr);
}

void* NulSoundSys::MemAllocLock(
	const uint32 size)
{
	return ::LTMemAlloc(size);
}

const char* NulSoundSys::LastError()
{
	static auto last_error = "";
	return last_error;
}

sint32 NulSoundSys::WaveOutOpen(
	LHDIGDRIVER* driver_ptr,
	PHWAVEOUT* wave_out_ptr,
	const sint32 device_id,
	const ul::WaveFormat* wave_format_ptr)
{
	static_cast<void>(driver_ptr);
	static_cast<void>(wave_out_ptr);
	static_cast<void>(device_id);
	static_cast<void>(wave_format_ptr);

	return {};
}

void NulSoundSys::WaveOutClose(
	LHDIGDRIVER driver_ptr)
{
	static_cast<void>(driver_ptr);
}

void NulSoundSys::SetDigitalMasterVolume(
	LHDIGDRIVER driver_ptr,
	const sint32 master_volume)
{
	static_cast<void>(driver_ptr);
	static_cast<void>(master_volume);
}

sint32 NulSoundSys::GetDigitalMasterVolume(
	LHDIGDRIVER driver_ptr)
{
	static_cast<void>(driver_ptr);

	return {};
}

sint32 NulSoundSys::DigitalHandleRelease(
	LHDIGDRIVER driver_ptr)
{
	static_cast<void>(driver_ptr);

	return {};
}

sint32 NulSoundSys::DigitalHandleReacquire(
	LHDIGDRIVER driver_ptr)
{
	static_cast<void>(driver_ptr);

	return {};
}

bool NulSoundSys::SetEAX20Filter(
	const bool is_enable,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	static_cast<void>(is_enable);
	static_cast<void>(filter_data_ptr);

	return {};
}

bool NulSoundSys::SupportsEAX20Filter()
{
	return {};
}

bool NulSoundSys::SetEAX20BufferSettings(
	LHSAMPLE sample_handle,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	static_cast<void>(sample_handle);
	static_cast<void>(filter_data_ptr);

	return {};
}

void NulSoundSys::Set3DProviderMinBuffers(
	const uint32 min_buffers)
{
	static_cast<void>(min_buffers);
}

sint32 NulSoundSys::Open3DProvider(
	LHPROVIDER provider_id)
{
	static_cast<void>(provider_id);

	return {};
}

void NulSoundSys::Close3DProvider(
	LHPROVIDER provider_id)
{
	static_cast<void>(provider_id);
}

void NulSoundSys::Set3DProviderPreference(
	LHPROVIDER provider_id,
	const char* name,
	const void* value)
{
	static_cast<void>(provider_id);
	static_cast<void>(name);
	static_cast<void>(value);
}

void NulSoundSys::Get3DProviderAttribute(
	LHPROVIDER provider_id,
	const char* name,
	void* value)
{
	static_cast<void>(provider_id);
	static_cast<void>(name);
	static_cast<void>(value);
}

sint32 NulSoundSys::Enumerate3DProviders(
	LHPROENUM* next_ptr,
	LHPROVIDER* destination_ptr,
	const char** name)
{
	static_cast<void>(next_ptr);
	static_cast<void>(destination_ptr);
	static_cast<void>(name);

	return {};
}

LH3DPOBJECT NulSoundSys::Open3DListener(
	LHPROVIDER provider_id)
{
	static_cast<void>(provider_id);

	return {};
}

void NulSoundSys::Close3DListener(
	LH3DPOBJECT listener_ptr)
{
	static_cast<void>(listener_ptr);
}

void NulSoundSys::SetListenerDoppler(
	LH3DPOBJECT listener_ptr,
	const float doppler)
{
	static_cast<void>(listener_ptr);
	static_cast<void>(doppler);
}

void NulSoundSys::CommitDeferred()
{
}

void NulSoundSys::Set3DPosition(
	LH3DPOBJECT object_ptr,
	const float x,
	const float y,
	const float z)
{
	static_cast<void>(object_ptr);
	static_cast<void>(x);
	static_cast<void>(y);
	static_cast<void>(z);
}

void NulSoundSys::Set3DVelocityVector(
	LH3DPOBJECT object_ptr,
	const float dx_per_s,
	const float dy_per_s,
	const float dz_per_s)
{
	static_cast<void>(object_ptr);
	static_cast<void>(dx_per_s);
	static_cast<void>(dy_per_s);
	static_cast<void>(dz_per_s);
}

void NulSoundSys::Set3DOrientation(
	LH3DPOBJECT object_ptr,
	const float x_face,
	const float y_face,
	const float z_face,
	const float x_up,
	const float y_up,
	const float z_up)
{
	static_cast<void>(object_ptr);
	static_cast<void>(x_face);
	static_cast<void>(y_face);
	static_cast<void>(z_face);
	static_cast<void>(x_up);
	static_cast<void>(y_up);
	static_cast<void>(z_up);
}

void NulSoundSys::Set3DUserData(
	LH3DPOBJECT object_ptr,
	const uint32 index,
	const sint32 value)
{
	static_cast<void>(object_ptr);
	static_cast<void>(index);
	static_cast<void>(value);
}

void NulSoundSys::Get3DPosition(
	LH3DPOBJECT object_ptr,
	float* x_ptr,
	float* y_ptr,
	float* z_ptr)
{
	static_cast<void>(object_ptr);
	static_cast<void>(x_ptr);
	static_cast<void>(y_ptr);
	static_cast<void>(z_ptr);
}

void NulSoundSys::Get3DVelocity(
	LH3DPOBJECT object_ptr,
	float* dx_per_ms_ptr,
	float* dy_per_ms_ptr,
	float* dz_per_ms_ptr)
{
	static_cast<void>(object_ptr);
	static_cast<void>(dx_per_ms_ptr);
	static_cast<void>(dy_per_ms_ptr);
	static_cast<void>(dz_per_ms_ptr);
}

void NulSoundSys::Get3DOrientation(
	LH3DPOBJECT object_ptr,
	float* x_face_ptr,
	float* y_face_ptr,
	float* z_face_ptr,
	float* x_up_ptr,
	float* y_up_ptr,
	float* z_up_ptr)
{
	static_cast<void>(object_ptr);
	static_cast<void>(x_face_ptr);
	static_cast<void>(y_face_ptr);
	static_cast<void>(z_face_ptr);
	static_cast<void>(x_up_ptr);
	static_cast<void>(y_up_ptr);
	static_cast<void>(z_up_ptr);
}

sint32 NulSoundSys::Get3DUserData(
	LH3DPOBJECT object_ptr,
	const uint32 index)
{
	static_cast<void>(object_ptr);
	static_cast<void>(index);

	return {};
}

LH3DSAMPLE NulSoundSys::Allocate3DSampleHandle(
	LHPROVIDER driver_id)
{
	static_cast<void>(driver_id);

	return {};
}

void NulSoundSys::Release3DSampleHandle(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void NulSoundSys::Stop3DSample(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void NulSoundSys::Start3DSample(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void NulSoundSys::Resume3DSample(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void NulSoundSys::End3DSample(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

sint32 NulSoundSys::Init3DSampleFromAddress(
	LH3DSAMPLE sample_handle,
	const void* ptr,
	const uint32 length,
	const ul::WaveFormatEx* wave_format_ptr,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	static_cast<void>(sample_handle);
	static_cast<void>(ptr);
	static_cast<void>(length);
	static_cast<void>(wave_format_ptr);
	static_cast<void>(playback_rate);
	static_cast<void>(filter_data_ptr);

	return {};
}

sint32 NulSoundSys::Init3DSampleFromFile(
	LH3DSAMPLE sample_handle,
	const void* file_image_ptr,
	const sint32 block,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	static_cast<void>(sample_handle);
	static_cast<void>(file_image_ptr);
	static_cast<void>(block);
	static_cast<void>(playback_rate);
	static_cast<void>(filter_data_ptr);

	return {};
}

sint32 NulSoundSys::Get3DSampleVolume(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

void NulSoundSys::Set3DSampleVolume(
	LH3DSAMPLE sample_handle,
	const sint32 volume)
{
	static_cast<void>(sample_handle);
	static_cast<void>(volume);
}

uint32 NulSoundSys::Get3DSampleStatus(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

void NulSoundSys::Set3DSampleMsPosition(
	LHSAMPLE sample_handle,
	const sint32 milliseconds)
{
	static_cast<void>(sample_handle);
	static_cast<void>(milliseconds);
}

sint32 NulSoundSys::Set3DSampleInfo(
	LH3DSAMPLE sample_handle,
	const LTSOUNDINFO* sound_info_ptr)
{
	static_cast<void>(sample_handle);
	static_cast<void>(sound_info_ptr);

	return {};
}

void NulSoundSys::Set3DSampleDistances(
	LH3DSAMPLE sample_handle,
	const float max_distance,
	const float min_distance)
{
	static_cast<void>(sample_handle);
	static_cast<void>(max_distance);
	static_cast<void>(min_distance);
}

void NulSoundSys::Set3DSamplePreference(
	LH3DSAMPLE sample_handle,
	const char* name,
	const void* value)
{
	static_cast<void>(sample_handle);
	static_cast<void>(name);
	static_cast<void>(value);
}

void NulSoundSys::Set3DSampleLoopBlock(
	LH3DSAMPLE sample_handle,
	const sint32 loop_start_offset,
	const sint32 loop_end_offset,
	const bool is_enable)
{
	static_cast<void>(sample_handle);
	static_cast<void>(loop_start_offset);
	static_cast<void>(loop_end_offset);
	static_cast<void>(is_enable);
}

void NulSoundSys::Set3DSampleLoop(
	LH3DSAMPLE sample_handle,
	const bool is_loop)
{
	static_cast<void>(sample_handle);
	static_cast<void>(is_loop);
}

void NulSoundSys::Set3DSampleObstruction(
	LH3DSAMPLE sample_handle,
	const float obstruction)
{
	static_cast<void>(sample_handle);
	static_cast<void>(obstruction);
}

float NulSoundSys::Get3DSampleObstruction(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

void NulSoundSys::Set3DSampleOcclusion(
	LH3DSAMPLE sample_handle,
	const float occlusion)
{
	static_cast<void>(sample_handle);
	static_cast<void>(occlusion);
}

float NulSoundSys::Get3DSampleOcclusion(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

LHSAMPLE NulSoundSys::AllocateSampleHandle(
	LHDIGDRIVER driver_ptr)
{
	static_cast<void>(driver_ptr);

	return {};
}

void NulSoundSys::ReleaseSampleHandle(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void NulSoundSys::InitSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void NulSoundSys::StopSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void NulSoundSys::StartSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void NulSoundSys::ResumeSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void NulSoundSys::EndSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void NulSoundSys::SetSampleVolume(
	LHSAMPLE sample_handle,
	const sint32 volume)
{
	static_cast<void>(sample_handle);
	static_cast<void>(volume);
}

void NulSoundSys::SetSamplePan(
	LHSAMPLE sample_handle,
	const sint32 pan)
{
	static_cast<void>(sample_handle);
	static_cast<void>(pan);
}

sint32 NulSoundSys::GetSampleVolume(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

sint32 NulSoundSys::GetSamplePan(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

void NulSoundSys::SetSampleUserData(
	LHSAMPLE sample_handle,
	const uint32 index,
	const sint32 value)
{
	static_cast<void>(sample_handle);
	static_cast<void>(index);
	static_cast<void>(value);
}

void NulSoundSys::GetDirectSoundInfo(
	LHSAMPLE sample_handle,
	PTDIRECTSOUND* direct_sound_ptr,
	PTDIRECTSOUNDBUFFER* direct_sound_buffer_ptr)
{
	static_cast<void>(sample_handle);
	static_cast<void>(direct_sound_ptr);
	static_cast<void>(direct_sound_buffer_ptr);
}

void NulSoundSys::SetSampleReverb(
	LHSAMPLE sample_handle,
	const float reverb_level,
	const float reverb_reflect_time,
	const float reverb_decay_time)
{
	static_cast<void>(sample_handle);
	static_cast<void>(reverb_level);
	static_cast<void>(reverb_reflect_time);
	static_cast<void>(reverb_decay_time);
}

sint32 NulSoundSys::InitSampleFromAddress(
	LHSAMPLE sample_handle,
	const void* ptr,
	const uint32 length,
	const ul::WaveFormatEx* wave_format_ptr,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	static_cast<void>(sample_handle);
	static_cast<void>(ptr);
	static_cast<void>(length);
	static_cast<void>(wave_format_ptr);
	static_cast<void>(playback_rate);
	static_cast<void>(filter_data_ptr);

	return {};
}

sint32 NulSoundSys::InitSampleFromFile(
	LHSAMPLE sample_handle,
	const void* file_image_ptr,
	const sint32 block,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	static_cast<void>(sample_handle);
	static_cast<void>(file_image_ptr);
	static_cast<void>(block);
	static_cast<void>(playback_rate);
	static_cast<void>(filter_data_ptr);

	return {};
}

void NulSoundSys::SetSampleLoopBlock(
	LHSAMPLE sample_handle,
	const sint32 loop_start_offset,
	const sint32 loop_end_offset,
	const bool is_enable)
{
	static_cast<void>(sample_handle);
	static_cast<void>(loop_start_offset);
	static_cast<void>(loop_end_offset);
	static_cast<void>(is_enable);
}

void NulSoundSys::SetSampleLoop(
	LHSAMPLE sample_handle,
	const bool is_loop)
{
	static_cast<void>(sample_handle);
	static_cast<void>(is_loop);
}

void NulSoundSys::SetSampleMsPosition(
	LHSAMPLE sample_handle,
	const sint32 milliseconds)
{
	static_cast<void>(sample_handle);
	static_cast<void>(milliseconds);
}

sint32 NulSoundSys::GetSampleUserData(
	LHSAMPLE sample_handle,
	const uint32 index)
{
	static_cast<void>(sample_handle);
	static_cast<void>(index);

	return {};
}

uint32 NulSoundSys::GetSampleStatus(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

LHSTREAM NulSoundSys::OpenStream(
	const char* file_name,
	const uint32 file_offset,
	LHDIGDRIVER driver_ptr,
	const char* stream_ptr,
	const sint32 stream_memory_size)
{
	static_cast<void>(file_name);
	static_cast<void>(file_offset);
	static_cast<void>(driver_ptr);
	static_cast<void>(stream_ptr);
	static_cast<void>(stream_memory_size);

	return {};
}

void NulSoundSys::SetStreamLoop(
	LHSTREAM stream_ptr,
	const bool is_loop)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(is_loop);
}

void NulSoundSys::SetStreamPlaybackRate(
	LHSTREAM stream_ptr,
	const sint32 rate)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(rate);
}

void NulSoundSys::SetStreamMsPosition(
	LHSTREAM stream_ptr,
	const sint32 milliseconds)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(milliseconds);
}

void NulSoundSys::SetStreamUserData(
	LHSTREAM stream_ptr,
	const uint32 index,
	const sint32 value)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(index);
	static_cast<void>(value);
}

sint32 NulSoundSys::GetStreamUserData(
	LHSTREAM stream_ptr,
	const uint32 index)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(index);

	return {};
}

void NulSoundSys::CloseStream(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);
}

void NulSoundSys::StartStream(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);
}

void NulSoundSys::PauseStream(
	LHSTREAM stream_ptr,
	const sint32 is_pause)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(is_pause);
}

void NulSoundSys::ResetStream(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);
}

void NulSoundSys::SetStreamVolume(
	LHSTREAM stream_ptr,
	const sint32 volume)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(volume);
}

void NulSoundSys::SetStreamPan(
	LHSTREAM stream_ptr,
	const sint32 pan)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(pan);
}

sint32 NulSoundSys::GetStreamVolume(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);

	return {};
}

sint32 NulSoundSys::GetStreamPan(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);

	return {};
}

uint32 NulSoundSys::GetStreamStatus(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);

	return {};
}

sint32 NulSoundSys::GetStreamBufferParam(
	LHSTREAM stream_ptr,
	const uint32 param)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(param);

	return {};
}

void NulSoundSys::ClearStreamBuffer(
	LHSTREAM stream_ptr,
	const bool is_clear_data_queue)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(is_clear_data_queue);
}

sint32 NulSoundSys::DecompressADPCM(
	const LTSOUNDINFO* sound_info_ptr,
	void** dst_data_ptr,
	uint32* dst_size_ptr)
{
	static_cast<void>(sound_info_ptr);
	static_cast<void>(dst_data_ptr);
	static_cast<void>(dst_size_ptr);

	return {};
}

sint32 NulSoundSys::DecompressASI(
	const void* srd_data_ptr,
	const uint32 src_size,
	const char* file_name_ext,
	void** dst_wav_ptr,
	uint32* dst_wav_size_ptr,
	LTLENGTHYCB callback)
{
	static_cast<void>(srd_data_ptr);
	static_cast<void>(src_size);
	static_cast<void>(file_name_ext);
	static_cast<void>(dst_wav_ptr);
	static_cast<void>(dst_wav_size_ptr);
	static_cast<void>(callback);

	return {};
}

uint32 NulSoundSys::GetThreadedSoundTicks()
{
	return {};
}

bool NulSoundSys::HasOnBoardMemory()
{
	return {};
}

NulSoundSys& NulSoundSys::get_singleton()
{
	static auto singleton = NulSoundSys{};
	return singleton;
}

extern "C"
{
	__declspec(dllexport) char* SoundSysDesc();
	__declspec(dllexport) ILTSoundSys* SoundSysMake();
}

char* SoundSysDesc()
{
	static char* description = const_cast<char*>("*** null sound driver ***");
	return description;
}

ILTSoundSys* SoundSysMake()
{
	return &NulSoundSys::get_singleton();
}
