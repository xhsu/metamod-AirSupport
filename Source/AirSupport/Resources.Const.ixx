module;

#define EXPORT export

export module Resources:Const;

export import <cstdint>;

export import <string_view>;
export import <unordered_map>;

#include "../../Enforcer/Resource_CRC64.hpp"
#include "../../Enforcer/Resource_ModelDetails.hpp"
#include "../../Enforcer/Resource_SoundDetails.hpp"
#include "../../Enforcer/Resource_SpriteDetails.hpp"
