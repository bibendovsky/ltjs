#include "ltjs_oal_efx_symbols.h"

#include "alc.h"

#include "ltjs_exception.h"


namespace ltjs
{
namespace oal
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class EfxSymbolsException :
	public SafeException
{
public:
	explicit EfxSymbolsException(
		const char* message)
		:
		SafeException{message}
	{
	}
}; // EfxSymbolsException
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

EfxSymbols make_efx_symbols()
{
	const auto al_context = alcGetCurrentContext();

	if (al_context == nullptr)
	{
		throw EfxSymbolsException{"Null current context."};
	}

	const auto al_device = alcGetContextsDevice(al_context);

	if (al_device == nullptr)
	{
		throw EfxSymbolsException{"Null device."};
	}

	const auto is_efx_present = (alcIsExtensionPresent(al_device, ALC_EXT_EFX_NAME) != ALC_FALSE);

	if (!is_efx_present)
	{
		throw EfxSymbolsException{"EFX extension not present."};
	}


	auto efx_symbols = EfxSymbols{};

	struct EfxSymbolDescriptor
	{
		const char* symbol_name;
		void** symbol_address;
	}; // EfxSymbolDescriptor

	EfxSymbolDescriptor efx_symbol_descriptors[] =
	{
		{"alGenEffects", reinterpret_cast<void**>(&efx_symbols.alGenEffects)},
		{"alDeleteEffects", reinterpret_cast<void**>(&efx_symbols.alDeleteEffects)},
		{"alIsEffect", reinterpret_cast<void**>(&efx_symbols.alIsEffect)},
		{"alEffecti", reinterpret_cast<void**>(&efx_symbols.alEffecti)},
		{"alEffectiv", reinterpret_cast<void**>(&efx_symbols.alEffectiv)},
		{"alEffectf", reinterpret_cast<void**>(&efx_symbols.alEffectf)},
		{"alEffectfv", reinterpret_cast<void**>(&efx_symbols.alEffectfv)},
		{"alGetEffecti", reinterpret_cast<void**>(&efx_symbols.alGetEffecti)},
		{"alGetEffectiv", reinterpret_cast<void**>(&efx_symbols.alGetEffectiv)},
		{"alGetEffectf", reinterpret_cast<void**>(&efx_symbols.alGetEffectf)},
		{"alGetEffectfv", reinterpret_cast<void**>(&efx_symbols.alGetEffectfv)},
		{"alGenFilters", reinterpret_cast<void**>(&efx_symbols.alGenFilters)},
		{"alDeleteFilters", reinterpret_cast<void**>(&efx_symbols.alDeleteFilters)},
		{"alIsFilter", reinterpret_cast<void**>(&efx_symbols.alIsFilter)},
		{"alFilteri", reinterpret_cast<void**>(&efx_symbols.alFilteri)},
		{"alFilteriv", reinterpret_cast<void**>(&efx_symbols.alFilteriv)},
		{"alFilterf", reinterpret_cast<void**>(&efx_symbols.alFilterf)},
		{"alFilterfv", reinterpret_cast<void**>(&efx_symbols.alFilterfv)},
		{"alGetFilteri", reinterpret_cast<void**>(&efx_symbols.alGetFilteri)},
		{"alGetFilteriv", reinterpret_cast<void**>(&efx_symbols.alGetFilteriv)},
		{"alGetFilterf", reinterpret_cast<void**>(&efx_symbols.alGetFilterf)},
		{"alGetFilterfv", reinterpret_cast<void**>(&efx_symbols.alGetFilterfv)},
		{"alGenAuxiliaryEffectSlots", reinterpret_cast<void**>(&efx_symbols.alGenAuxiliaryEffectSlots)},
		{"alDeleteAuxiliaryEffectSlots", reinterpret_cast<void**>(&efx_symbols.alDeleteAuxiliaryEffectSlots)},
		{"alIsAuxiliaryEffectSlot", reinterpret_cast<void**>(&efx_symbols.alIsAuxiliaryEffectSlot)},
		{"alAuxiliaryEffectSloti", reinterpret_cast<void**>(&efx_symbols.alAuxiliaryEffectSloti)},
		{"alAuxiliaryEffectSlotiv", reinterpret_cast<void**>(&efx_symbols.alAuxiliaryEffectSlotiv)},
		{"alAuxiliaryEffectSlotf", reinterpret_cast<void**>(&efx_symbols.alAuxiliaryEffectSlotf)},
		{"alAuxiliaryEffectSlotfv", reinterpret_cast<void**>(&efx_symbols.alAuxiliaryEffectSlotfv)},
		{"alGetAuxiliaryEffectSloti", reinterpret_cast<void**>(&efx_symbols.alGetAuxiliaryEffectSloti)},
		{"alGetAuxiliaryEffectSlotiv", reinterpret_cast<void**>(&efx_symbols.alGetAuxiliaryEffectSlotiv)},
		{"alGetAuxiliaryEffectSlotf", reinterpret_cast<void**>(&efx_symbols.alGetAuxiliaryEffectSlotf)},
		{"alGetAuxiliaryEffectSlotfv", reinterpret_cast<void**>(&efx_symbols.alGetAuxiliaryEffectSlotfv)},
	};

	for (auto& efx_symbol_descriptor : efx_symbol_descriptors)
	{
		const auto symbol_address = alGetProcAddress(efx_symbol_descriptor.symbol_name);

		if (symbol_address == nullptr)
		{
			throw EfxSymbolsException{"Required EFX symbol not found."};
		}

		*efx_symbol_descriptor.symbol_address = symbol_address;
	}

	return efx_symbols;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // oal
} // ltjs
