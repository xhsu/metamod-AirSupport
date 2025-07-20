module;

#include <cstdio>
#include <cstddef>

export module Sprite;

import std;
import hlsdk;

using std::unique_ptr;
using std::vector;

using std::int32_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;


export struct TranscriptedSprite
{
	static inline constexpr uint32_t FORMAT_IDENTIFIER = 0x50534449;
	static inline constexpr int32_t FORMAT_VERSION = 2;

	uint32_t m_iIdentifier{ FORMAT_IDENTIFIER };
	int32_t m_iFormatVersion{ FORMAT_VERSION };

	enum EDrawType : uint32_t
	{
		ParallelUpright = 0,
		FacingUpright = 1,
		Parallel = 2,
		Oriented = 3,
		ParallelOriented = 4
	}
	m_DrawType{ Parallel };

	enum ETextureType : uint32_t
	{
		Normal = 0,
		Additive = 1,
		IndexAlpha = 2,
		AlphaTest = 3
	}
	m_TextureType{ Additive };

	float m_flBoundingRadius{};

	uint32_t m_iMaxFrameWidth{};
	uint32_t m_iMaxFrameHeight{};
	uint32_t m_iNumOfFrames{};
	float m_flBeamLength{};

	enum ESynchronization : uint32_t
	{
		Synchronized = 0,
		Random = 1
	}
	m_Synchronization{};

	// Color Palette
	vector<color24> m_rgColorPalette{};

	// Frames
	struct Frame_t
	{
		int32_t m_iGroup{};
		int32_t m_iOriginX{}, m_iOriginY{};
		uint32_t m_iWidth{}, m_iHeight{};
		unique_ptr<uint8_t[]> m_Data{};
	};

	vector<Frame_t> m_rgFrames{};

	TranscriptedSprite() noexcept = default;
	explicit TranscriptedSprite(FILE* f) noexcept { ReadFromFile(f, this); }

	static inline void ReadFromFile(FILE* f, TranscriptedSprite* pSprite) noexcept
	{
		fseek(f, 0, SEEK_SET);

		fread(pSprite, offsetof(TranscriptedSprite, m_rgColorPalette), 1, f);

		uint16_t iSizeOfPalette{};
		fread(&iSizeOfPalette, sizeof(iSizeOfPalette), 1, f);

		pSprite->m_rgColorPalette.reserve(iSizeOfPalette);
		pSprite->m_rgFrames.reserve(pSprite->m_iNumOfFrames);

		for (decltype(iSizeOfPalette) i = 0; i < iSizeOfPalette; ++i)
		{
			auto&& Color24 = pSprite->m_rgColorPalette.emplace_back();

			fread(&Color24.r, sizeof(std::byte), 1, f);
			fread(&Color24.g, sizeof(std::byte), 1, f);
			fread(&Color24.b, sizeof(std::byte), 1, f);
		}

		for (decltype(pSprite->m_iNumOfFrames) i = 0; i < pSprite->m_iNumOfFrames; ++i)
		{
			auto&& Frame = pSprite->m_rgFrames.emplace_back();

			fread(&Frame.m_iGroup, sizeof(Frame.m_iGroup), 1, f);
			fread(&Frame.m_iOriginX, sizeof(Frame.m_iOriginX), 1, f);
			fread(&Frame.m_iOriginY, sizeof(Frame.m_iOriginY), 1, f);
			fread(&Frame.m_iWidth, sizeof(Frame.m_iWidth), 1, f);
			fread(&Frame.m_iHeight, sizeof(Frame.m_iHeight), 1, f);

			Frame.m_Data = std::make_unique<uint8_t[]>(Frame.m_iWidth * Frame.m_iHeight);

			fread(Frame.m_Data.get(), sizeof(uint8_t), Frame.m_iWidth * Frame.m_iHeight, f);
		}
	}

	__forceinline void ReadFromFile(FILE* f) noexcept { return ReadFromFile(f, this); }
};
