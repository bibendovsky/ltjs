#include "s_oal.h"


OalSoundSys::OalSoundSys()
{
}

OalSoundSys::~OalSoundSys()
{
}

bool OalSoundSys::Init()
{
	return true;
}

void OalSoundSys::Term()
{
}

void* OalSoundSys::GetDDInterface(
	const uint dd_interface_id)
{
	static_cast<void>(dd_interface_id);

	return {};
}

void OalSoundSys::Lock()
{
}

void OalSoundSys::Unlock()
{
}

sint32 OalSoundSys::Startup()
{
	return {};
}

void OalSoundSys::Shutdown()
{
}

uint32 OalSoundSys::MsCount()
{
	return {};
}

sint32 OalSoundSys::SetPreference(
	const uint32 number,
	const sint32 value)
{
	static_cast<void>(number);
	static_cast<void>(value);

	return {};
}

sint32 OalSoundSys::GetPreference(
	const uint32 number)
{
	static_cast<void>(number);

	return {};
}

void OalSoundSys::MemFreeLock(
	void* ptr)
{
	::LTMemFree(ptr);
}

void* OalSoundSys::MemAllocLock(
	const uint32 size)
{
	return ::LTMemAlloc(size);
}

const char* OalSoundSys::LastError()
{
	static auto last_error = "";
	return last_error;
}

sint32 OalSoundSys::WaveOutOpen(
	LHDIGDRIVER& driver,
	PHWAVEOUT& wave_out,
	const sint32 device_id,
	const ul::WaveFormatEx& wave_format)
{
	static_cast<void>(driver);
	static_cast<void>(wave_out);
	static_cast<void>(device_id);
	static_cast<void>(wave_format);

	return {};
}

void OalSoundSys::WaveOutClose(
	LHDIGDRIVER driver_ptr)
{
	static_cast<void>(driver_ptr);
}

void OalSoundSys::SetDigitalMasterVolume(
	LHDIGDRIVER driver_ptr,
	const sint32 master_volume)
{
	static_cast<void>(driver_ptr);
	static_cast<void>(master_volume);
}

sint32 OalSoundSys::GetDigitalMasterVolume(
	LHDIGDRIVER driver_ptr)
{
	static_cast<void>(driver_ptr);

	return {};
}

sint32 OalSoundSys::DigitalHandleRelease(
	LHDIGDRIVER driver_ptr)
{
	static_cast<void>(driver_ptr);

	return {};
}

sint32 OalSoundSys::DigitalHandleReacquire(
	LHDIGDRIVER driver_ptr)
{
	static_cast<void>(driver_ptr);

	return {};
}

bool OalSoundSys::SetEAX20Filter(
	const bool is_enable,
	const LTSOUNDFILTERDATA& filter_data)
{
	static_cast<void>(is_enable);
	static_cast<void>(filter_data);

	return {};
}

bool OalSoundSys::SupportsEAX20Filter()
{
	return {};
}

bool OalSoundSys::SetEAX20BufferSettings(
	LHSAMPLE sample_handle,
	const LTSOUNDFILTERDATA& filter_data)
{
	static_cast<void>(sample_handle);
	static_cast<void>(filter_data);

	return {};
}

void OalSoundSys::Set3DProviderMinBuffers(
	const uint32 min_buffers)
{
	static_cast<void>(min_buffers);
}

sint32 OalSoundSys::Open3DProvider(
	LHPROVIDER provider_id)
{
	static_cast<void>(provider_id);

	return {};
}

void OalSoundSys::Close3DProvider(
	LHPROVIDER provider_id)
{
	static_cast<void>(provider_id);
}

void OalSoundSys::Set3DProviderPreference(
	LHPROVIDER provider_id,
	const char* name,
	const void* value)
{
	static_cast<void>(provider_id);
	static_cast<void>(name);
	static_cast<void>(value);
}

