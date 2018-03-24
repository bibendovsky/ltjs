#include <cstdint>
#include "bibendovsky_spul_endian.h"
#include "bibendovsky_spul_riff_four_ccs.h"
#include "bibendovsky_spul_wave_four_ccs.h"
#include "bibendovsky_spul_wave_format.h"
#include "bibendovsky_spul_wave_format_utils.h"


namespace ul = bibendovsky::spul;


bool ParseWaveFile(
	void* pWaveFileBlock,
	void*& rpWaveFormat,
	std::uint32_t& ruiWaveFormatSize,
	void*& rpSampleData,
	std::uint32_t& ruiSampleDataSize)
{
	constexpr auto wavh_four_cc = ul::FourCc{"wavh"};
	constexpr auto guid_four_cc = ul::FourCc{"guid"};

	ruiSampleDataSize = 0;
	ruiWaveFormatSize = 0;
	rpSampleData = nullptr;
	rpWaveFormat = nullptr;

	auto pucFileBlock = static_cast<std::uint8_t*>(pWaveFileBlock);

	auto bFinished = false;

	while (!bFinished)
	{
		const auto uiChunkId = ul::Endian::little(*reinterpret_cast<std::uint32_t*>(pucFileBlock));

		pucFileBlock += sizeof(std::uint32_t);

		switch (uiChunkId)
		{
		case ul::RiffFourCcs::riff:
			pucFileBlock += sizeof(std::uint32_t);
			break;

		case ul::WaveFourCcs::wave:
			// skip this
			break;

		case ul::WaveFourCcs::fact:
		{
			const auto uiFactSize = ul::Endian::little(*reinterpret_cast<std::uint32_t*>(pucFileBlock));
			pucFileBlock += sizeof(std::uint32_t);
			pucFileBlock += uiFactSize;
			break;
		}

		case ul::WaveFourCcs::data:
		{
			const auto uiDataSize = ul::Endian::little(*reinterpret_cast<std::uint32_t*>(pucFileBlock));
			pucFileBlock += sizeof(std::uint32_t);

			rpSampleData = pucFileBlock;
			ruiSampleDataSize = uiDataSize;
			pucFileBlock += uiDataSize;

			bFinished = true;
			break;
		}

		case wavh_four_cc:
		case guid_four_cc:
		{
			// just skip these
			const auto uiSkipSize = ul::Endian::little(*reinterpret_cast<std::uint32_t*>(pucFileBlock));
			pucFileBlock += sizeof(std::uint32_t);
			pucFileBlock += uiSkipSize;
			break;
		}

		case ul::WaveFourCcs::fmt:
		{
			const auto uiFmtSize = ul::Endian::little(*reinterpret_cast<std::uint32_t*>(pucFileBlock));

			if (uiFmtSize < ul::WaveFormat::packed_size)
			{
				return false;
			}

			pucFileBlock += sizeof(std::uint32_t);
			rpWaveFormat = pucFileBlock;
			ruiWaveFormatSize = uiFmtSize;

			if (!ul::Endian::is_little())
			{
				if (uiFmtSize >= ul::WaveFormatEx::packed_size)
				{
					ul::WaveformatUtils::endian(*static_cast<ul::WaveFormatEx*>(rpWaveFormat));
				}
				else if (uiFmtSize >= ul::PcmWaveFormat::packed_size)
				{
					ul::WaveformatUtils::endian(*static_cast<ul::PcmWaveFormat*>(rpWaveFormat));
				}
				else
				{
					ul::WaveformatUtils::endian(*static_cast<ul::WaveFormat*>(rpWaveFormat));
				}
			}

			pucFileBlock += uiFmtSize;
			break;
		}

		default:
			return false;
		}
	}

	return rpWaveFormat && rpSampleData;
}
