#ifdef LTJS_SDL_BACKEND


#include <algorithm>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "SDL.h"

#include "ltjs_shared_library.h"

#include "iltsound.h"


#ifdef USE_ABSTRACT_SOUND_INTERFACES


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class SdlLTSoundFactory final :
	public ILTSoundFactory
{
public:
	bool EnumSoundSystems(
		FnEnumSoundSysCallback fnEnumCallback,
		void* pUserData = nullptr) override;

	bool FillSoundSystems(
		char* pcSoundSysNames,
		uint uiMaxStringLen) override;

	ILTSoundSys* MakeSoundSystem(
		const char* pcSoundSystemName) override;


private:
	struct Node
	{
		std::string description{};
		std::string path{};
		ltjs::SharedLibraryUPtr shared_library{};
		ILTSoundSys* sound_system{};


		void swap(
			Node& rhs) noexcept;

		explicit operator bool() const noexcept;
	}; // Node


	using Nodes = std::list<Node>;

	Nodes nodes_{};


	static Node make_node(
		const char* path);

	const Node* find_node(
		const char* path) const noexcept;

	const Node* add_node(
		const char* path);
}; // SdlLTSoundFactory

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void SdlLTSoundFactory::Node::swap(
	Node& rhs) noexcept
{
	description.swap(rhs.description);
	path.swap(rhs.path);
	shared_library.swap(rhs.shared_library);
	std::swap(sound_system, rhs.sound_system);
}

SdlLTSoundFactory::Node::operator bool() const noexcept
{
	return
		!path.empty() &&
		shared_library &&
		sound_system;
}

bool SdlLTSoundFactory::EnumSoundSystems(
	FnEnumSoundSysCallback fnEnumCallback,
	void* pUserData)
{
	return false;
}

bool SdlLTSoundFactory::FillSoundSystems(
	char* pcSoundSysNames,
	uint uiMaxStringLen)
{
	static_cast<void>(pcSoundSysNames);
	static_cast<void>(uiMaxStringLen);

	return false;
}

ILTSoundSys* SdlLTSoundFactory::MakeSoundSystem(
	const char* pcSoundSystemName)
{
	using Paths = std::vector<std::string>;
	auto paths = Paths{};
	paths.reserve(8);

	if (pcSoundSystemName)
	{
		paths.emplace_back(pcSoundSystemName);
	}
	else
	{
		const auto prefix = std::string{"ltjs_snd_drv_"};
		const auto suffix = ltjs::SharedLibrary::get_default_suffix();

		paths.emplace_back(prefix + "oal" + suffix);
#if _WIN32
		paths.emplace_back(prefix + "dx8" + suffix);
#endif
		paths.emplace_back(prefix + "nul" + suffix);
	}

	for (const auto& path : paths)
	{
		auto node = add_node(path.c_str());

		if (node)
		{
			return node->sound_system;
		}
	}

	return nullptr;
}

SdlLTSoundFactory::Node SdlLTSoundFactory::make_node(
	const char* path)
try
{
	auto shared_library = ltjs::make_shared_library(path);

	using SoundSysDescFunc = const char* (*)();
	using SoundSysMakeFunc = ILTSoundSys* (*)();

	const auto sound_sys_desc = shared_library->find_symbol<SoundSysDescFunc>("SoundSysDesc");
	const auto sound_sys_make = shared_library->find_symbol<SoundSysMakeFunc>("SoundSysMake");

	if (!sound_sys_desc || !sound_sys_make)
	{
		return Node{};
	}

	const auto description_c_str = sound_sys_desc();

	if (!description_c_str)
	{
		return Node{};
	}

	auto sound_system = sound_sys_make();

	if (!sound_system)
	{
		return Node{};
	}

	if (!sound_system->Init())
	{
		return Node{};
	}

	auto node = Node{};
	node.description = description_c_str;
	node.path = path;
	node.shared_library.swap(shared_library);
	std::swap(node.sound_system, sound_system);

	return node;
}
catch (...)
{
	return Node{};
}

const SdlLTSoundFactory::Node* SdlLTSoundFactory::find_node(
	const char* path) const noexcept
{
	if (!path)
	{
		return nullptr;
	}

	const auto node_end_it = nodes_.cend();

	const auto node_it = std::find_if(
		nodes_.cbegin(),
		node_end_it,
		[path](
			const Node& node)
		{
			return path == node.path;
		}
	);

	if (node_it == node_end_it)
	{
		return nullptr;
	}

	return &(*node_it);
}

