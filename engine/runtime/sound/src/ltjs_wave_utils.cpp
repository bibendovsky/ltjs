#include "ltjs_wave_utils.h"
#include "bibendovsky_spul_endian.h"
#include "bibendovsky_spul_riff_four_ccs.h"
#include "bibendovsky_spul_wave_format.h"
#include "bibendovsky_spul_wave_four_ccs.h"


namespace ltjs
{


namespace ul = bibendovsky::spul;


int WaveUtils::extract_wave_size(
	const void* raw_data)
{
	if (!raw_data)
	{
		return 0;
	}

	auto header = static_cast<const std::uint32_t*>(raw_data);

	const auto riff_id = ul::Endian::little(header[0]);

	if (riff_id != ul::RiffFourCcs::riff)
	{
		return 0;
	}

	const auto riff_size = ul::Endian::little(header[1]);

	constexpr auto min_riff_size =
		4 + // "WAVE"
		4 + 4 + ul::PcmWaveFormat::packed_size + // "fmt " + size + pcm_wave_format
		4 + 4 + 1 + // "data" + size + min_data_size
		0;

	if (riff_size < min_riff_size)
	{
		return 0;
	}

	const auto wave_id = ul::Endian::little(header[2]);

	if (wave_id != ul::WaveFourCcs::wave)
	{
		return 0;
	}

	return riff_size + 8;
}


} // ltjs
