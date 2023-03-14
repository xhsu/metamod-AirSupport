export module Plugin;

export import <format>;
export import <source_location>;

export import meta_api;
export import mutil;

export // Description of plugin
inline constexpr plugin_info_t gPluginInfo =
{
	.ifvers		= META_INTERFACE_VERSION,
	.name		= "Air Support",
	.version	= "1.7.0",
	.date		= __DATE__ __TIME__,
	.author		= "Luna the Reborn",
	.url		= "N/A",
	.logtag		= "AirSup",
	.loadable	= PT_STARTUP,
	.unloadable	= PT_NEVER,
};

export inline constexpr auto PLID = &gPluginInfo;