const SdlLTSoundFactory::Node* SdlLTSoundFactory::add_node(
	const char* path)
{
	const auto found_node = find_node(path);

	if (found_node)
	{
		return found_node;
	}

	auto node = make_node(path);

	if (!node)
	{
		return nullptr;
	}

	nodes_.emplace_back();
	auto& new_node = nodes_.back();
	new_node.swap(node);

	return &new_node;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

ILTSoundFactory* ILTSoundFactory::m_pSoundFactory{};

bool ILTSoundFactory::EnumSoundSystems(
	FnEnumSoundSysCallback fnEnumCallback,
	void* pUserData)
{
	static_cast<void>(fnEnumCallback);
	static_cast<void>(pUserData);

	return false;
}

ILTSoundFactory* ILTSoundFactory::GetSoundFactory()
{
	static auto factory = SdlLTSoundFactory{};

	return &factory;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#endif // USE_ABSTRACT_SOUND_INTERFACES


#ifdef USE_DX8_SOFTWARE_FILTERS

// filter support
void LTFILTERCHORUS::SetParam(const char* pszParam, float fValue)
{
	if (!strcmp(pszParam, "Delay"))
	{
		fDelay = fValue;
		uiFilterParamFlags |= SET_CHORUS_DELAY;
	}
	else if (!strcmp(pszParam, "Depth"))
	{
		fDepth = fValue;
		uiFilterParamFlags |= SET_CHORUS_DEPTH;
	}
	else if (!strcmp(pszParam, "Feedback"))
	{
		fFeedback = fValue;
		uiFilterParamFlags |= SET_CHORUS_FEEDBACK;
	}
	else if (!strcmp(pszParam, "Frequency"))
	{
		fFrequency = fValue;
		uiFilterParamFlags |= SET_CHORUS_FREQUENCY;
	}
	else if (!strcmp(pszParam, "WetDryMix"))
	{
		fWetDryMix = fValue;
		uiFilterParamFlags |= SET_CHORUS_WETDRYMIX;
	}
	else if (!strcmp(pszParam, "Phase"))
	{
		lPhase = (int)fValue;
		uiFilterParamFlags |= SET_CHORUS_PHASE;
	}
	else if (!strcmp(pszParam, "Waveform"))
	{
		lWaveform = (int)fValue;
		uiFilterParamFlags |= SET_CHORUS_WAVEFORM;
	}
}

void LTFILTERCOMPRESSOR::SetParam(const char* pszParam, float fValue)
{
	if (!strcmp(pszParam, "Attack"))
	{
		fAttack = fValue;
		uiFilterParamFlags |= SET_COMPRESSOR_ATTACK;
	}
	else if (!strcmp(pszParam, "Gain"))
	{
		fGain = fValue;
		uiFilterParamFlags |= SET_COMPRESSOR_GAIN;
	}
	else if (!strcmp(pszParam, "Predelay"))
	{
		fPredelay = fValue;
		uiFilterParamFlags |= SET_COMPRESSOR_PREDELAY;
	}
	else if (!strcmp(pszParam, "Ratio"))
	{
		fRatio = fValue;
		uiFilterParamFlags |= SET_COMPRESSOR_RATIO;
	}
	else if (!strcmp(pszParam, "Release"))
	{
		fRelease = fValue;
		uiFilterParamFlags |= SET_COMPRESSOR_RELEASE;
	}
	else if (!strcmp(pszParam, "Threshold"))
	{
		fThreshold = fValue;
		uiFilterParamFlags |= SET_COMPRESSOR_THRESHOLD;
	}
}

void LTFILTERDISTORTION::SetParam(const char* pszParam, float fValue)
{
	if (!strcmp(pszParam, "Edge"))
	{
		fEdge = fValue;
		uiFilterParamFlags |= SET_DISTORTION_EDGE;
	}
	else if (!strcmp(pszParam, "Gain"))
	{
		fGain = fValue;
		uiFilterParamFlags |= SET_DISTORTION_GAIN;
	}
	else if (!strcmp(pszParam, "PostEQBandwidth"))
	{
		fPostEQBandwidth = fValue;
		uiFilterParamFlags |= SET_DISTORTION_POSTEQBANDWIDTH;
	}
	else if (!strcmp(pszParam, "PostEQCenterFreq"))
	{
		fPostEQCenterFrequency = fValue;
		uiFilterParamFlags |= SET_DISTORTION_POSTEQCENTERFREQ;
	}
	else if (!strcmp(pszParam, "PreLowpassCutoff"))
	{
		fPreLowpassCutoff = fValue;
		uiFilterParamFlags |= SET_DISTORTION_PRELOWPASSCUTOFF;
	}
}

void LTFILTERECHO::SetParam(const char* pszParam, float fValue)
{
	if (!strcmp(pszParam, "Feedback"))
	{
		fFeedback = fValue;
		uiFilterParamFlags |= SET_ECHO_FEEDBACK;
	}
	else if (!strcmp(pszParam, "LeftDelay"))
	{
		fLeftDelay = fValue;
		uiFilterParamFlags |= SET_ECHO_LEFTDELAY;
	}
	else if (!strcmp(pszParam, "RightDelay"))
	{
		fRightDelay = fValue;
		uiFilterParamFlags |= SET_ECHO_RIGHTDELAY;
	}
	else if (!strcmp(pszParam, "PanDelay"))
	{
		lPanDelay = (int)fValue;
		uiFilterParamFlags |= SET_ECHO_PANDELAY;
	}
}

void LTFILTERFLANGE::SetParam(const char* pszParam, float fValue)
{
	if (!strcmp(pszParam, "Delay"))
	{
		fDelay = fValue;
		uiFilterParamFlags |= SET_FLANGE_DELAY;
	}
	else if (!strcmp(pszParam, "Depth"))
	{
		fDepth = fValue;
		uiFilterParamFlags |= SET_FLANGE_DEPTH;
	}
	else if (!strcmp(pszParam, "Feedback"))
	{
		fFeedback = fValue;
		uiFilterParamFlags |= SET_FLANGE_FEEDBACK;
	}
	else if (!strcmp(pszParam, "Frequency"))
	{
		fFrequency = fValue;
		uiFilterParamFlags |= SET_FLANGE_FREQUENCY;
	}
	else if (!strcmp(pszParam, "WetDryMix"))
	{
		fWetDryMix = fValue;
		uiFilterParamFlags |= SET_FLANGE_WETDRYMIX;
	}
	else if (!strcmp(pszParam, "Phase"))
	{
		lPhase = (int)fValue;
		uiFilterParamFlags |= SET_FLANGE_PHASE;
	}
	else if (!strcmp(pszParam, "Waveform"))
	{
		lWaveform = (int)fValue;
		uiFilterParamFlags |= SET_FLANGE_WAVEFORM;
	}
}

void LTFILTERPARAMEQ::SetParam(const char* pszParam, float fValue)
{
	if (!strcmp(pszParam, "Bandwidth"))
	{
		fBandwidth = fValue;
		uiFilterParamFlags |= SET_PARAMEQ_BANDWIDTH;
	}
	else if (!strcmp(pszParam, "Center"))
	{
		fCenter = fValue;
		uiFilterParamFlags |= SET_PARAMEQ_CENTER;
	}
	else if (!strcmp(pszParam, "Gain"))
	{
		fGain = fValue;
		uiFilterParamFlags |= SET_PARAMEQ_GAIN;
	}
}

void LTFILTERREVERB::SetParam(const char* pszParam, float fValue)
{
	if (!strcmp(pszParam, "DecayHFRatio"))
	{
		fDecayHFRatio = fValue;
		uiFilterParamFlags |= SET_REVERB_DECAYHFRATIO;
	}
	else if (!strcmp(pszParam, "DecayTime"))
	{
		fDecayTime = fValue;
		uiFilterParamFlags |= SET_REVERB_DECAYTIME;
	}
	else if (!strcmp(pszParam, "Density"))
	{
		fDensity = fValue;
		uiFilterParamFlags |= SET_REVERB_DENSITY;
	}
	else if (!strcmp(pszParam, "Diffusion"))
	{
		fDiffusion = fValue;
		uiFilterParamFlags |= SET_REVERB_DIFFUSION;
	}
	else if (!strcmp(pszParam, "HFReference"))
	{
		fHFReference = fValue;
		uiFilterParamFlags |= SET_REVERB_HFREFERENCE;
	}
	else if (!strcmp(pszParam, "ReflectionsDelay"))
	{
		fReflectionsDelay = fValue;
		uiFilterParamFlags |= SET_REVERB_REFLECTIONSDELAY;
	}
	else if (!strcmp(pszParam, "ReverbDelay"))
	{
		fReverbDelay = fValue;
		uiFilterParamFlags |= SET_REVERB_REVERBDELAY;
	}
	else if (!strcmp(pszParam, "RoomRolloffFactor"))
	{
		fRoomRolloffFactor = fValue;
		uiFilterParamFlags |= SET_REVERB_ROOMROLLOFFFACTOR;
	}
	else if (!strcmp(pszParam, "Reflections"))
	{
		lReflections = (int)fValue;
		uiFilterParamFlags |= SET_REVERB_REFLECTIONS;
	}
	else if (!strcmp(pszParam, "Reverb"))
	{
		lReverb = (int)fValue;
		uiFilterParamFlags |= SET_REVERB_REVERB;
	}
	else if (!strcmp(pszParam, "Room"))
	{
		lRoom = (int)fValue;
		uiFilterParamFlags |= SET_REVERB_ROOM;
	}
	else if (!strcmp(pszParam, "RoomHF"))
	{
		lRoomHF = (int)fValue;
		uiFilterParamFlags |= SET_REVERB_ROOMHF;
	}
}

#endif // USE_DX8_SOFTWARE_FILTERS


#ifdef USE_EAX20_HARDWARE_FILTERS

void LTFILTERREVERB::SetParam(
	const char* pszParam,
	float fValue)
{
	if (strcmp(pszParam, "DecayHFRatio") == 0)
	{
		fDecayHFRatio = fValue;
		uiFilterParamFlags |= SET_REVERB_DECAYHFRATIO;
	}
	else if (strcmp(pszParam, "DecayTime") == 0)
	{
		fDecayTime = fValue;
		uiFilterParamFlags |= SET_REVERB_DECAYTIME;
	}
	else if (strcmp(pszParam, "Size") == 0)
	{
		fSize = fValue;
		uiFilterParamFlags |= SET_REVERB_SIZE;
	}
	else if (strcmp(pszParam, "Diffusion") == 0)
	{
		fDiffusion = fValue;
		uiFilterParamFlags |= SET_REVERB_DIFFUSION;
	}
	else if (strcmp(pszParam, "AirAbsorptionHF") == 0)
	{
		fAirAbsorptionHF = fValue;
		uiFilterParamFlags |= SET_REVERB_AIRABSORPTIONHF;
	}
	else if (strcmp(pszParam, "ReflectionsDelay") == 0)
	{
		fReflectionsDelay = fValue;
		uiFilterParamFlags |= SET_REVERB_REFLECTIONSDELAY;
	}
	else if (strcmp(pszParam, "ReverbDelay") == 0)
	{
		fReverbDelay = fValue;
		uiFilterParamFlags |= SET_REVERB_REVERBDELAY;
	}
	else if (strcmp(pszParam, "RoomRolloffFactor") == 0)
	{
		fRoomRolloffFactor = fValue;
		uiFilterParamFlags |= SET_REVERB_ROOMROLLOFFFACTOR;
	}
	else if (strcmp(pszParam, "Reflections") == 0)
	{
		lReflections = static_cast<int>(fValue);
		uiFilterParamFlags |= SET_REVERB_REFLECTIONS;
	}
	else if (strcmp(pszParam, "Reverb") == 0)
	{
		lReverb = static_cast<int>(fValue);
		uiFilterParamFlags |= SET_REVERB_REVERB;
	}
	else if (strcmp(pszParam, "Room") == 0)
	{
		lRoom = static_cast<int>(fValue);
		uiFilterParamFlags |= SET_REVERB_ROOM;
	}
	else if (strcmp(pszParam, "RoomHF") == 0)
	{
		lRoomHF = static_cast<int>(fValue);
		uiFilterParamFlags |= SET_REVERB_ROOMHF;
	}
	else if (strcmp(pszParam, "Direct") == 0)
	{
		lDirect = static_cast<int>(fValue);
		uiFilterParamFlags |= SET_REVERB_DIRECT;
	}
	else if (strcmp(pszParam, "Environment") == 0)
	{
		lEnvironment = static_cast<int>(fValue);
		uiFilterParamFlags |= SET_REVERB_ENVIRONMENT;
	}
}

#endif // USE_EAX20_HARDWARE_FILTERS


#endif // LTJS_SDL_BACKEND
