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
	.version	= "1.6.4",
	.date		= __DATE__ __TIME__,
	.author		= "Luna the Reborn",
	.url		= "N/A",
	.logtag		= "AirSup",
	.loadable	= PT_STARTUP,
	.unloadable	= PT_NEVER,
};

export inline constexpr auto PLID = &gPluginInfo;

export void LOG_ERROR(const char *pszMessage, std::source_location const &SrcLoc = std::source_location::current()) noexcept // #REPORT_TO_MSVC_inline cuase error here.
{
	gpMetaUtilFuncs->pfnLogError(PLID, "[%s => %s]: %s", SrcLoc.file_name(), SrcLoc.function_name(), pszMessage);
	g_engfuncs.pfnServerPrint(std::format("\nError:\n\t[{} => {}()]: {}\n", SrcLoc.file_name(), SrcLoc.function_name(), pszMessage).c_str());
}
