module;

#define WIN32_LEAN_AND_MEAN
#define NOWINRES
#define NOSERVICE
#define NOMCX
#define NOIME
#define NOMINMAX	// LUNA: first thing to do when using windows header. It makes 'Windows SDK for C++' compatiable with C++. Fuck Microsoft.
#include <Windows.h>

export module FileSystem;

export import std;

export import Platform;

using std::FILE;
using std::fopen;

inline constexpr char CREATEINTERFACE_PROCNAME[] = "CreateInterface";

// All interfaces derive from this.
class IBaseInterface
{
public:
	virtual ~IBaseInterface() {}
};

typedef IBaseInterface* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);

export inline constexpr char STDIO_FILESYSTEM_LIB[] = "filesystem_stdio.dll";
export inline constexpr char STEAM_FILESYSTEM_LIB[] = "filesystem_steam.dll";

export using FileFindHandle_t = int;
export using WaitForResourcesHandle_t = int;
export using WarningFunc_t = void (*)(const char* fmt, ...);

export enum FileSystemSeek_t
{
	FILESYSTEM_SEEK_HEAD = 0,
	FILESYSTEM_SEEK_CURRENT,
	FILESYSTEM_SEEK_TAIL,
};

export enum
{
	FILESYSTEM_INVALID_FIND_HANDLE = -1
};

export enum FileWarningLevel_t
{
	FILESYSTEM_WARNING_QUIET = 0,				// Don't print anything
	FILESYSTEM_WARNING_REPORTUNCLOSED,			// On shutdown, report names of files left unclosed
	FILESYSTEM_WARNING_REPORTUSAGE,				// Report number of times a file was opened, closed
	FILESYSTEM_WARNING_REPORTALLACCESSES		// Report all open/close events to console (!slow!)
};

export class IFileSystem : public IBaseInterface
{
public:
	// Mount and unmount the filesystem
	virtual void			Mount() = 0;
	virtual void			Unmount() = 0;

	// Remove all search paths (including write path?)
	virtual void			RemoveAllSearchPaths() = 0;

	// Add paths in priority order (mod dir, game dir, ....)
	// If one or more .pak files are in the specified directory, then they are
	//  added after the file system path
	// If the path is the relative path to a .bsp file, then any previous .bsp file
	//  override is cleared and the current .bsp is searched for an embedded PAK file
	//  and this file becomes the highest priority search path (i.e., it's looked at first
	//   even before the mod's file system path).
	virtual void			AddSearchPath(const char* pPath, const char* pathID) = 0;
	virtual bool			RemoveSearchPath(const char* pPath) = 0;

	// Deletes a file
	virtual void			RemoveFile(const char* pRelativePath, const char* pathID) = 0;

	// this isn't implementable on STEAM as is.
	virtual void			CreateDirHierarchy(const char* path, const char* pathID) = 0;

	// File I/O and info
	virtual bool			FileExists(const char* pFileName) = 0;
	virtual bool			IsDirectory(const char* pFileName) = 0;

	// opens a file
	// if pathID is NULL, all paths will be searched for the file
	virtual FILE *			Open(const char* pFileName, const char* pOptions, const char* pathID = 0L) = 0;

	virtual void			Close(FILE* file) = 0;

	virtual void			Seek(FILE* file, int pos, FileSystemSeek_t seekType) = 0;
	virtual unsigned int	Tell(FILE* file) = 0;

	virtual unsigned int	Size(FILE* file) = 0;
	virtual unsigned int	Size(const char* pFileName) = 0;

	virtual long			GetFileTime(const char* pFileName) = 0;
	virtual void			FileTimeToString(char* pStrip, int maxCharsIncludingTerminator, long fileTime) = 0;

	virtual bool			IsOk(FILE* file) = 0;

	virtual void			Flush(FILE* file) = 0;
	virtual bool			EndOfFile(FILE* file) = 0;

	virtual int				Read(void* pOutput, int size, FILE* file) = 0;
	virtual int				Write(void const* pInput, int size, FILE* file) = 0;
	virtual char*			ReadLine(char* pOutput, int maxChars, FILE* file) = 0;
	virtual int				FPrintf(FILE* file, char* pFormat, ...) = 0;

	// direct filesystem buffer access
	// returns a handle to a buffer containing the file data
	// this is the optimal way to access the complete data for a file,
	// since the file preloader has probably already got it in memory
	virtual void*			GetReadBuffer(FILE* file, int* outBufferSize, bool failIfNotInCache) = 0;
	virtual void			ReleaseReadBuffer(FILE* file, void* readBuffer) = 0;

	// FindFirst/FindNext
	virtual const char*		FindFirst(const char* pWildCard, FileFindHandle_t* pHandle, const char* pathID = 0L) = 0;
	virtual const char*		FindNext(FileFindHandle_t handle) = 0;
	virtual bool			FindIsDirectory(FileFindHandle_t handle) = 0;
	virtual void			FindClose(FileFindHandle_t handle) = 0;

	virtual void			GetLocalCopy(const char* pFileName) = 0;

