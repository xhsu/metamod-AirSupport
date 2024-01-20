export module Debug;

export import const_;

import Platform;

export struct sizebuf_t
{
	const char* buffername{};
	uint32_t flags{};
	uint8_t* data{};
	uint32_t maxsize{};
	uint32_t cursize{};
};

export inline constexpr auto FSB_ALLOWOVERFLOW = (1 << 0);	// if false, do a Sys_Error
export inline constexpr auto FSB_OVERFLOWED = (1 << 1);	// set tp true if the buffer size failed

void* __cdecl SZ_GetSpace(sizebuf_t* buf, uint32_t length) noexcept
{
	if (buf->cursize + length > buf->maxsize)
	{
		if ((buf->flags & FSB_ALLOWOVERFLOW) == 0)
		{
			if (!buf->maxsize)
				UTIL_Terminate("SZ_GetSpace:  Tried to write to an uninitialized sizebuf_t: %s", buf->buffername ? buf->buffername : "???");
			else
				UTIL_Terminate("SZ_GetSpace: overflow without FSB_ALLOWOVERFLOW set on %s", buf->buffername ? buf->buffername : "???");
		}

		if (length > buf->maxsize)
		{
			if ((buf->flags & FSB_ALLOWOVERFLOW) == 0)
				UTIL_Terminate("SZ_GetSpace: %i is > full buffer size on %s", length, buf->buffername ? buf->buffername : "???");
			//else
			//	Con_DPrintf("SZ_GetSpace: %i is > full buffer size on %s, ignoring", length);
		}

		//Con_Printf("SZ_GetSpace: overflow on %s\n", buf->buffername ? buf->buffername : "???");
		buf->flags |= FSB_OVERFLOWED;
		buf->cursize = 0;
	}

	auto ret = buf->data + buf->cursize;
	buf->cursize += length;
	return ret;
}

void* __cdecl OrpheuF_SZ_GetSpace(sizebuf_t* buf, uint32_t length) noexcept
{

}
