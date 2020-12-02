#include "ltjs_oal_lt_sound_sys.h"

#include "spdlog/sinks/basic_file_sink.h"
#include "bibendovsky_spul_scope_guard.h"


namespace ul = bibendovsky::spul;


namespace ltjs
{


OalLtSoundSys::OalLtSoundSys()
{
	spdlog::flush_every(std::chrono::seconds(1));

	logger_ = spdlog::basic_logger_mt("snddrv_oal", "ltjs_snddrv_log.txt", true);
	logger_->info("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
}

OalLtSoundSys::~OalLtSoundSys()
{
	logger_->info(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	spdlog::shutdown();
}


// -------------------------------------------------------------------------
// ILTSoundSys

bool OalLtSoundSys::Init()
{
	AudioDecoder::initialize_current_thread();

	return true;
}

void OalLtSoundSys::Term()
{
}

void* OalLtSoundSys::GetDDInterface(
	const uint dd_interface_id)
{
	static_cast<void>(dd_interface_id);

	return nullptr;
}

void OalLtSoundSys::Lock()
{
}

void OalLtSoundSys::Unlock()
{
}

sint32 OalLtSoundSys::Startup()
{
	clock_base_ = Clock::now();

	return LS_OK;
}

void OalLtSoundSys::Shutdown()
{
}

std::uint32_t OalLtSoundSys::MsCount()
{
	constexpr auto max_uint32_t = std::numeric_limits<std::uint32_t>::max();

	const auto time_diff = Clock::now() - clock_base_;
	const auto time_diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_diff).count();

	return static_cast<std::uint32_t>(time_diff_ms % max_uint32_t);
}

sint32 OalLtSoundSys::SetPreference(
	const uint32 index,
	const sint32 value)
{
	static_cast<void>(index);
	static_cast<void>(value);

	return LS_ERROR;
}

sint32 OalLtSoundSys::GetPreference(
	const uint32 index)
{
	static_cast<void>(index);

	return 0;
}

void OalLtSoundSys::MemFreeLock(
	void* storage_ptr)
{
	AudioUtils::deallocate(storage_ptr);
}

void* OalLtSoundSys::MemAllocLock(
	const uint32 storage_size)
{
	return AudioUtils::allocate(storage_size);
}

const char* OalLtSoundSys::LastError()
{
	return error_message_.c_str();
}

sint32 OalLtSoundSys::WaveOutOpen(
	LHDIGDRIVER& driver,
	PHWAVEOUT& wave_out,
	const sint32 device_id,
	const ul::WaveFormatEx& wave_format)
{
	logger_->info("Open WaveOut.");

	static_cast<void>(device_id);
	static_cast<void>(wave_format);

	driver = nullptr;
	wave_out = nullptr;

	wave_out_close_internal();

	if (!wave_out_open_internal())
	{
		return LS_ERROR;
	}

	driver = system_.get();

	return LS_OK;
}

void OalLtSoundSys::WaveOutClose(
	LHDIGDRIVER driver_ptr)
{
	logger_->info("Close WaveOut.");

	static_cast<void>(driver_ptr);

	wave_out_close_internal();
}

void OalLtSoundSys::SetDigitalMasterVolume(
	LHDIGDRIVER driver_ptr,
	const sint32 master_volume)
{
	if (driver_ptr == nullptr || driver_ptr != system_.get())
	{
		return;
	}

	listener_3d_uptr_->set_master_3d_listener_volume(master_volume);
}

sint32 OalLtSoundSys::GetDigitalMasterVolume(
	LHDIGDRIVER driver_ptr)
{
	if (driver_ptr == nullptr || driver_ptr != system_.get())
	{
		return 0;
	}

	return listener_3d_uptr_->get_master_3d_listener_volume();
}

sint32 OalLtSoundSys::DigitalHandleRelease(
	LHDIGDRIVER driver_ptr)
{
	static_cast<void>(driver_ptr);

	return 0;
}

sint32 OalLtSoundSys::DigitalHandleReacquire(
	LHDIGDRIVER driver_ptr)
{
	static_cast<void>(driver_ptr);

	return 0;
}

#ifdef USE_EAX20_HARDWARE_FILTERS
bool OalLtSoundSys::SetEAX20Filter(
	const bool is_enable,
	const LTSOUNDFILTERDATA& filter_data)
{
	if (lt_filter_ == nullptr)
	{
		return false;
	}

	try
	{
		lt_filter_->set_listener(
			(is_enable ? oal::LtFilterState::enable : oal::LtFilterState::disable),
			filter_data
		);

		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool OalLtSoundSys::SupportsEAX20Filter()
{
	return lt_filter_ != nullptr;
}

bool OalLtSoundSys::SetEAX20BufferSettings(
	LHSAMPLE sample_handle,
	const LTSOUNDFILTERDATA& filter_data)
{
	if (sample_handle == nullptr)
	{
		return false;
	}

	if (lt_filter_ == nullptr)
	{
		return false;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);
	const auto al_source = source.get_al_source();

	try
	{
		lt_filter_->set_source(al_source, filter_data, source.get_lt_filter_direct_mb());
	}
	catch (...)
	{
	}

	return true;
}
#endif // USE_EAX20_HARDWARE_FILTERS

void OalLtSoundSys::Set3DProviderMinBuffers(
	const uint32 buffer_count)
{
	static_cast<void>(buffer_count);
}

sint32 OalLtSoundSys::Open3DProvider(
	LHPROVIDER provider_id)
{
	switch (provider_id)
	{
	case SOUND3DPROVIDERID_DS3D_SOFTWARE:
	case SOUND3DPROVIDERID_DS3D_HARDWARE:
	case SOUND3DPROVIDERID_DS3D_DEFAULT:
		return true;

	default:
		return false;
	}
}

void OalLtSoundSys::Close3DProvider(
	LHPROVIDER provider_id)
{
	static_cast<void>(provider_id);
}

void OalLtSoundSys::Set3DProviderPreference(
	LHPROVIDER provider_id,
	const char* name,
	const void* value)
{
	static_cast<void>(provider_id);
	static_cast<void>(name);
	static_cast<void>(value);
}

void OalLtSoundSys::Get3DProviderAttribute(
	LHPROVIDER provider_id,
	const char* name,
	void* value)
{
	if (value == nullptr)
	{
		return;
	}

	auto& int_value = *static_cast<sint32*>(value);

	int_value = -1;

	if (name == nullptr)
	{
		return;
	}

	const auto name_string = std::string{name};

	if (name_string != "Max samples")
	{
		return;
	}

	int_value = 255;
}

sint32 OalLtSoundSys::Enumerate3DProviders(
	LHPROENUM& index,
	LHPROVIDER& id,
	const char*& name)
{
	const auto current_index = index++;

	id = 0;
	name = nullptr;

	if (current_index < 0 || current_index > 0)
	{
		return false;
	}

	id = SOUND3DPROVIDERID_DS3D_HARDWARE;
	name = "OpenAL";

	return true;
}

LH3DPOBJECT OalLtSoundSys::Open3DListener(
	LHPROVIDER provider_id)
{
	static_cast<void>(provider_id);

	return listener_3d_uptr_.get();
}

void OalLtSoundSys::Close3DListener(
	LH3DPOBJECT listener_ptr)
{
	static_cast<void>(listener_ptr);
}

void OalLtSoundSys::SetListenerDoppler(
	LH3DPOBJECT listener_ptr,
	const float doppler_factor)
{
	if (listener_ptr != listener_3d_uptr_.get())
	{
		return;
	}

	auto& source = *listener_3d_uptr_.get();

	source.set_3d_doppler_factor(doppler_factor);
}

void OalLtSoundSys::CommitDeferred()
{
}

void OalLtSoundSys::Set3DPosition(
	LH3DPOBJECT object_ptr,
	const float x,
	const float y,
	const float z)
{
	if (object_ptr == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(object_ptr);

	source.set_3d_position({x, y, z});
}

void OalLtSoundSys::Set3DVelocityVector(
	LH3DPOBJECT object_ptr,
	const float dx_per_s,
	const float dy_per_s,
	const float dz_per_s)
{
	if (object_ptr == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(object_ptr);

	source.set_3d_velocity({dx_per_s, dy_per_s, dz_per_s});
}

void OalLtSoundSys::Set3DOrientation(
	LH3DPOBJECT object_ptr,
	const float x_face,
	const float y_face,
	const float z_face,
	const float x_up,
	const float y_up,
	const float z_up)
{
	if (object_ptr == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(object_ptr);

	source.set_3d_orientation({x_face, y_face, z_face, x_up, y_up, z_up});
}

void OalLtSoundSys::Set3DUserData(
	LH3DPOBJECT object_ptr,
	const uint32 index,
	const std::intptr_t value)
{
	if (object_ptr == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(object_ptr);

	return source.set_user_value(index, value);
}

void OalLtSoundSys::Get3DPosition(
	LH3DPOBJECT object_ptr,
	float& x,
	float& y,
	float& z)
{
	if (object_ptr == nullptr)
	{
		x = 0.0F;
		y = 0.0F;
		z = 0.0F;

		return;
	}

	const auto& source = *static_cast<const OalLtSoundSysStreamingSource*>(object_ptr);

	const auto& position = source.get_3d_position();

	x = position[0];
	y = position[1];
	z = position[2];
}

void OalLtSoundSys::Get3DVelocity(
	LH3DPOBJECT object_ptr,
	float& dx_per_ms,
	float& dy_per_ms,
	float& dz_per_ms)
{
	if (object_ptr == nullptr)
	{
		dx_per_ms = 0.0F;
		dy_per_ms = 0.0F;
		dz_per_ms = 0.0F;

		return;
	}

	const auto& source = *static_cast<const OalLtSoundSysStreamingSource*>(object_ptr);

	const auto& velocity = source.get_3d_velocity();

	dx_per_ms = velocity[0];
	dy_per_ms = velocity[1];
	dz_per_ms = velocity[2];
}

void OalLtSoundSys::Get3DOrientation(
	LH3DPOBJECT object_ptr,
	float& x_face,
	float& y_face,
	float& z_face,
	float& x_up,
	float& y_up,
	float& z_up)
{
	if (object_ptr == nullptr)
	{
		x_face = 0.0F;
		y_face = 0.0F;
		z_face = 0.0F;

		x_up = 0.0F;
		y_up = 0.0F;
		z_up = 0.0F;

		return;
	}

	const auto& source = *static_cast<const OalLtSoundSysStreamingSource*>(object_ptr);

	const auto& orientation = source.get_3d_orientation();

	x_face = orientation[0];
	y_face = orientation[1];
	z_face = orientation[2];

	x_up = orientation[3];
	y_up = orientation[4];
	z_up = orientation[5];
}

std::intptr_t OalLtSoundSys::Get3DUserData(
	LH3DPOBJECT object_ptr,
	const uint32 index)
{
	if (object_ptr == nullptr)
	{
		return 0;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(object_ptr);

	return source.get_user_value(index);
}

LH3DSAMPLE OalLtSoundSys::Allocate3DSampleHandle(
	LHPROVIDER provider_id)
{
	static_cast<void>(provider_id);

	objects_3d_.emplace_back(
		OalLtSoundSysStreamingSourceType::spatial,
		OalLtSoundSysStreamingSourceSpatialType::source);

	auto& source = objects_3d_.back();

	if (source.is_failed())
	{
		objects_3d_.pop_back();
		return nullptr;
	}

	{
		MtMutexGuard lock{mt_3d_objects_mutex_};
		mt_open_3d_objects_.emplace_back(&source);
	}

	return &source;
}

void OalLtSoundSys::Release3DSampleHandle(
	LH3DSAMPLE sample_handle)
{
	if (sample_handle == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	{
		MtMutexGuard lock{mt_3d_objects_mutex_};

		mt_open_3d_objects_.remove_if(
			[&](const auto& item)
			{
				return &source == item;
			}
		);
	}

	objects_3d_.remove_if(
		[&](const auto& item)
		{
			return &source == &item;
		}
	);
}

void OalLtSoundSys::Stop3DSample(
	LH3DSAMPLE sample_handle)
{
	if (sample_handle == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	{
		MtMutexGuard lock{mt_3d_objects_mutex_};

		if (source.is_failed())
		{
			return;
		}

		source.pause();
	}

	mt_notify_sound();
}

void OalLtSoundSys::Start3DSample(
	LH3DSAMPLE sample_handle)
{
	if (sample_handle == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	{
		MtMutexGuard lock{mt_3d_objects_mutex_};

		if (source.is_failed())
		{
			return;
		}

		source.stop();
		source.resume();
	}

	mt_notify_sound();
}

void OalLtSoundSys::Resume3DSample(
	LH3DSAMPLE sample_handle)
{
	if (sample_handle == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	{
		MtMutexGuard lock{mt_3d_objects_mutex_};

		if (source.is_failed())
		{
			return;
		}

		source.resume();
	}

	mt_notify_sound();
}

void OalLtSoundSys::End3DSample(
	LH3DSAMPLE sample_handle)
{
	if (sample_handle == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	{
		MtMutexGuard lock{mt_3d_objects_mutex_};

		if (source.is_failed())
		{
			return;
		}

		source.stop();
	}

	mt_notify_sound();
}

sint32 OalLtSoundSys::Init3DSampleFromAddress(
	LH3DSAMPLE sample_handle,
	const void* storage_ptr,
	const uint32 storage_size,
	const ul::WaveFormatEx& wave_format,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	static_cast<void>(filter_data_ptr);

	if (sample_handle == nullptr)
	{
		return false;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	auto open_param = OalLtSoundSysStreamingSourceOpenParam{};

	open_param.is_memory_ = true;
	open_param.memory_ptr_ = storage_ptr;
	open_param.memory_size_ = storage_size;
	open_param.memory_wave_format_ = wave_format;
	open_param.playback_rate_ = playback_rate;

	{
		MtMutexGuard lock{mt_samples_mutex_};

		if (source.is_failed())
		{
			return false;
		}

		if (!source.open(open_param))
		{
			return false;
		}

		initialize_lt_filter_for_source(source, filter_data_ptr);
	}

	mt_notify_sound();

	return true;
}

sint32 OalLtSoundSys::Init3DSampleFromFile(
	LH3DSAMPLE sample_handle,
	const void* storage_ptr,
	const sint32 block,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	static_cast<void>(block);
	static_cast<void>(filter_data_ptr);

	if (sample_handle == nullptr)
	{
		return false;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	auto open_param = OalLtSoundSysStreamingSourceOpenParam{};

	open_param.is_mapped_file_ = true;
	open_param.mapped_decoder_ = &audio_decoder_;
	open_param.mapped_storage_ptr = storage_ptr;
	open_param.playback_rate_ = playback_rate;

	{
		MtMutexGuard lock{mt_samples_mutex_};

		if (source.is_failed())
		{
			return false;
		}

		if (!source.open(open_param))
		{
			return false;
		}

		initialize_lt_filter_for_source(source, filter_data_ptr);
	}

	mt_notify_sound();

	return true;
}

sint32 OalLtSoundSys::Get3DSampleVolume(
	LH3DSAMPLE sample_handle)
{
	if (sample_handle == nullptr)
	{
		return 0;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	return source.get_3d_volume();
}

void OalLtSoundSys::Set3DSampleVolume(
	LH3DSAMPLE sample_handle,
	const sint32 volume)
{
	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	MtMutexGuard lock{mt_3d_objects_mutex_};

	source.set_3d_volume(volume);
}

uint32 OalLtSoundSys::Get3DSampleStatus(
	LH3DSAMPLE sample_handle)
{
	if (sample_handle == nullptr)
	{
		return LS_STOPPED;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	MtMutexGuard lock{mt_3d_objects_mutex_};

	return source.is_playing() ? LS_PLAYING : LS_STOPPED;
}

void OalLtSoundSys::Set3DSampleMsPosition(
	LHSAMPLE sample_handle,
	const sint32 milliseconds)
{
	if (sample_handle == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	MtMutexGuard lock{mt_3d_objects_mutex_};

	source.set_ms_position(milliseconds);
}

sint32 OalLtSoundSys::Set3DSampleInfo(
	LH3DSAMPLE sample_handle,
	const LTSOUNDINFO& sound_info)
{
	static_cast<void>(sample_handle);
	static_cast<void>(sound_info);

	return 0;
}

void OalLtSoundSys::Set3DSampleDistances(
	LH3DSAMPLE sample_handle,
	const float max_distance,
	const float min_distance)
{
	if (sample_handle == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	source.set_3d_distances(min_distance, max_distance);
}

void OalLtSoundSys::Set3DSamplePreference(
	LH3DSAMPLE sample_handle,
	const char* name,
	const void* value)
{
	static_cast<void>(sample_handle);
	static_cast<void>(name);
	static_cast<void>(value);
}

void OalLtSoundSys::Set3DSampleLoopBlock(
	LH3DSAMPLE sample_handle,
	const sint32 loop_begin_offset,
	const sint32 loop_end_offset,
	const bool is_enable)
{
	if (sample_handle == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	MtMutexGuard lock{mt_3d_objects_mutex_};

	source.set_loop_block(loop_begin_offset, loop_end_offset, is_enable);
}

void OalLtSoundSys::Set3DSampleLoop(
	LH3DSAMPLE sample_handle,
	const bool is_enable)
{
	if (sample_handle == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	MtMutexGuard lock{mt_3d_objects_mutex_};

	source.set_loop(is_enable);
}

void OalLtSoundSys::Set3DSampleObstruction(
	LH3DSAMPLE sample_handle,
	const float obstruction)
{
	static_cast<void>(sample_handle);
	static_cast<void>(obstruction);
}

float OalLtSoundSys::Get3DSampleObstruction(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return 0.0F;
}

void OalLtSoundSys::Set3DSampleOcclusion(
	LH3DSAMPLE sample_handle,
	const float occlusion)
{
	static_cast<void>(sample_handle);
	static_cast<void>(occlusion);
}

float OalLtSoundSys::Get3DSampleOcclusion(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return 0.0F;
}

LHSAMPLE OalLtSoundSys::AllocateSampleHandle(
	LHDIGDRIVER driver_ptr)
{
	if (driver_ptr != system_.get())
	{
		return nullptr;
	}

	samples_.emplace_back(
		OalLtSoundSysStreamingSourceType::panning,
		OalLtSoundSysStreamingSourceSpatialType::none);

	auto& source = samples_.back();

	if (source.is_failed())
	{
		samples_.pop_back();
		return nullptr;
	}

	{
		MtMutexGuard lock{mt_samples_mutex_};
		mt_open_samples_.emplace_back(&source);
	}

	return &source;
}

void OalLtSoundSys::ReleaseSampleHandle(
	LHSAMPLE sample_ptr)
{
	if (sample_ptr == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_ptr);

	{
		MtMutexGuard lock{mt_samples_mutex_};

		mt_open_samples_.remove_if(
			[&](const auto& item)
			{
				return &source == item;
			}
		);
	}

	samples_.remove_if(
		[&](const auto& item)
		{
			return &source == &item;
		}
	);
}

void OalLtSoundSys::InitSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalLtSoundSys::StopSample(
	LHSAMPLE sample_ptr)
{
	if (sample_ptr == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_ptr);

	{
		MtMutexGuard lock{mt_samples_mutex_};

		if (source.is_failed())
		{
			return;
		}

		source.pause();
	}

	mt_notify_sound();
}

void OalLtSoundSys::StartSample(
	LHSAMPLE sample_ptr)
{
	if (sample_ptr == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_ptr);

	{
		MtMutexGuard lock{mt_samples_mutex_};

		if (source.is_failed())
		{
			return;
		}

		source.stop();
		source.resume();
	}

	mt_notify_sound();
}

void OalLtSoundSys::ResumeSample(
	LHSAMPLE sample_ptr)
{
	if (sample_ptr == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_ptr);

	{
		MtMutexGuard lock{mt_samples_mutex_};

		if (source.is_failed())
		{
			return;
		}

		source.resume();
	}

	mt_notify_sound();
}

void OalLtSoundSys::EndSample(
	LHSAMPLE sample_ptr)
{
	if (sample_ptr == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_ptr);

	{
		MtMutexGuard lock{mt_samples_mutex_};

		if (source.is_failed())
		{
			return;
		}

		source.stop();
	}

	mt_notify_sound();
}

void OalLtSoundSys::SetSampleVolume(
	LHSAMPLE sample_ptr,
	const sint32 volume)
{
	if (sample_ptr == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_ptr);

	MtMutexGuard lock{mt_samples_mutex_};

	source.set_volume(volume);
}

void OalLtSoundSys::SetSamplePan(
	LHSAMPLE sample_ptr,
	const sint32 pan)
{
	if (sample_ptr == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_ptr);

	MtMutexGuard lock{mt_samples_mutex_};

	source.set_pan(pan);
}

sint32 OalLtSoundSys::GetSampleVolume(
	LHSAMPLE sample_ptr)
{
	if (sample_ptr == nullptr)
	{
		return 0;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_ptr);

	return source.get_volume();
}

sint32 OalLtSoundSys::GetSamplePan(
	LHSAMPLE sample_ptr)
{
	if (sample_ptr == nullptr)
	{
		return 0;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_ptr);

	return source.get_pan();
}

void OalLtSoundSys::SetSampleUserData(
	LHSAMPLE sample_handle,
	const uint32 index,
	const std::intptr_t value)
{
	if (sample_handle == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	source.set_user_value(index, value);
}

void OalLtSoundSys::GetDirectSoundInfo(
	LHSAMPLE sample_handle,
	PTDIRECTSOUND& ds_instance,
	PTDIRECTSOUNDBUFFER& ds_buffer)
{
	static_cast<void>(sample_handle);

	ds_instance = nullptr;
	ds_buffer = nullptr;
}

void OalLtSoundSys::SetSampleReverb(
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

sint32 OalLtSoundSys::InitSampleFromAddress(
	LHSAMPLE sample_handle,
	const void* storage_ptr,
	const uint32 storage_size,
	const ul::WaveFormatEx& wave_format,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	static_cast<void>(filter_data_ptr);

	if (sample_handle == nullptr)
	{
		return false;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	auto open_param = OalLtSoundSysStreamingSourceOpenParam{};

	open_param.is_memory_ = true;
	open_param.memory_ptr_ = storage_ptr;
	open_param.memory_size_ = storage_size;
	open_param.memory_wave_format_ = wave_format;
	open_param.playback_rate_ = playback_rate;

	{
		MtMutexGuard lock{mt_samples_mutex_};

		if (source.is_failed())
		{
			return false;
		}

		if (!source.open(open_param))
		{
			return false;
		}

		initialize_lt_filter_for_source(source, filter_data_ptr);
	}

	mt_notify_sound();

	return true;
}

sint32 OalLtSoundSys::InitSampleFromFile(
	LHSAMPLE sample_handle,
	const void* storage_ptr,
	const sint32 block,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	static_cast<void>(block);
	static_cast<void>(filter_data_ptr);

	if (sample_handle == nullptr)
	{
		return false;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	auto open_param = OalLtSoundSysStreamingSourceOpenParam{};

	open_param.is_mapped_file_ = true;
	open_param.mapped_decoder_ = &audio_decoder_;
	open_param.mapped_storage_ptr = storage_ptr;
	open_param.playback_rate_ = playback_rate;

	{
		MtMutexGuard lock{mt_samples_mutex_};

		if (source.is_failed())
		{
			return false;
		}

		if (!source.open(open_param))
		{
			return false;
		}

		initialize_lt_filter_for_source(source, filter_data_ptr);
	}

	mt_notify_sound();

	return true;
}

void OalLtSoundSys::SetSampleLoopBlock(
	LHSAMPLE sample_handle,
	const sint32 loop_begin_offset,
	const sint32 loop_end_offset,
	const bool is_enable)
{
	if (sample_handle == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	MtMutexGuard lock{mt_samples_mutex_};

	source.set_loop_block(loop_begin_offset, loop_end_offset, is_enable);
}

void OalLtSoundSys::SetSampleLoop(
	LHSAMPLE sample_handle,
	const bool is_enable)
{
	if (sample_handle == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	MtMutexGuard lock{mt_samples_mutex_};

	source.set_loop(is_enable);
}

void OalLtSoundSys::SetSampleMsPosition(
	LHSAMPLE sample_handle,
	const sint32 milliseconds)
{
	if (sample_handle == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	MtMutexGuard lock{mt_samples_mutex_};

	source.set_ms_position(milliseconds);
}

std::intptr_t OalLtSoundSys::GetSampleUserData(
	LHSAMPLE sample_handle,
	const uint32 index)
{
	if (sample_handle == nullptr)
	{
		return 0;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	return source.get_user_value(index);
}

uint32 OalLtSoundSys::GetSampleStatus(
	LHSAMPLE sample_handle)
{
	if (sample_handle == nullptr)
	{
		return 0;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(sample_handle);

	MtMutexGuard lock{mt_samples_mutex_};

	return source.is_playing() ? LS_PLAYING : LS_STOPPED;
}

LHSTREAM OalLtSoundSys::OpenStream(
	const char* file_name,
	const uint32 file_offset,
	LHDIGDRIVER driver_ptr,
	const char* storage_ptr,
	const sint32 storage_size)
{
	static_cast<void>(storage_ptr);
	static_cast<void>(storage_size);

	if (file_name == nullptr || driver_ptr != system_.get())
	{
		return nullptr;
	}

	streams_.emplace_back(
		OalLtSoundSysStreamingSourceType::panning,
		OalLtSoundSysStreamingSourceSpatialType::none);

	auto& source = streams_.back();

	if (source.is_failed())
	{
		streams_.pop_back();
		return nullptr;
	}

	auto open_param = OalLtSoundSysStreamingSourceOpenParam{};

	open_param.is_file_ = true;
	open_param.file_name_ = file_name;
	open_param.file_offset_ = file_offset;

	if (!source.open(open_param))
	{
		streams_.pop_back();
		return nullptr;
	}

	initialize_lt_filter_for_source(source, nullptr);

	{
		MtMutexGuard lock{mt_streams_mutex_};
		mt_open_streams_.emplace_back(&source);
	}

	mt_notify_sound();

	return &source;
}

void OalLtSoundSys::SetStreamLoop(
	LHSTREAM stream_ptr,
	const bool is_enable)
{
	if (stream_ptr == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(stream_ptr);

	MtMutexGuard mt_stream_lock{mt_streams_mutex_};

	source.set_loop(is_enable);
}

void OalLtSoundSys::SetStreamPlaybackRate(
	LHSTREAM stream_ptr,
	const sint32 rate)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(rate);
}

void OalLtSoundSys::SetStreamMsPosition(
	LHSTREAM stream_ptr,
	const sint32 milliseconds)
{
	if (stream_ptr == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(stream_ptr);

	MtMutexGuard mt_stream_lock{mt_streams_mutex_};

	source.set_ms_position(milliseconds);
}

void OalLtSoundSys::SetStreamUserData(
	LHSTREAM stream_ptr,
	const uint32 index,
	const std::intptr_t value)
{
	if (stream_ptr == nullptr || index > oal_lt_sound_sys_max_user_data_index)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(stream_ptr);

	source.set_user_value(index, value);
}

std::intptr_t OalLtSoundSys::GetStreamUserData(
	LHSTREAM stream_ptr,
	const uint32 index)
{
	if (stream_ptr == nullptr || index > oal_lt_sound_sys_max_user_data_index)
	{
		return 0;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(stream_ptr);

	return source.get_user_value(index);
}

void OalLtSoundSys::CloseStream(
	LHSTREAM stream_ptr)
{
	if (stream_ptr == nullptr)
	{
		return;
	}

	auto& stream = *static_cast<OalLtSoundSysStreamingSource*>(stream_ptr);

	{
		MtMutexGuard mt_stream_lock{mt_streams_mutex_};

		mt_open_streams_.remove_if(
			[&](const auto& item)
			{
				return &stream == item;
			}
		);
	}

	streams_.remove_if(
		[&](const auto& item)
		{
			return &stream == &item;
		}
	);
}

void OalLtSoundSys::StartStream(
	LHSTREAM stream_ptr)
{
	if (stream_ptr == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(stream_ptr);

	{
		MtMutexGuard mt_stream_lock{mt_streams_mutex_};

		if (source.is_failed())
		{
			return;
		}

		source.stop();
		source.resume();
	}

	mt_notify_sound();
}

void OalLtSoundSys::PauseStream(
	LHSTREAM stream_ptr,
	const sint32 is_enable)
{
	if (stream_ptr == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(stream_ptr);

	{
		MtMutexGuard mt_stream_lock{mt_streams_mutex_};

		if (source.is_failed())
		{
			return;
		}

		if (is_enable)
		{
			source.pause();
		}
		else
		{
			source.resume();
		}
	}

	mt_notify_sound();
}

void OalLtSoundSys::ResetStream(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);
}

void OalLtSoundSys::SetStreamVolume(
	LHSTREAM stream_ptr,
	const sint32 volume)
{
	if (stream_ptr == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(stream_ptr);

	MtMutexGuard mt_stream_lock{mt_streams_mutex_};

	source.set_volume(volume);
}

void OalLtSoundSys::SetStreamPan(
	LHSTREAM stream_ptr,
	const sint32 pan)
{
	if (stream_ptr == nullptr)
	{
		return;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(stream_ptr);

	MtMutexGuard mt_stream_lock{mt_streams_mutex_};

	source.set_pan(pan);
}

sint32 OalLtSoundSys::GetStreamVolume(
	LHSTREAM stream_ptr)
{
	if (stream_ptr == nullptr)
	{
		return 0;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(stream_ptr);

	return source.get_volume();
}

sint32 OalLtSoundSys::GetStreamPan(
	LHSTREAM stream_ptr)
{
	if (stream_ptr == nullptr)
	{
		return 0;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(stream_ptr);

	return source.get_pan();
}

uint32 OalLtSoundSys::GetStreamStatus(
	LHSTREAM stream_ptr)
{
	if (stream_ptr == nullptr)
	{
		return LS_ERROR;
	}

	auto& source = *static_cast<OalLtSoundSysStreamingSource*>(stream_ptr);

	MtMutexGuard mt_stream_lock{mt_streams_mutex_};

	return source.is_playing() ? LS_PLAYING : LS_STOPPED;
}

sint32 OalLtSoundSys::GetStreamBufferParam(
	LHSTREAM stream_ptr,
	const uint32 index)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(index);

	return 0;
}

void OalLtSoundSys::ClearStreamBuffer(
	LHSTREAM stream_ptr,
	const bool is_clear_data_queue)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(is_clear_data_queue);
}

sint32 OalLtSoundSys::DecompressADPCM(
	const LTSOUNDINFO& sound_info,
	void*& dst_data,
	uint32& dst_size)
{
	static_cast<void>(sound_info);
	static_cast<void>(dst_data);
	static_cast<void>(dst_size);

	return 0;
}

sint32 OalLtSoundSys::DecompressASI(
	const void* src_data_ptr,
	const uint32 src_data_size,
	const char* file_name_ext,
	void*& ds_wav_ptr,
	uint32& ds_wav_size,
	LTLENGTHYCB callback)
{
	static_cast<void>(file_name_ext);
	static_cast<void>(callback);

	return AudioUtils::decode_mp3(audio_decoder_, src_data_ptr, src_data_size, ds_wav_ptr, ds_wav_size);
}

uint32 OalLtSoundSys::GetThreadedSoundTicks()
{
	return 0;
}

bool OalLtSoundSys::HasOnBoardMemory()
{
	return false;
}

void OalLtSoundSys::handle_focus_lost(
	const bool is_focus_lost)
{
	listener_3d_uptr_->mute_3d_listener(is_focus_lost);
}

ILTSoundSys::GenericStreamHandle OalLtSoundSys::open_generic_stream(
	const int sample_rate,
	const int buffer_size)
{
	auto generic_stream = OalLtSoundSysGenericStream{};

	if (!generic_stream.open(sample_rate, buffer_size))
	{
		return nullptr;
	}

	MtMutexGuard mutex_guard{mt_generic_streams_mutex_};

	generic_streams_.emplace_back(std::move(generic_stream));

	return &generic_streams_.back();
}

void OalLtSoundSys::close_generic_stream(
	GenericStreamHandle stream_handle)
{
	if (stream_handle == nullptr)
	{
		return;
	}

	MtMutexGuard mutex_guard{mt_generic_streams_mutex_};

	generic_streams_.remove_if(
		[&](const auto& item)
		{
			return &item == stream_handle;
		}
	);
}

int OalLtSoundSys::get_generic_stream_queue_size()
{
	return OalLtSoundSysGenericStream::queue_size;
}

int OalLtSoundSys::get_generic_stream_free_buffer_count(
	GenericStreamHandle stream_handle)
{
	if (stream_handle == nullptr)
	{
		return false;
	}

	MtMutexGuard mutex_guard{mt_generic_streams_mutex_};

	auto& stream = *static_cast<OalLtSoundSysGenericStream*>(stream_handle);

	return stream.get_free_buffer_count();
}

bool OalLtSoundSys::enqueue_generic_stream_buffer(
	GenericStreamHandle stream_handle,
	const void* buffer)
{
	if (stream_handle == nullptr)
	{
		return false;
	}

	MtMutexGuard mutex_guard{mt_generic_streams_mutex_};

	auto& stream = *static_cast<OalLtSoundSysGenericStream*>(stream_handle);

	return stream.enqueue_data(buffer);
}

bool OalLtSoundSys::set_generic_stream_pause(
	GenericStreamHandle stream_handle,
	const bool is_pause)
{
	if (stream_handle == nullptr)
	{
		return false;
	}

	MtMutexGuard mutex_guard{mt_generic_streams_mutex_};

	auto& stream = *static_cast<OalLtSoundSysGenericStream*>(stream_handle);

	return stream.set_pause(is_pause);
}

bool OalLtSoundSys::get_generic_stream_pause(
	GenericStreamHandle stream_handle)
{
	if (stream_handle == nullptr)
	{
		return false;
	}

	MtMutexGuard mutex_guard{mt_generic_streams_mutex_};

	auto& stream = *static_cast<OalLtSoundSysGenericStream*>(stream_handle);

	return stream.get_pause();
}

bool OalLtSoundSys::set_generic_stream_volume(
	GenericStreamHandle stream_handle,
	const int ds_volume)
{
	if (stream_handle == nullptr)
	{
		return false;
	}

	MtMutexGuard mutex_guard{mt_generic_streams_mutex_};

	auto& stream = *static_cast<OalLtSoundSysGenericStream*>(stream_handle);

	return stream.set_volume(ds_volume);
}

int OalLtSoundSys::get_generic_stream_volume(
	GenericStreamHandle stream_handle)
{
	if (stream_handle == nullptr)
	{
		return false;
	}

	MtMutexGuard mutex_guard{mt_generic_streams_mutex_};

	auto& stream = *static_cast<OalLtSoundSysGenericStream*>(stream_handle);

	return stream.get_volume();
}

// ILTSoundSys
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
// Utils

void OalLtSoundSys::uninitialize_streaming()
{
	mt_is_stop_sound_worker_ = true;

	mt_notify_sound();

	if (mt_sound_thread_.joinable())
	{
		mt_sound_thread_.join();
	}

	mt_is_stop_sound_worker_ = false;

	samples_.clear();
	mt_open_samples_.clear();

	objects_3d_.clear();
	mt_open_3d_objects_.clear();

	streams_.clear();
	mt_open_streams_.clear();
}

void OalLtSoundSys::initialize_streaming()
{
	uninitialize_streaming();

	mt_sound_thread_ = MtThread{std::bind(&OalLtSoundSys::sound_worker, this)};
}

void OalLtSoundSys::mt_notify_sound()
{
	MtUniqueLock cv_lock{mt_sound_cv_mutex_};
	mt_sound_cv_flag_ = true;
	mt_sound_cv_.notify_one();
}

void OalLtSoundSys::mt_wait_for_sound_cv()
{
	MtUniqueLock cv_lock{mt_sound_cv_mutex_};
	mt_sound_cv_.wait(cv_lock, [&](){ return mt_sound_cv_flag_; });
	mt_sound_cv_flag_ = false;
}

void OalLtSoundSys::sound_worker()
{
	const auto sleep_delay = std::chrono::milliseconds{10};

	while (!mt_is_stop_sound_worker_)
	{
		auto are_samples_idle = false;

		{
			MtUniqueLock lock{mt_samples_mutex_};

			if (mt_open_samples_.empty())
			{
				are_samples_idle = true;
			}
			else
			{
				const auto total_count = static_cast<int>(mt_open_samples_.size());

				auto paused_count = 0;

				for (auto stream_ptr : mt_open_samples_)
				{
					auto& stream = *stream_ptr;

					stream.mix();

					if (!stream.is_playing())
					{
						paused_count += 1;
					}
				}

				if (paused_count == total_count)
				{
					are_samples_idle = true;
				}
			}
		}


		auto are_3d_objects_idle = false;

		{
			MtUniqueLock lock{mt_3d_objects_mutex_};

			if (mt_open_3d_objects_.empty())
			{
				are_3d_objects_idle = true;
			}
			else
			{
				const auto total_count = static_cast<int>(mt_open_3d_objects_.size());

				auto paused_count = 0;

				for (auto stream_ptr : mt_open_3d_objects_)
				{
					auto& stream = *stream_ptr;

					stream.mix();

					if (!stream.is_playing())
					{
						paused_count += 1;
					}
				}

				if (paused_count == total_count)
				{
					are_3d_objects_idle = true;
				}
			}
		}


		auto are_streams_idle = false;

		{
			MtUniqueLock lock{mt_streams_mutex_};

			if (mt_open_streams_.empty())
			{
				are_streams_idle = true;
			}
			else
			{
				const auto total_count = static_cast<int>(mt_open_streams_.size());

				auto paused_count = 0;

				for (auto stream_ptr : mt_open_streams_)
				{
					auto& stream = *stream_ptr;

					stream.mix();

					if (!stream.is_playing())
					{
						paused_count += 1;
					}
				}

				if (paused_count == total_count)
				{
					are_streams_idle = true;
				}
			}
		}

		if (are_samples_idle && are_3d_objects_idle && are_streams_idle)
		{
			mt_wait_for_sound_cv();
		}
		else
		{
			std::this_thread::sleep_for(sleep_delay);
		}
	}
}

void OalLtSoundSys::initialize_lt_filter_for_source(
	OalLtSoundSysStreamingSource& source,
	const LTSOUNDFILTERDATA* lt_filter_data)
{
	if (lt_filter_ == nullptr)
	{
		return;
	}

	const auto al_source = source.get_al_source();

	try
	{
		auto& direct_mb = source.get_lt_filter_direct_mb();

		lt_filter_->initialize_source(al_source, direct_mb);

		if (lt_filter_data != nullptr)
		{
			lt_filter_->set_source(al_source, *lt_filter_data, direct_mb);
		}
	}
	catch (...)
	{
	}
}

// Utils
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
// API utils

bool OalLtSoundSys::wave_out_open_internal()
{
	auto is_succeed = false;

	auto guard_this = ul::ScopeGuard{
		[&]()
		{
			if (!is_succeed)
			{
				wave_out_close_internal();
			}
		}
	};

	try
	{
		auto oal_system_param = oal::SystemParam{};
		oal_system_param.device_name = nullptr;

		logger_->info("Initialize system.");

		system_ = oal::make_system(oal_system_param);


		const auto& system_info = system_->get_info();

		logger_->info("Device name: {}", system_info.device_name);
		logger_->info("Vendor: {}", system_info.vendor);
		logger_->info("Renderer: {}", system_info.renderer);
		logger_->info("Version: {}", system_info.version);
	}
	catch (const std::exception& ex)
	{
		error_message_ = ex.what();
		logger_->error(error_message_);
		return false;
	}

	try
	{
		logger_->info("Create suitable LT filter.");
		lt_filter_ = oal::make_lt_filter();
		const auto& lt_filter_info = lt_filter_->get_info();
		logger_->info("LT filter: {}", lt_filter_info.name);

		for (const auto& feature_name : lt_filter_info.feature_names)
		{
			logger_->info("    {}", feature_name);
		}
	}
	catch (const std::exception& ex)
	{
		logger_->warn(ex.what());
	}

	is_succeed = true;

	listener_3d_uptr_ = std::make_unique<OalLtSoundSysStreamingSource>(
		OalLtSoundSysStreamingSourceType::spatial,
		OalLtSoundSysStreamingSourceSpatialType::listener);

	initialize_streaming();

	return true;
}

void OalLtSoundSys::wave_out_close_internal()
{
	logger_->info("Close WaveOut.");

	uninitialize_streaming();

	listener_3d_uptr_ = nullptr;

	lt_filter_ = nullptr;
	system_ = nullptr;
}

// API utils
// -------------------------------------------------------------------------


} // ltjs
