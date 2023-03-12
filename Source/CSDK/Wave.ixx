export module Wave;

export import <cstdint>;
export import <cstring>;

export import <algorithm>;
export import <filesystem>;
export import <fstream>;
export import <string_view>;

using std::string_view;

export struct wav_hdr_t
{
	/* RIFF Chunk Descriptor */
	char RIFF[4]{ 'R', 'I', 'F', 'F' };		// RIFF Header Magic header
	uint32_t m_iRiffChunkSize{};			// RIFF Chunk Size
	char WAVE[4]{ 'W', 'A', 'V', 'E' };		// WAVE Header

	/* "fmt" sub-chunk */
	char FMT[4]{ 'f', 'm', 't', ' ' };		// FMT header
	uint32_t m_iSubchunk1Size = 16;			// Size of the fmt chunk
	uint16_t m_iAudioFormat = 1;			// Audio format 1=PCM, 6=mulaw, 7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
	uint16_t m_iNumOfChan = 1;				// Number of channels 1=Mono 2=Sterio
	uint32_t m_iSamplesPerSec = 16000;		// Sampling Frequency in Hz
	uint32_t m_iBytesPerSec = 16000 * 2;	// bytes per second
	uint16_t m_iBlockAlign = 2;				// 2=16-bit mono, 4=16-bit stereo
	uint16_t m_iBitsPerSample = 16;			// Number of bits per sample
};

export struct riff_chunk_t
{
	char m_desc[4]{};
	uint32_t m_iChunkSize{};
};

export namespace Wave
{
	inline double Length(FILE* f) noexcept
	{
		auto eof = [&](void) noexcept -> bool { auto const c = fgetc(f); ungetc(c, f); return c == EOF; };
		fseek(f, 0, SEEK_SET);

		wav_hdr_t WaveHeader{};
		fread(&WaveHeader, sizeof(WaveHeader), 1, f);

		riff_chunk_t Chunk{};
		string_view szChunkDesc(Chunk.m_desc, 4);
		uint32_t iSampledDataLength{};

		for (; !eof();)
		{
			fread(&Chunk, sizeof(Chunk), 1, f);
			fseek(f, Chunk.m_iChunkSize, SEEK_CUR);

			std::ranges::transform(Chunk.m_desc, Chunk.m_desc, tolower);

			if (szChunkDesc == "data")
				iSampledDataLength = Chunk.m_iChunkSize;
		}

		auto const numSamples = iSampledDataLength / (WaveHeader.m_iNumOfChan * (WaveHeader.m_iBitsPerSample / 8.0));
		auto const durationSeconds = numSamples / WaveHeader.m_iSamplesPerSec;

		return durationSeconds;
	}

	inline double Length(const char* pszPath) noexcept
	{
		if (FILE* f = fopen(pszPath, "rb"); f)
		{
			auto const ret = ::Wave::Length(f);
			fclose(f);

			return ret;
		}

		return {};
	}
};