	virtual const char*		GetLocalPath(const char* pFileName, char* pLocalPath, int localPathBufferSize) = 0;

	// Note: This is sort of a secondary feature; but it's really useful to have it here
	virtual char*			ParseFile(char* pFileBytes, char* pToken, bool* pWasQuoted) = 0;

	// Returns true on success (based on current list of search paths, otherwise false if it can't be resolved)
	virtual bool			FullPathToRelativePath(const char* pFullpath, char* pRelative) = 0;

	// Gets the current working directory
	virtual bool			GetCurrentDirectory(char* pDirectory, int maxlen) = 0;

	// Dump to printf/OutputDebugString the list of files that have not been closed
	virtual void			PrintOpenedFiles() = 0;

	virtual void			SetWarningFunc(WarningFunc_t pfnWarning) = 0;
	virtual void			SetWarningLevel(FileWarningLevel_t level) = 0;

	virtual void			LogLevelLoadStarted(const char* name) = 0;
	virtual void			LogLevelLoadFinished(const char* name) = 0;
	virtual int				HintResourceNeed(const char* hintlist, int forgetEverything) = 0;
	virtual int				PauseResourcePreloading() = 0;
	virtual int				ResumeResourcePreloading() = 0;
	virtual int				SetVBuf(FILE* stream, char* buffer, int mode, long size) = 0;
	virtual void			GetInterfaceVersion(char* p, int maxlen) = 0;
	virtual bool			IsFileImmediatelyAvailable(const char* pFileName) = 0;

	// starts waiting for resources to be available
	// returns FILESYSTEM_INVALID_HANDLE if there is nothing to wait on
	virtual WaitForResourcesHandle_t WaitForResources(const char* resourcelist) = 0;

	// get progress on waiting for resources; progress is a float [0, 1], complete is true on the waiting being done
	// returns false if no progress is available
	// any calls after complete is true or on an invalid handle will return false, 0.0f, true
	virtual bool			GetWaitForResourcesProgress(WaitForResourcesHandle_t handle, float* progress /* out */, bool* complete /* out */) = 0;

	// cancels a progress call
	virtual void			CancelWaitForResources(WaitForResourcesHandle_t handle) = 0;
	// returns true if the appID has all its caches fully preloaded
	virtual bool			IsAppReadyForOfflinePlay(int appID) = 0;

	// interface for custom pack files > 4Gb
	virtual bool			AddPackFile(const char* fullpath, const char* pathID) = 0;

	// open a file but force the data to come from the steam cache, NOT from disk
	virtual FILE *			OpenFromCacheForRead(const char* pFileName, const char* pOptions, const char* pathID = 0L) = 0;
	virtual void			AddSearchPathNoWrite(const char* pPath, const char* pathID) = 0;
};

namespace FileSystem
{
	export inline constexpr char INTERFACE_VERSION[] = "VFileSystem009";

	export extern "C++" inline IFileSystem* m_pObject = nullptr;
	export extern "C++" inline HMODULE m_pModuleHandle = nullptr;

	// Call once in GameInit_Post()
	export bool Init() noexcept
	{
		m_pModuleHandle = LoadLibraryA(STDIO_FILESYSTEM_LIB);
		if (!m_pModuleHandle) [[unlikely]]
			return false;

		// Get FileSystem interface
		auto const filesystemFactoryFn = (CreateInterfaceFn)GetProcAddress(m_pModuleHandle, CREATEINTERFACE_PROCNAME);
		if (!filesystemFactoryFn) [[unlikely]]
			UTIL_Terminate("Unable to find function '%s' from %s", CREATEINTERFACE_PROCNAME, STDIO_FILESYSTEM_LIB);

		m_pObject = (IFileSystem *)filesystemFactoryFn(INTERFACE_VERSION, nullptr);
		if (!m_pObject) [[unlikely]]
			UTIL_Terminate("Can not retrive filesystem interface version '%s'.", INTERFACE_VERSION);

		return true;
	}

	export std::string_view GetAbsolutePath(const char *pszRelativePath) noexcept
	{
		static char szBuffer[1024]{};
		m_pObject->GetLocalPath(pszRelativePath, szBuffer, sizeof(szBuffer) - 1);

		return szBuffer;
	}

	export std::string RelativeToWorkingDir(std::string_view szRelativePath) noexcept
	{
		static auto const ExecutePath = std::filesystem::current_path();
		auto const AbsolutePath = FileSystem::GetAbsolutePath(szRelativePath.data());

		return std::filesystem::relative(AbsolutePath, ExecutePath).u8string();
	}

	export FILE *FOpen(const char *pszRelativePath, const char *pszMode) noexcept
	{
		auto const pszAbsPath = GetAbsolutePath(pszRelativePath);

		return fopen(pszAbsPath.data(), pszMode);
	}

	export void Shutdown() noexcept
	{
		if (m_pModuleHandle) [[likely]]
		{
			FreeLibrary(m_pModuleHandle);
			m_pModuleHandle = nullptr;
			m_pObject = nullptr;
		}
	}
};
