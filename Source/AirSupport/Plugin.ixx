export module Plugin;

export import std;

export import hlsdk;
export import metamod_api;

import Application;

export // Description of plugin
inline constexpr plugin_info_t gPluginInfo =
{
	.ifvers		= META_INTERFACE_VERSION,
	.name		= "Air Support",
	.version	= APP_VERSION_STRING.data(),
	.date		= __DATE__ __TIME__,
	.author		= "Luna the Reborn",
	.url		= "N/A",
	.logtag		= "AirSup",
	.loadable	= PT_STARTUP,
	.unloadable	= PT_NEVER,
};

export inline constexpr auto PLID = &gPluginInfo;
