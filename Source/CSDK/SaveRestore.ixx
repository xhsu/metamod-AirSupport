module;

#include <stdlib.h>
#include <string.h>

export module SaveRestore;

import std;
import hlsdk;

import CBase;

#define _FIELD(type,name,fieldtype,count,flags)		{ fieldtype, #name, offsetof(type, name), count, flags }
#define DEFINE_FIELD(type,name,fieldtype)			_FIELD(type, name, fieldtype, 1, 0)
#define DEFINE_ARRAY(type,name,fieldtype,count)		_FIELD(type, name, fieldtype, count, 0)
#define DEFINE_ENTITY_FIELD(name,fieldtype)			_FIELD(entvars_t, name, fieldtype, 1, 0 )
#define DEFINE_ENTITY_GLOBAL_FIELD(name,fieldtype)	_FIELD(entvars_t, name, fieldtype, 1, FTYPEDESC_GLOBAL )
#define DEFINE_GLOBAL_FIELD(type,name,fieldtype)	_FIELD(type, name, fieldtype, 1, FTYPEDESC_GLOBAL )

inline constexpr TYPEDESCRIPTION g_EntvarsDescription[] =
{
	DEFINE_ENTITY_FIELD(classname, FIELD_STRING),
	DEFINE_ENTITY_GLOBAL_FIELD(globalname, FIELD_STRING),
	DEFINE_ENTITY_FIELD(origin, FIELD_POSITION_VECTOR),
	DEFINE_ENTITY_FIELD(oldorigin, FIELD_POSITION_VECTOR),
	DEFINE_ENTITY_FIELD(velocity, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(basevelocity, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(movedir, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(angles, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(avelocity, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(punchangle, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(v_angle, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(fixangle, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(idealpitch, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(pitch_speed, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(ideal_yaw, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(yaw_speed, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(modelindex, FIELD_INTEGER),
	DEFINE_ENTITY_GLOBAL_FIELD(model, FIELD_MODELNAME),
	DEFINE_ENTITY_FIELD(viewmodel, FIELD_MODELNAME),
	DEFINE_ENTITY_FIELD(weaponmodel, FIELD_MODELNAME),
	DEFINE_ENTITY_FIELD(absmin, FIELD_POSITION_VECTOR),
	DEFINE_ENTITY_FIELD(absmax, FIELD_POSITION_VECTOR),
	DEFINE_ENTITY_GLOBAL_FIELD(mins, FIELD_VECTOR),
	DEFINE_ENTITY_GLOBAL_FIELD(maxs, FIELD_VECTOR),
	DEFINE_ENTITY_GLOBAL_FIELD(size, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(ltime, FIELD_TIME),
	DEFINE_ENTITY_FIELD(nextthink, FIELD_TIME),
	DEFINE_ENTITY_FIELD(solid, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(movetype, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(skin, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(body, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(effects, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(gravity, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(friction, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(light_level, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(frame, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(scale, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(sequence, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(animtime, FIELD_TIME),
	DEFINE_ENTITY_FIELD(framerate, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(controller, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(blending, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(rendermode, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(renderamt, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(rendercolor, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(renderfx, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(health, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(frags, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(weapons, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(takedamage, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(deadflag, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(view_ofs, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(button, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(impulse, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(chain, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(dmg_inflictor, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(enemy, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(aiment, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(owner, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(groundentity, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(spawnflags, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(flags, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(colormap, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(team, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(max_health, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(teleport_time, FIELD_TIME),
	DEFINE_ENTITY_FIELD(armortype, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(armorvalue, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(waterlevel, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(watertype, FIELD_INTEGER),
	DEFINE_ENTITY_GLOBAL_FIELD(target, FIELD_STRING),
	DEFINE_ENTITY_GLOBAL_FIELD(targetname, FIELD_STRING),
	DEFINE_ENTITY_FIELD(netname, FIELD_STRING),
	DEFINE_ENTITY_FIELD(message, FIELD_STRING),
	DEFINE_ENTITY_FIELD(dmg_take, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(dmg_save, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(dmg, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(dmgtime, FIELD_TIME),
	DEFINE_ENTITY_FIELD(noise, FIELD_SOUNDNAME),
	DEFINE_ENTITY_FIELD(noise1, FIELD_SOUNDNAME),
	DEFINE_ENTITY_FIELD(noise2, FIELD_SOUNDNAME),
	DEFINE_ENTITY_FIELD(noise3, FIELD_SOUNDNAME),
	DEFINE_ENTITY_FIELD(speed, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(air_finished, FIELD_TIME),
	DEFINE_ENTITY_FIELD(pain_finished, FIELD_TIME),
	DEFINE_ENTITY_FIELD(radsuit_finished, FIELD_TIME),
};

export struct HEADER
{
	unsigned short size{};
	unsigned short token{};
	char* pData{};
};

export class CSaveRestoreBuffer
{
public:
	CSaveRestoreBuffer() noexcept = default;
	CSaveRestoreBuffer(SAVERESTOREDATA* pdata) noexcept : m_pData{ pdata } {};
	CSaveRestoreBuffer(const CSaveRestoreBuffer&) noexcept = delete;
	CSaveRestoreBuffer(CSaveRestoreBuffer&&) noexcept = delete;
	CSaveRestoreBuffer& operator=(const CSaveRestoreBuffer&) noexcept = delete;
	CSaveRestoreBuffer& operator=(CSaveRestoreBuffer&&) noexcept = delete;
	~CSaveRestoreBuffer() noexcept = default;

	int EntityIndex(entvars_t* pevLookup) noexcept
	{
		if (!pevLookup)
			return -1;

		return EntityIndex(pevLookup->pContainingEntity);
	}
	int EntityIndex(edict_t* pentLookup) noexcept
	{
		if (!m_pData || !pentLookup)
			return -1;

		for (int i = 0; i < m_pData->tableCount; i++)
		{
			ENTITYTABLE* pTable = &m_pData->pTable[i];
			if (pTable->pent == pentLookup)
				return i;
		}

		return -1;
	}
	int EntityIndex(std::ptrdiff_t eoLookup) noexcept { return EntityIndex(g_engfuncs.pfnPEntityOfEntOffset(eoLookup)); }
	int EntityIndex(CBaseEntity* pEntity) noexcept
	{
		if (!pEntity)
			return -1;

		return EntityIndex(pEntity->edict());
	}
	int EntityFlags(int entityIndex, int flags) noexcept
	{
		return EntityFlagsSet(entityIndex, flags);
	}
	int EntityFlagsSet(int entityIndex, int flags) noexcept
	{
		if (!m_pData || entityIndex < 0)
			return 0;

		if (!m_pData || entityIndex < 0 || entityIndex > m_pData->tableCount)
			return 0;

		m_pData->pTable[entityIndex].flags |= flags;
		return m_pData->pTable[entityIndex].flags;
	}
	edict_t* EntityFromIndex(int entityIndex) noexcept
	{
		if (!m_pData || entityIndex < 0)
			return nullptr;

		for (int i = 0; i < m_pData->tableCount; i++)
		{
			ENTITYTABLE* pTable = &m_pData->pTable[i];
			if (pTable->id == entityIndex)
				return pTable->pent;
		}

		return nullptr;
	}
	unsigned short TokenHash(const char* pszToken) noexcept
	{
		unsigned short hash = (unsigned short)(HashString(pszToken) % (unsigned)m_pData->tokenCount);
		for (int i = 0; i < m_pData->tokenCount; i++)
		{
			int index = hash + i;
			if (index >= m_pData->tokenCount)
				index -= m_pData->tokenCount;

			if (!m_pData->pTokens[index] || !std::strcmp(pszToken, m_pData->pTokens[index]))
			{
				m_pData->pTokens[index] = (char*)pszToken;
				return index;
			}
		}

		g_engfuncs.pfnAlertMessage(at_error, "CSaveRestoreBuffer :: TokenHash() is COMPLETELY FULL!");
		return 0;
	}

protected:
	inline static constexpr int m_Sizes[]=
	{
		sizeof(float),     // FIELD_FLOAT
		sizeof(int),       // FIELD_STRING
		sizeof(int),       // FIELD_ENTITY
		sizeof(int),       // FIELD_CLASSPTR
		sizeof(int),       // FIELD_EHANDLE
		sizeof(int),       // FIELD_entvars_t
		sizeof(int),       // FIELD_EDICT
		sizeof(float) * 3, // FIELD_VECTOR
		sizeof(float) * 3, // FIELD_POSITION_VECTOR
		sizeof(int *),     // FIELD_POINTER
		sizeof(int),       // FIELD_INTEGER
		sizeof(int *),     // FIELD_FUNCTION
		sizeof(int),       // FIELD_BOOLEAN
		sizeof(short),     // FIELD_SHORT
		sizeof(char),      // FIELD_CHARACTER
		sizeof(float),     // FIELD_TIME
		sizeof(int),       // FIELD_MODELNAME
		sizeof(int),       // FIELD_SOUNDNAME
	};

	SAVERESTOREDATA* m_pData{};
	void BufferRewind(int size) noexcept
	{
		if (!m_pData)
			return;

		if (m_pData->size < size)
			size = m_pData->size;

		m_pData->pCurrentData -= size;
		m_pData->size -= size;
	}
	unsigned int HashString(const char* pszToken) noexcept
	{
		unsigned int hash = 0;
		while (*pszToken)
			hash = _rotr(hash, 4) ^ *pszToken++;

		return hash;
	}
};

inline constexpr auto MAX_ENTITY_ARRAY = 64;

export class CSave : public CSaveRestoreBuffer
{
public:
	CSave(SAVERESTOREDATA* pdata) noexcept : CSaveRestoreBuffer(pdata) {};

	void WriteShort(const char* pname, const short* data, int count) noexcept { BufferField(pname, sizeof(short) * count, (const char*)data); };
	void WriteInt(const char* pname, const int* data, int count) noexcept { BufferField(pname, sizeof(int) * count, (const char*)data); };
	void WriteFloat(const char* pname, const float* data, int count) noexcept { BufferField(pname, sizeof(float) * count, (const char*)data); };
	void WriteTime(const char* pname, const float* data, int count) noexcept
	{
		BufferHeader(pname, sizeof(float) * count);

		for (int i = 0; i < count; i++)
		{
			float tmp = data[0];
			if (m_pData) {
				tmp -= m_pData->time;
			}

			BufferData((const char*)&tmp, sizeof(float));
			data++;
		}
	}
	void WriteData(const char* pname, int size, const char* pdata) noexcept
	{
		if (!m_pData)
			return;

		if (m_pData->size + size > m_pData->bufferSize)
		{
			g_engfuncs.pfnAlertMessage(at_error, "Save/Restore overflow!");
			m_pData->size = m_pData->bufferSize;
			return;
		}

		std::memcpy(m_pData->pCurrentData, pdata, size);
		m_pData->pCurrentData += size;
		m_pData->size += size;
	}
	void WriteString(const char* pname, const char* pdata) noexcept { BufferField(pname, std::strlen(pdata) + 1, pdata); }
	void WriteString(const char* pname, const int* stringId, int count) noexcept
	{
		int i;
		int size = 0;

		for (i = 0; i < count; i++) {
			size += std::strlen(STRING(stringId[i])) + 1;
		}

		BufferHeader(pname, size);
		for (i = 0; i < count; i++)
		{
			const char* pString = STRING(stringId[i]);
			BufferData(pString, std::strlen(pString) + 1);
		}
	}
	void WriteVector(const char* pname, const Vector& value) noexcept { WriteVector(pname, &value.x, 1); }
	void WriteVector(const char* pname, const float* value, int count) noexcept
	{
		BufferHeader(pname, sizeof(float) * 3 * count);
		BufferData((const char*)value, sizeof(float) * 3 * count);
	}
	void WritePositionVector(const char* pname, const Vector& value) noexcept
	{
		if (m_pData && m_pData->fUseLandmark)
		{
			Vector tmp = value - m_pData->vecLandmarkOffset;
			WriteVector(pname, tmp);
		}
		WriteVector(pname, value);
	}
	void WritePositionVector(const char* pname, const float* value, int count) noexcept
	{
		BufferHeader(pname, sizeof(float) * 3 * count);
		for (int i = 0; i < count; i++)
		{
			Vector tmp(value[0], value[1], value[2]);
			if (m_pData && m_pData->fUseLandmark) {
				tmp -= m_pData->vecLandmarkOffset;
			}

			BufferData((const char*)&tmp.x, sizeof(float) * 3);
			value += 3;
		}
	}
	void WriteFunction(const char* pname, void** data, int count) noexcept
	{
		const char* functionName = g_engfuncs.pfnNameForFunction((std::uintptr_t)*data);

		if (functionName)
			BufferField(pname, std::strlen(functionName) + 1, functionName);
		else
			g_engfuncs.pfnAlertMessage(at_error, "Invalid function pointer in entity!");
	}
	int WriteEntVars(const char* pname, entvars_t* pev) noexcept {	return WriteFields(pname, pev, g_EntvarsDescription, std::ssize(g_EntvarsDescription)); }
	int WriteFields(const char* pname, void* pBaseData, TYPEDESCRIPTION const* pFields, int fieldCount) noexcept
	{
		int i{};
		int emptyCount = 0;

		for (i = 0; i < fieldCount; i++)
		{
			void* pOutputData = ((char*)pBaseData + pFields[i].fieldOffset);
			if (DataEmpty((const char*)pOutputData, pFields[i].fieldSize * m_Sizes[pFields[i].fieldType]))
				emptyCount++;
		}

		int entityArray[MAX_ENTITY_ARRAY]{};
		int actualCount = fieldCount - emptyCount;

		WriteInt(pname, &actualCount, 1);
		for (i = 0; i < fieldCount; i++)
		{
			auto pTest = &pFields[i];
			void* pOutputData = ((char*)pBaseData + pTest->fieldOffset);

			if (DataEmpty((const char*)pOutputData, pTest->fieldSize * m_Sizes[pTest->fieldType]))
				continue;

			switch (pTest->fieldType)
			{
			case FIELD_FLOAT:
				WriteFloat(pTest->fieldName, (float*)pOutputData, pTest->fieldSize);
				break;

			case FIELD_TIME:
				WriteTime(pTest->fieldName, (float*)pOutputData, pTest->fieldSize);
				break;

			case FIELD_MODELNAME:
			case FIELD_SOUNDNAME:
			case FIELD_STRING:
				WriteString(pTest->fieldName, (int*)pOutputData, pTest->fieldSize);
				break;

			case FIELD_CLASSPTR:
			case FIELD_EVARS:
			case FIELD_EDICT:
			case FIELD_ENTITY:
			case FIELD_EHANDLE:
			{
				if (pTest->fieldSize > MAX_ENTITY_ARRAY)
					g_engfuncs.pfnAlertMessage(at_error, "Can't save more than %d entities in an array!!!\n", MAX_ENTITY_ARRAY);

				for (int j = 0; j < pTest->fieldSize; j++)
				{
					switch (pTest->fieldType)
					{
					case FIELD_EVARS:
						entityArray[j] = EntityIndex(((entvars_t**)pOutputData)[j]);
						break;
					case FIELD_CLASSPTR:
						entityArray[j] = EntityIndex(((CBaseEntity**)pOutputData)[j]);
						break;
					case FIELD_EDICT:
						entityArray[j] = EntityIndex(((edict_t**)pOutputData)[j]);
						break;
					case FIELD_ENTITY:
						entityArray[j] = EntityIndex(((std::intptr_t*)pOutputData)[j]);
						break;
					case FIELD_EHANDLE:
						entityArray[j] = EntityIndex((CBaseEntity*)(((EHANDLE<CBaseEntity>*)pOutputData)[j]));
						break;
					default:
						break;
					}
				}
				WriteInt(pTest->fieldName, entityArray, pTest->fieldSize);
				break;
			}
			case FIELD_POSITION_VECTOR:
				WritePositionVector(pTest->fieldName, (float*)pOutputData, pTest->fieldSize);
				break;
			case FIELD_VECTOR:
				WriteVector(pTest->fieldName, (float*)pOutputData, pTest->fieldSize);
				break;
			case FIELD_BOOLEAN:
			case FIELD_INTEGER:
				WriteInt(pTest->fieldName, (int*)pOutputData, pTest->fieldSize);
				break;
			case FIELD_SHORT:
				WriteData(pTest->fieldName, 2 * pTest->fieldSize, ((char*)pOutputData));
				break;
			case FIELD_CHARACTER:
				WriteData(pTest->fieldName, pTest->fieldSize, ((char*)pOutputData));
				break;
			case FIELD_POINTER:
				WriteInt(pTest->fieldName, (int*)(char*)pOutputData, pTest->fieldSize);
				break;
			case FIELD_FUNCTION:
				WriteFunction(pTest->fieldName, &pOutputData, pTest->fieldSize);
				break;
			default:
				g_engfuncs.pfnAlertMessage(at_error, "Bad field type\n");
				break;
			}
		}

		return 1;
	}

private:
	int DataEmpty(const char* pdata, int size) noexcept
	{
		for (int i = 0; i < size; i++)
		{
			if (pdata[i])
				return 0;
		}

		return 1;
	}
	void BufferField(const char* pname, int size, const char* pdata) noexcept
	{
		BufferHeader(pname, size);
		BufferData(pdata, size);
	}
	void BufferString(char* pdata, int len) noexcept
	{
		char c = 0;
		BufferData(pdata, len);
		BufferData(&c, 1);
	}
	void BufferData(const char* pdata, int size) noexcept
	{
		if (!m_pData)
			return;

		if (m_pData->size + size > m_pData->bufferSize)
		{
			g_engfuncs.pfnAlertMessage(at_error, "Save/Restore overflow!");
			m_pData->size = m_pData->bufferSize;
			return;
		}

		std::memcpy(m_pData->pCurrentData, pdata, size);
		m_pData->pCurrentData += size;
		m_pData->size += size;
	}
	void BufferHeader(const char* pname, int size) noexcept
	{
		short hashvalue = TokenHash(pname);
		if (size > (1 << (sizeof(short) * 8)))
			g_engfuncs.pfnAlertMessage(at_error, "CSave :: BufferHeader() size parameter exceeds 'short'!");

		BufferData((const char*)&size, sizeof(short));
		BufferData((const char*)&hashvalue, sizeof(short));
	}
};

export class CRestore : public CSaveRestoreBuffer
{
public:
	CRestore(SAVERESTOREDATA* pdata) noexcept : CSaveRestoreBuffer(pdata) {}

	int ReadEntVars(const char* pname, entvars_t* pev) noexcept { return ReadFields(pname, pev, g_EntvarsDescription, std::ssize(g_EntvarsDescription));}
	int ReadFields(const char* pname, void* pBaseData, TYPEDESCRIPTION const* pFields, int fieldCount) noexcept
	{
		unsigned short i = ReadShort();
		unsigned short token = ReadShort();

		if (token != TokenHash(pname))
		{
			BufferRewind(2 * sizeof(short));
			return 0;
		}

		int fileCount = ReadInt();
		int lastField = 0;

		for (i = 0; i < fieldCount; i++)
		{
			if (!m_global || !(pFields[i].flags & FTYPEDESC_GLOBAL))
				std::memset(((char*)pBaseData + pFields[i].fieldOffset), 0, pFields[i].fieldSize * m_Sizes[pFields[i].fieldType]);
		}

		for (i = 0; i < fileCount; i++)
		{
			HEADER header;
			BufferReadHeader(&header);

			lastField = ReadField(pBaseData, pFields, fieldCount, lastField, header.size, m_pData->pTokens[header.token], header.pData);
			lastField++;
		}

		return 1;
	}
	int ReadField(void* pBaseData, TYPEDESCRIPTION const* pFields, int fieldCount, int startField, int size, char* pName, void* pData) noexcept
	{
		float time = 0.0f;
		Vector position(0, 0, 0);

		if (m_pData)
		{
			time = m_pData->time;
			if (m_pData->fUseLandmark) {
				position = m_pData->vecLandmarkOffset;
			}
		}
		for (int i = 0; i < fieldCount; i++)
		{
			int fieldNumber = (i + startField) % fieldCount;
			auto pTest = &pFields[fieldNumber];

			if (!_stricmp(pTest->fieldName, pName))
			{
				if (!m_global || !(pTest->flags & FTYPEDESC_GLOBAL))
				{
					for (int j = 0; j < pTest->fieldSize; j++)
					{
						void* pOutputData = ((char*)pBaseData + pTest->fieldOffset + (j * m_Sizes[pTest->fieldType]));
						void* pInputData = (char*)pData + j * m_Sizes[pTest->fieldType];

						switch (pTest->fieldType)
						{
						case FIELD_TIME:
						{
							float timeData = *(float*)pInputData;
							timeData += time;
							*((float*)pOutputData) = timeData;
							break;
						}
						case FIELD_FLOAT: *((float*)pOutputData) = *(float*)pInputData; break;
						case FIELD_MODELNAME:
						case FIELD_SOUNDNAME:
						case FIELD_STRING:
						{
							char* pString = (char*)pData;
							for (int stringCount = 0; stringCount < j; stringCount++)
							{
								while (*pString)
									pString++;

								pString++;
							}

							pInputData = pString;
							if (!std::strlen((char*)pInputData))
								*((int*)pOutputData) = 0;
							else
							{
								int string = g_engfuncs.pfnAllocString((char const*)pInputData);
								*((int*)pOutputData) = string;

								if (!FStringNull(string) && m_precache)
								{
									if (pTest->fieldType == FIELD_MODELNAME)
										g_engfuncs.pfnPrecacheModel(STRING(string));
									else if (pTest->fieldType == FIELD_SOUNDNAME)
										g_engfuncs.pfnPrecacheSound(STRING(string));
								}
							}
							break;
						}
						case FIELD_EVARS:
						{
							int entityIndex = *(int*)pInputData;
							edict_t* pent = EntityFromIndex(entityIndex);

							if (pent)
								*((entvars_t**)pOutputData) = &pent->v;
							else
								*((entvars_t**)pOutputData) = nullptr;

							break;
						}
						case FIELD_CLASSPTR:
						{
							int entityIndex = *(int*)pInputData;
							edict_t* pent = EntityFromIndex(entityIndex);

							if (pent)
								*((CBaseEntity**)pOutputData) = ent_cast<CBaseEntity*>(pent);
							else
								*((CBaseEntity**)pOutputData) = nullptr;

							break;
						}
						case FIELD_EDICT:
						{
							int entityIndex = *(int*)pInputData;
							edict_t* pent = EntityFromIndex(entityIndex);
							*((edict_t**)pOutputData) = pent;
							break;
						}
						case FIELD_EHANDLE:
						{
							pOutputData = (char*)pOutputData + j * (sizeof(EHANDLE<CBaseEntity>) - m_Sizes[pTest->fieldType]);
							int entityIndex = *(int*)pInputData;
							edict_t* pent = EntityFromIndex(entityIndex);

							if (pent)
								*((EHANDLE<CBaseEntity>*)pOutputData) = ent_cast<CBaseEntity*>(pent);
							else
								*((EHANDLE<CBaseEntity>*)pOutputData) = nullptr;

							break;
						}
						case FIELD_ENTITY:
						{
							int entityIndex = *(int*)pInputData;
							edict_t* pent = EntityFromIndex(entityIndex);

							if (pent)
								*((std::intptr_t*)pOutputData) = g_engfuncs.pfnEntOffsetOfPEntity(pent);
							else
								*((std::intptr_t*)pOutputData) = 0;

							break;
						}
						case FIELD_VECTOR:
						{
							((float*)pOutputData)[0] = ((float*)pInputData)[0];
							((float*)pOutputData)[1] = ((float*)pInputData)[1];
							((float*)pOutputData)[2] = ((float*)pInputData)[2];
							break;
						}
						case FIELD_POSITION_VECTOR:
						{
							((float*)pOutputData)[0] = ((float*)pInputData)[0] + position.x;
							((float*)pOutputData)[1] = ((float*)pInputData)[1] + position.y;
							((float*)pOutputData)[2] = ((float*)pInputData)[2] + position.z;
							break;
						}
						case FIELD_BOOLEAN:
						case FIELD_INTEGER:
							*((int*)pOutputData) = *(int*)pInputData;
							break;
						case FIELD_SHORT:
							*((short*)pOutputData) = *(short*)pInputData;
							break;
						case FIELD_CHARACTER:
							*((char*)pOutputData) = *(char*)pInputData;
							break;
						case FIELD_POINTER:
							*((int*)pOutputData) = *(int*)pInputData;
							break;
						case FIELD_FUNCTION:
						{
							if (!std::strlen((char*)pInputData))
								*((int*)pOutputData) = 0;
							else
								*((int*)pOutputData) = g_engfuncs.pfnFunctionFromName((char*)pInputData);

							break;
						}
						default:
							g_engfuncs.pfnAlertMessage(at_error, "Bad field type\n");
							break;
						}
					}
				}
				return fieldNumber;
			}
		}
		return -1;
	}
	int ReadInt() noexcept
	{
		int tmp = 0;
		BufferReadBytes((char*)&tmp, sizeof(int));
		return tmp;
	}
	short ReadShort() noexcept
	{
		short tmp = 0;
		BufferReadBytes((char*)&tmp, sizeof(short));
		return tmp;
	}
	int ReadNamedInt(const char* pName) noexcept
	{
		HEADER header;
		BufferReadHeader(&header);
		return ((int*)header.pData)[0];
	}
	char* ReadNamedString(const char* pName) noexcept
	{
		HEADER header;
		BufferReadHeader(&header);
		return (char*)header.pData;
	}

	bool Empty() const noexcept { return (!m_pData || ((m_pData->pCurrentData - m_pData->pBaseData) >= m_pData->bufferSize)); }
	void SetGlobalMode(qboolean global) noexcept { m_global = global; }
	void PrecacheMode(qboolean mode) noexcept { m_precache = mode; }

private:
	char* BufferPointer() noexcept
	{
		if (!m_pData)
			return nullptr;

		return m_pData->pCurrentData;
	}
	void BufferReadBytes(char* pOutput, int size) noexcept
	{
		if (!m_pData || Empty())
			return;

		if ((m_pData->size + size) > m_pData->bufferSize)
		{
			g_engfuncs.pfnAlertMessage(at_error, "Restore overflow!");
			m_pData->size = m_pData->bufferSize;
			return;
		}

		if (pOutput)
			std::memcpy(pOutput, m_pData->pCurrentData, size);

		m_pData->pCurrentData += size;
		m_pData->size += size;
	}
	void BufferSkipBytes(int bytes) noexcept { BufferReadBytes(nullptr, bytes); }
	int BufferSkipZString() noexcept
	{
		if (!m_pData)
			return 0;

		int maxLen = m_pData->bufferSize - m_pData->size;
		int len = 0;
		char* pszSearch = m_pData->pCurrentData;

		while (*pszSearch++ && len < maxLen)
			len++;

		len++;
		BufferSkipBytes(len);
		return len;
	}
	int BufferCheckZString(const char* string) noexcept
	{
		if (!m_pData)
			return 0;

		int maxLen = m_pData->bufferSize - m_pData->size;
		int len = std::strlen(string);

		if (len <= maxLen)
		{
			if (!std::strncmp(string, m_pData->pCurrentData, len))
				return 1;
		}

		return 0;
	}
	void BufferReadHeader(HEADER* pheader) noexcept
	{
		pheader->size = ReadShort();
		pheader->token = ReadShort();
		pheader->pData = BufferPointer();

		BufferSkipBytes(pheader->size);
	}

private:
	qboolean m_global{ false };
	qboolean m_precache{ true };
};