void OalSoundSys::Get3DProviderAttribute(
	LHPROVIDER provider_id,
	const char* name,
	void* value)
{
	static_cast<void>(provider_id);
	static_cast<void>(name);
	static_cast<void>(value);
}

sint32 OalSoundSys::Enumerate3DProviders(
	LHPROENUM& next,
	LHPROVIDER& destination,
	const char*& name)
{
	static_cast<void>(next);
	static_cast<void>(destination);
	static_cast<void>(name);

	return {};
}

LH3DPOBJECT OalSoundSys::Open3DListener(
	LHPROVIDER provider_id)
{
	static_cast<void>(provider_id);

	return {};
}

void OalSoundSys::Close3DListener(
	LH3DPOBJECT listener_ptr)
{
	static_cast<void>(listener_ptr);
}

void OalSoundSys::SetListenerDoppler(
	LH3DPOBJECT listener_ptr,
	const float doppler)
{
	static_cast<void>(listener_ptr);
	static_cast<void>(doppler);
}

void OalSoundSys::CommitDeferred()
{
}

void OalSoundSys::Set3DPosition(
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

void OalSoundSys::Set3DVelocityVector(
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

void OalSoundSys::Set3DOrientation(
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

void OalSoundSys::Set3DUserData(
	LH3DPOBJECT object_ptr,
	const uint32 index,
	const sint32 value)
{
	static_cast<void>(object_ptr);
	static_cast<void>(index);
	static_cast<void>(value);
}

void OalSoundSys::Get3DPosition(
	LH3DPOBJECT object_ptr,
	float& x,
	float& y,
	float& z)
{
	static_cast<void>(object_ptr);
	static_cast<void>(x);
	static_cast<void>(y);
	static_cast<void>(z);
}

void OalSoundSys::Get3DVelocity(
	LH3DPOBJECT object_ptr,
	float& dx_per_ms,
	float& dy_per_ms,
	float& dz_per_ms)
{
	static_cast<void>(object_ptr);
	static_cast<void>(dx_per_ms);
	static_cast<void>(dy_per_ms);
	static_cast<void>(dz_per_ms);
}

void OalSoundSys::Get3DOrientation(
	LH3DPOBJECT object_ptr,
	float& x_face,
	float& y_face,
	float& z_face,
	float& x_up,
	float& y_up,
	float& z_up)
{
	static_cast<void>(object_ptr);
	static_cast<void>(x_face);
	static_cast<void>(y_face);
	static_cast<void>(z_face);
	static_cast<void>(x_up);
	static_cast<void>(y_up);
	static_cast<void>(z_up);
}

sint32 OalSoundSys::Get3DUserData(
	LH3DPOBJECT object_ptr,
	const uint32 index)
{
	static_cast<void>(object_ptr);
	static_cast<void>(index);

	return {};
}

LH3DSAMPLE OalSoundSys::Allocate3DSampleHandle(
	LHPROVIDER driver_id)
{
	static_cast<void>(driver_id);

	return {};
}

void OalSoundSys::Release3DSampleHandle(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::Stop3DSample(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::Start3DSample(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::Resume3DSample(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::End3DSample(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

sint32 OalSoundSys::Init3DSampleFromAddress(
	LH3DSAMPLE sample_handle,
	const void* ptr,
	const uint32 length,
	const ul::WaveFormatEx& wave_format,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	static_cast<void>(sample_handle);
	static_cast<void>(ptr);
	static_cast<void>(length);
	static_cast<void>(wave_format);
	static_cast<void>(playback_rate);
	static_cast<void>(filter_data_ptr);

	return {};
}

sint32 OalSoundSys::Init3DSampleFromFile(
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

sint32 OalSoundSys::Get3DSampleVolume(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

void OalSoundSys::Set3DSampleVolume(
	LH3DSAMPLE sample_handle,
	const sint32 volume)
{
	static_cast<void>(sample_handle);
	static_cast<void>(volume);
}

uint32 OalSoundSys::Get3DSampleStatus(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

void OalSoundSys::Set3DSampleMsPosition(
	LHSAMPLE sample_handle,
	const sint32 milliseconds)
{
	static_cast<void>(sample_handle);
	static_cast<void>(milliseconds);
}

sint32 OalSoundSys::Set3DSampleInfo(
	LH3DSAMPLE sample_handle,
	const LTSOUNDINFO& sound_info)
{
	static_cast<void>(sample_handle);
	static_cast<void>(sound_info);

	return {};
}

void OalSoundSys::Set3DSampleDistances(
	LH3DSAMPLE sample_handle,
	const float max_distance,
	const float min_distance)
{
	static_cast<void>(sample_handle);
	static_cast<void>(max_distance);
	static_cast<void>(min_distance);
}

void OalSoundSys::Set3DSamplePreference(
	LH3DSAMPLE sample_handle,
	const char* name,
	const void* value)
{
	static_cast<void>(sample_handle);
	static_cast<void>(name);
	static_cast<void>(value);
}

void OalSoundSys::Set3DSampleLoopBlock(
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

void OalSoundSys::Set3DSampleLoop(
	LH3DSAMPLE sample_handle,
	const bool is_loop)
{
	static_cast<void>(sample_handle);
	static_cast<void>(is_loop);
}

void OalSoundSys::Set3DSampleObstruction(
	LH3DSAMPLE sample_handle,
	const float obstruction)
{
	static_cast<void>(sample_handle);
	static_cast<void>(obstruction);
}

float OalSoundSys::Get3DSampleObstruction(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

void OalSoundSys::Set3DSampleOcclusion(
	LH3DSAMPLE sample_handle,
	const float occlusion)
{
	static_cast<void>(sample_handle);
	static_cast<void>(occlusion);
}

float OalSoundSys::Get3DSampleOcclusion(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

LHSAMPLE OalSoundSys::AllocateSampleHandle(
	LHDIGDRIVER driver_ptr)
{
	static_cast<void>(driver_ptr);

	return {};
}

void OalSoundSys::ReleaseSampleHandle(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::InitSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::StopSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::StartSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::ResumeSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::EndSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::SetSampleVolume(
	LHSAMPLE sample_handle,
	const sint32 volume)
{
	static_cast<void>(sample_handle);
	static_cast<void>(volume);
}

void OalSoundSys::SetSamplePan(
	LHSAMPLE sample_handle,
	const sint32 pan)
{
	static_cast<void>(sample_handle);
	static_cast<void>(pan);
}

sint32 OalSoundSys::GetSampleVolume(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

sint32 OalSoundSys::GetSamplePan(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

void OalSoundSys::SetSampleUserData(
	LHSAMPLE sample_handle,
	const uint32 index,
	const sint32 value)
{
	static_cast<void>(sample_handle);
	static_cast<void>(index);
	static_cast<void>(value);
}

void OalSoundSys::GetDirectSoundInfo(
	LHSAMPLE sample_handle,
	PTDIRECTSOUND& direct_sound,
	PTDIRECTSOUNDBUFFER& direct_sound_buffer)
{
	static_cast<void>(sample_handle);
	static_cast<void>(direct_sound);
	static_cast<void>(direct_sound_buffer);
}

void OalSoundSys::SetSampleReverb(
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

sint32 OalSoundSys::InitSampleFromAddress(
	LHSAMPLE sample_handle,
	const void* ptr,
	const uint32 length,
	const ul::WaveFormatEx& wave_format,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	static_cast<void>(sample_handle);
	static_cast<void>(ptr);
	static_cast<void>(length);
	static_cast<void>(wave_format);
	static_cast<void>(playback_rate);
	static_cast<void>(filter_data_ptr);

	return {};
}

sint32 OalSoundSys::InitSampleFromFile(
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

void OalSoundSys::SetSampleLoopBlock(
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

void OalSoundSys::SetSampleLoop(
	LHSAMPLE sample_handle,
	const bool is_loop)
{
	static_cast<void>(sample_handle);
	static_cast<void>(is_loop);
}

void OalSoundSys::SetSampleMsPosition(
	LHSAMPLE sample_handle,
	const sint32 milliseconds)
{
	static_cast<void>(sample_handle);
	static_cast<void>(milliseconds);
}

sint32 OalSoundSys::GetSampleUserData(
	LHSAMPLE sample_handle,
	const uint32 index)
{
	static_cast<void>(sample_handle);
	static_cast<void>(index);

	return {};
}

uint32 OalSoundSys::GetSampleStatus(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

LHSTREAM OalSoundSys::OpenStream(
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

void OalSoundSys::SetStreamLoop(
	LHSTREAM stream_ptr,
	const bool is_loop)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(is_loop);
}

void OalSoundSys::SetStreamPlaybackRate(
	LHSTREAM stream_ptr,
	const sint32 rate)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(rate);
}

void OalSoundSys::SetStreamMsPosition(
	LHSTREAM stream_ptr,
	const sint32 milliseconds)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(milliseconds);
}

void OalSoundSys::SetStreamUserData(
	LHSTREAM stream_ptr,
	const uint32 index,
	const sint32 value)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(index);
	static_cast<void>(value);
}

sint32 OalSoundSys::GetStreamUserData(
	LHSTREAM stream_ptr,
	const uint32 index)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(index);

	return {};
}

void OalSoundSys::CloseStream(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);
}

void OalSoundSys::StartStream(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);
}

void OalSoundSys::PauseStream(
	LHSTREAM stream_ptr,
	const sint32 is_pause)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(is_pause);
}

void OalSoundSys::ResetStream(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);
}

void OalSoundSys::SetStreamVolume(
	LHSTREAM stream_ptr,
	const sint32 volume)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(volume);
}

void OalSoundSys::SetStreamPan(
	LHSTREAM stream_ptr,
	const sint32 pan)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(pan);
}

sint32 OalSoundSys::GetStreamVolume(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);

	return {};
}

sint32 OalSoundSys::GetStreamPan(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);

	return {};
}

uint32 OalSoundSys::GetStreamStatus(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);

	return {};
}

sint32 OalSoundSys::GetStreamBufferParam(
	LHSTREAM stream_ptr,
	const uint32 param)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(param);

	return {};
}

void OalSoundSys::ClearStreamBuffer(
	LHSTREAM stream_ptr,
	const bool is_clear_data_queue)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(is_clear_data_queue);
}

sint32 OalSoundSys::DecompressADPCM(
	const LTSOUNDINFO& sound_info,
	void*& dst_data,
	uint32& dst_size)
{
	static_cast<void>(sound_info);
	static_cast<void>(dst_data);
	static_cast<void>(dst_size);

	return {};
}

sint32 OalSoundSys::DecompressASI(
	const void* srd_data_ptr,
	const uint32 src_size,
	const char* file_name_ext,
	void*& dst_wav,
	uint32& dst_wav_size,
	LTLENGTHYCB callback)
{
	static_cast<void>(srd_data_ptr);
	static_cast<void>(src_size);
	static_cast<void>(file_name_ext);
	static_cast<void>(dst_wav);
	static_cast<void>(dst_wav_size);
	static_cast<void>(callback);

	return {};
}

uint32 OalSoundSys::GetThreadedSoundTicks()
{
	return {};
}

bool OalSoundSys::HasOnBoardMemory()
{
	return {};
}

OalSoundSys& OalSoundSys::get_singleton()
{
	static auto singleton = OalSoundSys{};
	return singleton;
}

extern "C"
{
	__declspec(dllexport) char* SoundSysDesc();
	__declspec(dllexport) ILTSoundSys* SoundSysMake();
}

char* SoundSysDesc()
{
	static char* description = const_cast<char*>("OpenAL");
	return description;
}

ILTSoundSys* SoundSysMake()
{
	return &OalSoundSys::get_singleton();
}
