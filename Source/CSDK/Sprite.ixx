module;

#include <cstdio>
#include <cstddef>

export module Sprite;

import std;
import hlsdk;

import FileSystem;

using std::unique_ptr;
using std::vector;

using std::int32_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

struct CaseIgnoredStrLess final
{
	static constexpr char to_lower(char c) noexcept
	{
		if ((c & 0b1000'0000) == 0b1000'0000)	// utf-8 character, keep it as it is.
			return c;

		if (c >= 'A' && c <= 'Z')
			return static_cast<char>(c ^ 0b0010'0000);	// Flip the 6th bit

		return c;
	}

	static constexpr bool operator() (std::string_view lhs, std::string_view rhs) noexcept
	{
		return std::ranges::lexicographical_compare(
			lhs,
			rhs,
			{},
			&to_lower,
			&to_lower
		);
	}

	using is_transparent = int;
};

namespace GoldSrc
{
	export struct Sprite_t
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

		Sprite_t() noexcept = default;
		explicit Sprite_t(FILE *f) noexcept { ReadFromFile(f, this); }

		static inline void ReadFromFile(FILE* f, Sprite_t* pSprite) noexcept
		{
			fseek(f, 0, SEEK_SET);

			fread(pSprite, offsetof(Sprite_t, m_rgColorPalette), 1, f);

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

	struct SpriteInfoManager
	{
		std::map<std::string_view, Sprite_t, CaseIgnoredStrLess> m_Info{};

		template <size_t N>
		bool Add(const char(&szPath)[N]) noexcept
		{
			if (auto f = FileSystem::FOpen(szPath, "rb"); f != nullptr)
			{
				auto const [it, bNew] = m_Info.try_emplace(
					szPath,
					f
				);
				fclose(f);

				return bNew;
			}

			return false;
		}

		[[nodiscard]]
		__forceinline auto operator[](std::string_view szPath) const noexcept -> Sprite_t const*
		{
			if (auto const it = m_Info.find(szPath); it != m_Info.cend())
				return std::addressof(it->second);

			return nullptr;
		}
	};

	export inline SpriteInfoManager SpriteInfo{};
};
