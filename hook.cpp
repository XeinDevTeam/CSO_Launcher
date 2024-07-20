#include "hook.h"
#include "hookutils.h"
#include <stdio.h>
#include <locale.h>
#include <ICommandLine.h>
#include <IGameUI.h>
#include <VGUI/IPanel.h>
#include "DediCsv.h"
#include "ChattingManager.h"

#define MAX_ZIP_SIZE	(1024 * 1024 * 16 )
#include "XZip.h"

#include <vector>

HMODULE g_hEngineModule;
DWORD g_dwEngineBase;
DWORD g_dwEngineSize;

DWORD g_dwGameUIBase;
DWORD g_dwGameUISize;

DWORD g_dwMpBase;
DWORD g_dwMpSize;

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT "30002"

#define SOCKETMANAGER_SIG_CSNZ23 "\xE8\x00\x00\x00\x00\xEB\x02\x33\xC0\xA3\x00\x00\x00\x00\xB9\x00\x00\x00\x00"
#define SOCKETMANAGER_MASK_CSNZ23 "x????xxxxx????x????"

#define SERVERCONNECT_SIG_CSNZ2019 "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x53\x56\x57\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00\x00\x8B\xF1\x8B\x4E\x04\x85\xC9\x74\x25\x8B\x01\x6A\x01\xFF\x10\x68\x00\x00\x00\x00"
#define SERVERCONNECT_MASK_CSNZ2019 "xxxxxx????xx????xxx????x????xxxxxxxxxxxxxx????xxxxxxxxxxxxxxxx????"

#define PARSE_W_UDP_SIG_CSNZ "\x55\x8B\xEC\x8B\x4D\x08\x8B\x41\x04"
#define PARSE_W_UDP_MASK_CSNZ "xxxxxxxxx"

#define PACKET_METADATA_PARSE_SIG_CSNZ "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x56\x57\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00\x00\x8B\xF9\x89\xBD\x00\x00\x00\x00\x8B\x45\x08\x33\xF6\x89\xB5\x00\x00\x00\x00\x89\x85\x00\x00\x00\x00\x8B\x45\x0C\xC7\x85\x00\x00\x00\x00\x00\x00\x00\x00\x89\xB5\x00\x00\x00\x00\x89\x85\x00\x00\x00\x00\x6A\x01\x8D\x85\x00\x00\x00\x00\x89\x75\xFC\x50\x8D\x8D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x0F\xB6\x95\x00\x00\x00\x00"
#define PACKET_METADATA_PARSE_MASK_CSNZ "xxxxxx????xx????xxx????x????xxxxxxxxxxxxx????xxxx????xxxxxxx????xx????xxxxx????????xx????xx????xxxx????xxxxxx????x????xxx????"
#define PACKET_QUEST_PARSE_SIG_CSNZ "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x83\xEC\x2C\x53\x56\x57\xA1\x00\x00\x00\x00\x33\xC5\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00\x00\x8B\xF9\x8B\x45\x08"
#define PACKET_QUEST_PARSE_MASK_CSNZ "xxxxxx????xx????xxxxxxxx????xxxxxxxx????xxxxx"
#define PACKET_UMSG_PARSE_SIG_CSNZ "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\xB8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x56\x57\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00\x00\x8B\xF9\x89\xBD\x00\x00\x00\x00\x8B\x45\x08\x89\x85\x00\x00\x00\x00\x8B\x45\x0C\xC7\x85\x00\x00\x00\x00\x00\x00\x00\x00\xC7\x85\x00\x00\x00\x00\x00\x00\x00\x00\x89\x85\x00\x00\x00\x00\x6A\x01\x8D\x85\x00\x00\x00\x00\xC7\x45\x00\x00\x00\x00\x00\x50\x8D\x8D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x0F\xB6\x85\x00\x00\x00\x00\x8D\x4F\x2C"
#define PACKET_UMSG_PARSE_MASK_CSNZ "xxxxxx????xx????xx????x????x????xxxxxxxxxxxxx????xxxx????xxxxx????xxxxx????????xx????????xx????xxxx????xx?????xxx????x????xxx????xxx"
#define PACKET_ALARM_PARSE_SIG_CSNZ "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x53\x56\x57\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00\x00\x8B\x45\x08\x33\xFF"
#define PACKET_ALARM_PARSE_MASK_CSNZ "xxxxxx????xx????xxx????x????xxxxxxxxxxxxxx????xxxxx"
#define PACKET_ITEM_PARSE_SIG_CSNZ "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x56\x57\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00\x00\x8B\xF1\x8B\x45\x08\xC7\x85\x00\x00\x00\x00\x00\x00\x00\x00"
#define PACKET_ITEM_PARSE_MASK_CSNZ "xxxxxx????xx????xxx????x????xxxxxxxxxxxxx????xxxxxxx????????"

#define PACKET_HOST_PTR_SIG_CSNZ "\xA1\x00\x00\x00\x00\x6A\x18\x89\x81\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4\x04\x89\x45\xF0\xC7\x45\x00\x00\x00\x00\x00\x85\xC0\x74\x09\x8B\xC8\xE8\x00\x00\x00\x00\xEB\x02\x33\xC0\x56\x8B\xC8\xC7\x45\x00\x00\x00\x00\x00\xA3\x00\x00\x00\x00\xE8\x00\x00\x00\x00"
#define PACKET_HOST_PTR_MASK_CSNZ "x????xxxx????x????xxxxxxxx?????xxxxxxx????xxxxxxxxx?????x????x????"

#define BOT_MANAGER_PTR_SIG_CSNZ "\xA3\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x83\xC4\x04\x8B\x4D\xF4"
#define BOT_MANAGER_PTR_MASK_CSNZ "x????xx????xxxxxx"

#define SHOWLOGINDLG_SIG_CSNZ "\xA1\x00\x00\x00\x00\x56\x57\x8B\xF9\x8B\x80\x00\x00\x00\x00\xFF\xD0\x8B\xF0\x8B\xCE\x8B\x16\x8B\x52\x24"
#define SHOWLOGINDLG_MASK_CSNZ "x????xxxxxx????xxxxxxxxxxx"

#define CSOMAINPANEL_PTR_SIG_CSNZ "\x8B\x0D\x00\x00\x00\x00\x6A\x01\x8B\x01\xFF\x90\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x6A\x01\xE8\x00\x00\x00\x00\x8B\x03"
#define CSOMAINPANEL_PTR_MASK_CSNZ "xx????xxxxxx????xx????xxx????xx"

#define CALL_PANEL_FINDCHILDBYNAME_SIG_CSNZ "\xE8\x00\x00\x00\x00\x89\x47\xC8"
#define CALL_PANEL_FINDCHILDBYNAME_MASK_CSNZ "x????xxx"

#define CALL_BLACKCIPHER_INIT_SIG "\xE8\x00\x00\x00\x00\x84\xC0\x75\x0C\xE8\x00\x00\x00\x00"
#define CALL_BLACKCIPHER_INIT_MASK "x????xxxxx????"

#define PACKET_REPORT_PTR_FUNC_REF_SIG "\xE8\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x8B\x01\xFF\x10"
#define PACKET_REPORT_PTR_FUNC_REF_MASK "x????xx????x????xx????xxxx"

#define PACKET_CRYPT_SIG "\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\xF0\x53\x56\x57\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00\x00\x8B\x45\x08\x89\x85\x00\x00\x00\x00\x8B\x45\x0C\xC7\x85\x00\x00\x00\x00\x00\x00\x00\x00\xC7\x85\x00\x00\x00\x00\x00\x00\x00\x00\x89\x85\x00\x00\x00\x00\x6A\x01\x8D\x85\x00\x00\x00\x00\xC7\x45\x00\x00\x00\x00\x00\x50\x8D\x8D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x0F\xB6\x9D\x00\x00\x00\x00"
#define PACKET_CRYPT_MASK "xxxxxx????xx????xxx????x????xxxxxxxxxxxxxx????xxxxx????xxxxx????????xx????????xx????xxxx????xx?????xxx????x????xxx????"

#define PACKET_HACK_SIG "\x55\x8B\xEC\x6A\x00\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x83\xEC\x00\x53\x56\x57\xA1\x00\x00\x00\x00\x33\xC5\x50\x8D\x45\x00\x64\xA3\x00\x00\x00\x00\x8B\xD9\x89\x5D\x00\x8B\x45\x00\x89\x45\x00\x8B\x45\x00\xC7\x45\x00\x00\x00\x00\x00\xC7\x45\x00\x00\x00\x00\x00\x89\x45\x00\x6A\x00\x8D\x45\x00\xC7\x45\x00\x00\x00\x00\x00\x50\x8D\x4D\x00\xE8\x00\x00\x00\x00\x0F\xB6\x45\x00\x89\x43\x00\x83\xE8"
#define PACKET_HACK_MASK "xxxx?x????xx????xxx?xxxx????xxxxx?xx????xxxx?xx?xx?xx?xx?????xx?????xx?x?xx?xx?????xxx?x????xxx?xx?xx"

char g_pServerIP[16];
char g_pServerPort[6];
char g_pLogin[64];
char g_pPassword[64];

bool g_bUseOriginalServer = false;
bool g_bDumpMetadata = false;
bool g_bIgnoreMetadata = false;
bool g_bDumpQuest = false;
bool g_bDumpUMsg = false;
bool g_bDumpAlarm = false;
bool g_bDumpItem = false;
bool g_bDumpCrypt = false;
bool g_bDumpAll = false;
bool g_bDisableAuthUI = false;
bool g_bDumpUDP = false;
bool g_bUseSSL = false;
bool g_bWriteMetadata = false;
bool g_bLoadZBSkillFromFile = false;
bool g_bRegister = false;
bool g_bNoNGHook = false;

void* g_pSocketManager = nullptr;

cl_enginefunc_t* g_pEngine;

class CCSBotManager
{
public:
	virtual void Unknown() = NULL;
	virtual void Bot_Add(int side) = NULL;
};

CCSBotManager* g_pBotManager = NULL;

vgui::IPanel* g_pPanel = nullptr;
IGameUI* g_pGameUI = nullptr;
ChattingManager* g_pChattingManager;

WNDPROC oWndProc;
HWND hWnd;

void* g_pSocketManagerConstructor;
void* (__thiscall* g_pfnSocketManagerConstructor)(void* _this, bool useSSL);

void* g_pServerConnect;

int(__thiscall* g_pfnGameUI_RunFrame)(void* _this);

typedef void* (__thiscall* tPanel_FindChildByName)(void* _this, const char* name, bool recurseDown);
tPanel_FindChildByName g_pfnPanel_FindChildByName;

typedef int(__thiscall* tLoginDlg_OnCommand)(void* _this, const char* command);
tLoginDlg_OnCommand g_pfnLoginDlg_OnCommand;

typedef bool(__thiscall* tCreateStringTable)(int* _this, const char* filename);
tCreateStringTable g_pfnCreateStringTable;

typedef void(__thiscall* tParseCSV)(int* _this, unsigned char* buffer, int size);
tParseCSV g_pfnParseCSV;

void* g_pPacket_Metadata_Parse;
int(__thiscall* g_pfnPacket_Metadata_Parse)(void* _this, void* packetBuffer, int packetSize);

void* g_pPacket_Quest_Parse;
int(__thiscall* g_pfnPacket_Quest_Parse)(void* _this, void* packetBuffer, int packetSize);

void* g_pPacket_UMsg_Parse;
int(__thiscall* g_pfnPacket_UMsg_Parse)(void* _this, void* packetBuffer, int packetSize);

void* g_pPacket_Alarm_Parse;
int(__thiscall* g_pfnPacket_Alarm_Parse)(void* _this, void* packetBuffer, int packetSize);

void* g_pPacket_Item_Parse;
int(__thiscall* g_pfnPacket_Item_Parse)(void* _this, void* packetBuffer, int packetSize);

void* g_pPacket_Crypt_Parse;
int(__thiscall* g_pfnPacket_Crypt_Parse)(void* _this, void* packetBuffer, int packetSize);

void* g_pPacket_Host;
typedef int(__thiscall* tPacket_Host_Parse)(void* _this, void* packetBuffer, int packetSize);
tPacket_Host_Parse g_pfnPacket_Host_Parse;

const char*(__thiscall* g_pfnGetCryptoProtocolName)(void* _this);

void* (__thiscall* g_pfnSocketConstructor)(int _this, int a2, int a3, char a4);

int (__thiscall* g_pfnSocket_Read)(void* _this, char* outBuf, int len, void* a4, bool initialMsg);

typedef void*(*tEVP_CIPHER_CTX_new)();
tEVP_CIPHER_CTX_new g_pfnEVP_CIPHER_CTX_new;

#pragma region Nexon NGClient/NXGSM
char NGClient_Return1()
{
	return 1;
}

void NGClient_Void()
{
}

// logger shit
bool NXGSM_Dummy()
{
	return false;
}

void NXGSM_WriteStageLogA(int a1, char* a2)
{
}

void NXGSM_WriteErrorLogA(int a1, char* a2)
{
}
#pragma endregion

void Pbuf_AddText(const char* text)
{
	g_pEngine->pfnClientCmd((char*)text);
}

void* __fastcall SockMgr(void* __this, int reg, bool useSSL)
{
	return g_pfnSocketManagerConstructor(__this, g_bUseSSL);
}

CreateHookClass(int, ServerConnect, unsigned long ip, unsigned short port, bool validate)
{
	return g_pfnServerConnect(ptr, inet_addr(g_pServerIP), htons(atoi(g_pServerPort)), validate);
}

CreateHook(__cdecl, void, HolePunch__SetServerInfo, unsigned long ip, unsigned short port)
{
	g_pfnHolePunch__SetServerInfo(inet_addr(g_pServerIP), htons(atoi(g_pServerPort)));
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == 0x113 && wParam == 250)
	{
		// handle dropclient msg if the client detected abnormal things
		printf("handle_dropclient\n");
		return 0;
	}
	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool LoadCsv(int* _this, const char* filename, unsigned char* defaultBuf, int defaultBufSize, bool fromFile)
{
	unsigned char* buffer = NULL;
	long size = 0;
	if (g_bLoadZBSkillFromFile)
	{
		FILE* f = fopen("ZombieSkillTable.csv", "rb");
		if (!f)
		{
			return g_pfnCreateStringTable(_this, filename);
		}

		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fseek(f, 0, SEEK_SET);

		if (size <= 0)
		{
			return g_pfnCreateStringTable(_this, filename);
		}

		buffer = (unsigned char*)malloc(size + 1);
		if (!buffer)
		{
			return g_pfnCreateStringTable(_this, filename);
		}

		fread(buffer, 1, size, f);
		fclose(f);
	}
	else
	{
		buffer = defaultBuf;
		size = defaultBufSize;
	}

	g_pfnParseCSV(_this, buffer, size);

	bool result = 0;
	if (_this[2])
		result = _this[3] != 0;

	return result;
}

bool __fastcall CreateStringTable(int* _this, int shit, const char* filename)
{
	if (!strcmp(filename, "resource/zombi/ZombieSkillTable_Dedi.csv"))
	{
		return LoadCsv(_this, filename, g_ZBSkill, sizeof(g_ZBSkill), g_bLoadZBSkillFromFile);
	}
	else if (!strcmp(filename, "resource/allstar/AllStar_Status-Dedi.csv"))
	{
		return LoadCsv(_this, filename, g_AllStar_Status, sizeof(g_AllStar_Status), false);
	}
	else if (!strcmp(filename, "resource/allstar/AllStar_Skill-Dedi.csv"))
	{
		return LoadCsv(_this, filename, g_AllStar_Skill, sizeof(g_AllStar_Skill), false);
	}

	//printf("%s\n", filename);
	return g_pfnCreateStringTable(_this, filename);
}

const char* GetCSVMetadataName(int metaDataID)
{
	switch (metaDataID)
	{
	case 0:
		return "MapList.csv";
	case 1:
		return "ClientTable.csv";
	case 2:
		return "ModeList.csv";
	case 9:
		return "MatchOption.csv";
	case 17:
		return "weaponparts.csv";
	case 18:
		return "MileageShop.csv";
	case 24:
		return "GameModeList.csv";
	case 25:
		return "badwordadd.csv";
	case 26:
		return "badworddel.csv";
	case 27:
		return "progress_unlock.csv";
	case 28:
		return "ReinforceMaxLv.csv";
	case 29:
		return "ReinforceMaxExp.csv";
	case 32:
		return "Item.csv";
	case 33:
		return "voxel_list.csv";
	case 34:
		return "voxel_item.csv";
	case 35:
		return "CodisData.csv";
	case 36:
		return "HonorMoneyShop.csv";
	case 37:
		return "ItemExpireTime.csv";
	case 38:
		return "scenariotx_common.json";
	case 39:
		return "scenariotx_dedi.json";
	case 40:
		return "shopitemlist_dedi.json";
	case 41:
		return "EpicPieceShop.csv";
	case 42:
		return "WeaponProp.json";
	case 44:
		return "SeasonBadgeShop.csv";
	case 45:
		return "ppsystem.json";
	case 46:
		return "classmastery.json";
	case 48:
		return "ZBCompetitive.json"; // required or game will crash
	case 50:
		return "ModeEvent.csv";
	case 51:
		return "EventShop.csv";
	}
	return NULL;
}

const char* GetBinMetadataName(int metaDataID)
{
	switch (metaDataID)
	{
	case 6:
		return "PaintItemList";
	case 16:
		return "RandomWeaponList";
	case 30:
		return "ReinforceItemsExp";
	}
	return NULL;
}

#pragma region Packet

class Packet
{
public:
	int unk;
	void* ptr;
	void* ptr2;
	int unk1;
	int unk2;
	int unk3;
	int unk4;
};

int __fastcall Packet_Metadata_Parse(Packet* _this, int a2, void* packetBuffer, int packetSize)
{
	if (g_bIgnoreMetadata)
	{
		return false;
	}

	unsigned char metaDataID = *(unsigned char*)packetBuffer;
	printf("%d\n", metaDataID);

	bool csvMetaData = true;
	const char* metaDataName = GetCSVMetadataName(metaDataID);
	if (!metaDataName)
	{
		metaDataName = GetBinMetadataName(metaDataID);
		csvMetaData = false;
	}

	if (g_bDumpMetadata)
	{
		char name[MAX_PATH];
		FILE* file = NULL;
		unsigned short metaDataSize = 0;

		CreateDirectory("MetadataDump", NULL);

		if (metaDataName)
		{
			if (csvMetaData)
				metaDataSize = *((unsigned short*)((char*)packetBuffer + 1));

			sprintf_s(name, "MetadataDump/Metadata_%s.bin", metaDataName);
		}
		else
		{
			sprintf_s(name, "MetadataDump/Metadata_Unk%d.bin", metaDataID);
		}

		file = fopen(name, "wb");
		if (!file)
		{
			printf("Can't open '%s' file to write metadata dump\n", name);
		}
		else
		{
			if (metaDataSize && csvMetaData)
			{
				fwrite(((unsigned short*)((char*)packetBuffer + 3)), metaDataSize, 1, file);
			}
			else
			{
				fwrite(packetBuffer, packetSize, 1, file);
			}
			fclose(file);
		}
	}

	if (g_bWriteMetadata && metaDataName != NULL)
	{
		HZIP hMetaData = CreateZip(0, MAX_ZIP_SIZE, ZIP_MEMORY);

		if (!hMetaData)
		{
			printf("CreateZip returned NULL.\n");
			return g_pfnPacket_Metadata_Parse(_this, packetBuffer, packetSize);
		}

		char path[MAX_PATH];
		sprintf(path, "Metadata/%s", metaDataName);
		printf("%s\n", path);

		if (ZipAdd(hMetaData, metaDataName, path, 0, ZIP_FILENAME))
		{
			printf("ZipAdd returned error.\n");
			return g_pfnPacket_Metadata_Parse(_this, packetBuffer, packetSize);
		}

		void* buffer;
		unsigned long length = 0;
		ZipGetMemory(hMetaData, &buffer, &length);

		if (length == 0)
		{
			printf("ZipGetMemory returned zero length.\n");
			return g_pfnPacket_Metadata_Parse(_this, packetBuffer, packetSize);
		}

		std::vector<unsigned char> destBuffer;
		std::vector<unsigned char> metaDataBuffer((char*)buffer, (char*)buffer + length);

		destBuffer.push_back(metaDataID);
		destBuffer.push_back((unsigned char)(length & 0xFF));
		destBuffer.push_back((unsigned char)(length >> 8));
		destBuffer.insert(destBuffer.end(), metaDataBuffer.begin(), metaDataBuffer.end());

		CloseZip(hMetaData);

		return g_pfnPacket_Metadata_Parse(_this, static_cast<void*>(destBuffer.data()), destBuffer.size());
	}

	return g_pfnPacket_Metadata_Parse(_this, packetBuffer, packetSize);
}

int counter = 0;
void DumpPacket(const char* packetName, void* packetBuffer, int packetSize)
{
	//char subType = *(char*)packetBuffer;

	CreateDirectory(packetName, NULL);

	char name[MAX_PATH];
	sprintf_s(name, "%s/%s_%d.bin", packetName, packetName, counter++);

	FILE* file = fopen(name, "wb");
	if (file)
	{
		fwrite(packetBuffer, packetSize, 1, file);
		fclose(file);
	}
	else
	{
		printf("Can't open '%s' file to write %s dump\n", name, packetName);
	}
}

int __fastcall Packet_Quest_Parse(Packet* _this, int a2, void* packetBuffer, int packetSize)
{
	DumpPacket("Quest", packetBuffer, packetSize);
	return g_pfnPacket_Quest_Parse(_this, packetBuffer, packetSize);
}

int __fastcall Packet_UMsg_Parse(Packet* _this, int a2, void* packetBuffer, int packetSize)
{
	DumpPacket("UMsg", packetBuffer, packetSize);
	return g_pfnPacket_UMsg_Parse(_this, packetBuffer, packetSize);
}

int __fastcall Packet_Alarm_Parse(Packet* _this, int a2, void* packetBuffer, int packetSize)
{
	DumpPacket("Alarm", packetBuffer, packetSize);
	return g_pfnPacket_Alarm_Parse(_this, packetBuffer, packetSize);
}

int __fastcall Packet_Item_Parse(Packet* _this, int a2, void* packetBuffer, int packetSize)
{
	DumpPacket("Item", packetBuffer, packetSize);
	return g_pfnPacket_Item_Parse(_this, packetBuffer, packetSize);
}

int __fastcall Packet_Crypt_Parse(Packet* _this, int a2, void* packetBuffer, int packetSize)
{
	DumpPacket("Crypt", packetBuffer, packetSize);
	return g_pfnPacket_Crypt_Parse(_this, packetBuffer, packetSize);
}

int __fastcall Packet_Host_Parse(Packet* _this, int a2, void* packetBuffer, int packetSize)
{
	char subType = *(char*)packetBuffer;
	printf("%d\n", subType);
	if (subType == 1 || subType == 5)
	{
		// replace packet buffer with our modified
		return false;
	}

	return g_pfnPacket_Host_Parse(_this, packetBuffer, packetSize);
}

int __fastcall Packet_Hack_Parse(Packet* _this, int a2, void* packetBuffer, int packetSize)
{
	return 1;
}
#pragma endregion

void __fastcall LoginDlg_OnCommand(void* _this, int r, const char* command)
{
	if (!strcmp(command, "Login"))
	{
		DWORD** v3 = (DWORD**)_this;
		char login[256];
		char password[256];

		//void* pLoginTextEntry = g_pfnPanel_FindChildByName(_this, "1");
		//void* pPasswordTextEntry = g_pfnPanel_FindChildByName(_this, "1");
		(*(void(__thiscall**)(DWORD*, char*, signed int))(*v3[109] + 624))(v3[109], login, 256); // textentry->GetText() // before 23.12.23 *v3[109] + 620
		(*(void(__thiscall**)(DWORD*, char*, signed int))(*v3[110] + 624))(v3[110], password, 256);

		wchar_t buf[256];
		swprintf(buf, L"/login %S %S", login, password);
		g_pChattingManager->PrintToChat(1, buf);
		return;
	}
	else if (!strcmp(command, "Register"))
	{
		DWORD** v3 = (DWORD**)_this;
		char login[256];
		char password[256];

		(*(void(__thiscall**)(DWORD*, char*, signed int))(*v3[109] + 624))(v3[109], login, 256); // textentry->GetText()
		(*(void(__thiscall**)(DWORD*, char*, signed int))(*v3[110] + 624))(v3[110], password, 256);

		wchar_t buf[256];
		swprintf(buf, L"/register %S %S", login, password);
		g_pChattingManager->PrintToChat(1, buf);
		return;
	}

	g_pfnLoginDlg_OnCommand(_this, command);
}

bool bShowLoginDlg = false;
int __fastcall GameUI_RunFrame(void* _this)
{
	if (!bShowLoginDlg)
	{
		if (strlen(g_pLogin) != 0 || strlen(g_pPassword) != 0)
		{
			wchar_t buf[256];
			swprintf(buf, g_bRegister ? L"/register %S %S" : L"/login %S %S", g_pLogin, g_pPassword);
			g_pChattingManager->PrintToChat(1, buf);
		}

		if (!g_bDisableAuthUI)
		{
			__try
			{
				void* pCSOMainPanel = **((void***)(FindPattern(CSOMAINPANEL_PTR_SIG_CSNZ, CSOMAINPANEL_PTR_MASK_CSNZ, g_dwGameUIBase, g_dwGameUIBase + g_dwGameUISize, 2)));
				if (!pCSOMainPanel)
				{
					MessageBox(NULL, "pCSOMainPanel == NULL!!!\nUse -disableauthui parameter to disable VGUI login dialog", "Error", MB_OK);
					bShowLoginDlg = true;
					return g_pfnGameUI_RunFrame(_this);
				}

				DWORD dwPanel_FindChildByNameRelAddr = FindPattern(CALL_PANEL_FINDCHILDBYNAME_SIG_CSNZ, CALL_PANEL_FINDCHILDBYNAME_MASK_CSNZ, g_dwGameUIBase, g_dwGameUIBase + g_dwGameUISize, 1);
				if (!dwPanel_FindChildByNameRelAddr)
				{
					MessageBox(NULL, "dwPanel_FindChildByNameRelAddr == NULL!!!\nUse -disableauthui parameter to disable VGUI login dialog", "Error", MB_OK);
					bShowLoginDlg = true;
					return g_pfnGameUI_RunFrame(_this);
				}
				
				g_pfnPanel_FindChildByName = (tPanel_FindChildByName)(dwPanel_FindChildByNameRelAddr + 4 + *(DWORD*)dwPanel_FindChildByNameRelAddr);
				if (!g_pfnPanel_FindChildByName)
				{
					MessageBox(NULL, "g_pfnPanel_FindChildByName == NULL!!!\nUse -disableauthui parameter to disable VGUI login dialog", "Error", MB_OK);
					bShowLoginDlg = true;
					return g_pfnGameUI_RunFrame(_this);
				}

				void* pLoginDlg = *(void**)((DWORD)pCSOMainPanel + 364);
				if (!pLoginDlg)
				{
					MessageBox(NULL, "pLoginDlg == NULL!!!\nUse -disableauthui parameter to disable VGUI login dialog", "Error", MB_OK);
					bShowLoginDlg = true;
					return g_pfnGameUI_RunFrame(_this);
				}

				VFTHook(pLoginDlg, 0, 98, LoginDlg_OnCommand, (void*&)g_pfnLoginDlg_OnCommand);

				void* pRegisterBtn = g_pfnPanel_FindChildByName(pLoginDlg, "RegisterBtn", false);
				void* pFindIDBtn = g_pfnPanel_FindChildByName(pLoginDlg, "FindIDBtn", false);
				void* pFindPWBtn = g_pfnPanel_FindChildByName(pLoginDlg, "FindPWBtn", false);
				void* pImagePanel1 = g_pfnPanel_FindChildByName(pLoginDlg, "ImagePanel1", false);

				if (!pRegisterBtn || !pFindIDBtn || !pFindPWBtn || !pImagePanel1)
				{
					MessageBox(NULL, "Invalid ptrs!!!\nUse -disableauthui parameter to disable VGUI login dialog", "Error", MB_OK);
					bShowLoginDlg = true;
					return g_pfnGameUI_RunFrame(_this);
				}

				void* v27 = (**(void* (__thiscall***)(void*))pRegisterBtn)(pRegisterBtn);
				g_pPanel->SetPos((vgui::IPanel*)v27, 50, 141);
				//(*(void(__stdcall**)(void*, int, int))(*(DWORD*)pRegisterBtn + 4))(pRegisterBtn, 50, 141); // button->SetPos()
				(*(void(__thiscall**)(void*, bool))(*(DWORD*)pFindIDBtn + 160))(pFindIDBtn, false); // button->SetVisible()
				(*(void(__thiscall**)(void*, bool))(*(DWORD*)pFindPWBtn + 160))(pFindPWBtn, false); // button->SetVisible()
				(*(void(__thiscall**)(void*, const char*))(*(DWORD*)pRegisterBtn + 608))(pRegisterBtn, "Register"); // button->SetText() // before 23.12.23 pRegisterBtn + 604
				//(*(void(__thiscall**)(void*, const char*))(*(DWORD*)pImagePanel1 + 600))(pImagePanel1, "resource/login.tga"); // imagepanel->SetImage()
				(*(void(__thiscall**)(void*))(*(DWORD*)pLoginDlg + 836))(pLoginDlg); // loginDlg->DoModal() // before 23.12.23 pLoginDlg + 832

				// i lost fucking g_pfnShowLoginDlg reference...
				/*if (g_pfnShowLoginDlg)
				{
					g_pfnShowLoginDlg(g_pCSOMainPanel);
				}
				else
				{
					MessageBox(NULL, "g_pfnShowLoginDlg == NULL!!!\nUse -disableauthui parameter to disable VGUI login dialog", "Error", MB_OK);
				}*/
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				MessageBox(NULL, "Something went wrong while initializing the Auth UI!!!\nUse -disableauthui parameter to disable VGUI login dialog", "Error", MB_OK);
			}
		}
		bShowLoginDlg = true;
	}
	return g_pfnGameUI_RunFrame(_this);
}

void CSO_Bot_Add()
{
	// get current botmgr ptr
	DWORD dwBotManagerPtr = FindPattern(BOT_MANAGER_PTR_SIG_CSNZ, BOT_MANAGER_PTR_MASK_CSNZ, g_dwMpBase, g_dwMpBase + g_dwMpSize, 1);
	g_pBotManager = **((CCSBotManager***)(dwBotManagerPtr));

	if (!g_pBotManager)
	{
		g_pEngine->Con_Printf("CSO_Bot_Add: g_pBotManager == NULL\n");
		return;
	}
	int arg1 = 0, arg2 = 0;
	int argc = g_pEngine->Cmd_Argc();
	if (argc > 0)
	{
		arg1 = atoi(g_pEngine->Cmd_Argv(1));
		if (argc >= 2)
		{
			arg2 = atoi(g_pEngine->Cmd_Argv(2));
		}
	}
	g_pBotManager->Bot_Add(arg1);
}

const char* __fastcall GetCryptoProtocolName(void* _this)
{
	if (g_bUseSSL)
	{
		return g_pfnGetCryptoProtocolName(_this);
	}

	return "None";
}

void* __fastcall SocketConstructor(int _this, int reg, int a2, int a3, char a4)
{
	*(DWORD*)(_this + 72) = (DWORD)g_pfnEVP_CIPHER_CTX_new();
	*(DWORD*)(_this + 76) = (DWORD)g_pfnEVP_CIPHER_CTX_new();
	*(DWORD*)(_this + 80) = (DWORD)g_pfnEVP_CIPHER_CTX_new();
	*(DWORD*)(_this + 84) = (DWORD)g_pfnEVP_CIPHER_CTX_new();

	return g_pfnSocketConstructor(_this, a2, a3, a4);
}

int __fastcall Socket_Read(void* _this, int reg, char* outBuf, int len, unsigned short *outLen, bool initialMsg)
{
	int result = g_pfnSocket_Read(_this, outBuf, len, outLen, initialMsg);

	// this + 0x34 - read buf

	// 0 - got message, 4 - wrong header, 6 - idk, 7 - got less than 4 bytes, 8 - bad sequence
	if (!initialMsg && result == 0)
	{
		// create folder
		CreateDirectory("Packets", NULL);

		static int directoryCounter = 0;
		if (!directoryCounter)
		{
			while (true)
			{
				char directory[MAX_PATH];
				snprintf(directory, sizeof(directory), "Packets/%d", ++directoryCounter);

				DWORD dwAttr = GetFileAttributes(directory);
				if (dwAttr != 0xffffffff && (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
				{
					continue;
				}

				CreateDirectory(directory, NULL);
				break;
			}
		}

		// write file
		unsigned char* buf = (unsigned char*)(outBuf);
		unsigned short dataLen = *outLen;

		static int packetCounter = 0;

		char filename[MAX_PATH];
		bool moreInfo = false;
		if (moreInfo)
			snprintf(filename, sizeof(filename), "Packets/%d/Packet_%d_ID_%d_%d.bin", directoryCounter, packetCounter++, buf[0], dataLen);
		else
			snprintf(filename, sizeof(filename), "Packets/%d/Packet_%d.bin", directoryCounter, packetCounter++);

		FILE* file = fopen(filename, "wb");
		fwrite(buf, dataLen, 1, file);
		fclose(file);
	}

	return result;
}

CreateHook(__cdecl, void, LogToErrorLog, void* pLogFile, char* fmt, ...)
{
	char outputString[4096];

	va_list va;
	va_start(va, fmt);
	_vsnprintf_s(outputString, sizeof(outputString), fmt, va);
	outputString[4095] = 0;
	va_end(va);

	printf("[LogToErrorLog] %s", outputString);

	g_pfnLogToErrorLog(pLogFile, outputString);
}

CreateHook(WINAPI, void, OutputDebugStringA, LPCSTR lpOutString)
{
	printf("[OutputDebugString] %s\n", lpOutString);
}

CreateHook(__cdecl, int, HolePunch__GetUserSocketInfo, int userID, char* data)
{
	auto ret = g_pfnHolePunch__GetUserSocketInfo(userID, data);

	data[0] = 2; // unsafety method, since other places port are corrected

	short port = (short&)data[14];
	in_addr ip = (in_addr&)data[16];

	printf("[HolePunch__GetUserSocketInfo] ret: %d | UserID: %d, %s:%d\n", ret, userID, inet_ntoa(ip), ntohs(port));

	return ret;
}

void CreateDebugConsole()
{
	AllocConsole();

	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	SetConsoleTitleA("CSO launcher debug console");
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	setlocale(LC_ALL, "");
}

DWORD WINAPI HookThread(LPVOID lpThreadParameter)
{
	hWnd = FindWindow(NULL, "Counter-Strike Nexon: Studio");
	oWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

	if (!g_bUseOriginalServer)
	{
		while (!g_dwGameUIBase) // wait for gameui module
		{
			g_dwGameUIBase = (DWORD)GetModuleHandle("gameui.dll");
			Sleep(500);
		}

		g_dwGameUISize = GetModuleSize(GetModuleHandle("gameui.dll"));

		g_pChattingManager = g_pEngine->GetChatManager();
		if (!g_pChattingManager)
			MessageBox(NULL, "g_pChattingManager == NULL!!!", "Error", MB_OK);

		CreateInterfaceFn gameui_factory = CaptureFactory("gameui.dll");
		CreateInterfaceFn vgui2_factory = CaptureFactory("vgui2.dll");
		g_pGameUI = (IGameUI*)(CaptureInterface(gameui_factory, GAMEUI_INTERFACE_VERSION));
		g_pPanel = (vgui::IPanel*)(CaptureInterface(vgui2_factory, VGUI_PANEL_INTERFACE_VERSION));
		VFTHook(g_pGameUI, 0, 7, GameUI_RunFrame, (void*&)g_pfnGameUI_RunFrame);

		while (!g_dwMpBase) // wait for mp.dll module
		{
			g_dwMpBase = (DWORD)GetModuleHandle("mp.dll");
			Sleep(500);
		}
		g_dwMpSize = GetModuleSize(GetModuleHandle("mp.dll"));

		{
			// NOP IsDedi() function to load allstar Skill/Status csv
			DWORD pushStr = FindPush(g_dwMpBase, g_dwMpBase + g_dwMpSize, (PCHAR)("Failed to Open AllStar_Skill-Dedi Table"));
			// call FF 15
			// test 85
			// jz 0F 84
			// cmp 80 7D
			// 90 90 90 90 90 90 90 90 90 90 90 90 90 90 80 7D
			DWORD patchAddr = pushStr - 0x1B;
			unsigned char patch[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
			WriteMemory((void*)patchAddr, (BYTE*)patch, 14);

			pushStr = FindPush(g_dwMpBase, g_dwMpBase + g_dwMpSize, (PCHAR)("Failed to Open AllStar_Status-Dedi Table"));
			patchAddr = pushStr - 0x1E; // or 0x1B?
			WriteMemory((void*)patchAddr, (BYTE*)patch, 14);
		}

		g_pEngine->pfnAddCommand("cso_bot_add", CSO_Bot_Add);
	}

	return TRUE;
}

void Init(HMODULE hModule)
{
	printf("Init()\n");

	if (CommandLine()->CheckParm("-debug") || CommandLine()->CheckParm("-dev") || CommandLine()->CheckParm("+developer 1") || CommandLine()->CheckParm("-developer"))
		CreateDebugConsole();

	g_hEngineModule = hModule;
	g_dwEngineBase = GetModuleBase(g_hEngineModule);
	g_dwEngineSize = GetModuleSize(g_hEngineModule);

	const char* port;
	const char* ip;

	if (CommandLine()->CheckParm("-ip", &ip) && ip)
	{
		strncpy(g_pServerIP, ip, sizeof(g_pServerIP));
	}
	else
	{
		strncpy(g_pServerIP, DEFAULT_IP, sizeof(DEFAULT_IP));
	}

	if (CommandLine()->CheckParm("-port", &port) && port)
	{
		strncpy(g_pServerPort, port, sizeof(g_pServerPort));
	}
	else
	{
		strncpy(g_pServerPort, DEFAULT_PORT, sizeof(DEFAULT_PORT));
	}

	const char* login;
	const char* password;

	g_bRegister = CommandLine()->CheckParm("-register");
	if (CommandLine()->CheckParm("-login", &login) && login)
	{
		strncpy(g_pLogin, login, sizeof(g_pLogin));
		printf("g_pLogin = %s\n", g_pLogin);
	}
	if (CommandLine()->CheckParm("-password", &password) && password)
	{
		strncpy(g_pPassword, password, sizeof(g_pPassword));
		printf("g_pPassword = %s\n", g_pPassword);
	}

	g_bUseOriginalServer = CommandLine()->CheckParm("-useoriginalserver");
	g_bDumpMetadata = CommandLine()->CheckParm("-dumpmetadata");
	g_bIgnoreMetadata = CommandLine()->CheckParm("-ignoremetadata");
	g_bDumpQuest = CommandLine()->CheckParm("-dumpquest");
	g_bDumpUMsg = CommandLine()->CheckParm("-dumpumsg");
	g_bDumpAlarm = CommandLine()->CheckParm("-dumpalarm");
	g_bDumpItem = CommandLine()->CheckParm("-dumpitem");
	g_bDumpAll = CommandLine()->CheckParm("-dumpall");
	g_bDumpCrypt = CommandLine()->CheckParm("-dumpcrypt");
	g_bDisableAuthUI = CommandLine()->CheckParm("-disableauthui");
	g_bDumpUDP = CommandLine()->CheckParm("-dumpudp");
	g_bUseSSL = CommandLine()->CheckParm("-usessl");
	g_bWriteMetadata = CommandLine()->CheckParm("-writemetadata");
	g_bLoadZBSkillFromFile = CommandLine()->CheckParm("-loadzbskillfromfile");
	g_bNoNGHook = CommandLine()->CheckParm("-nonghook");

	printf("g_pServerIP = %s, g_pServerPort = %s\n", g_pServerIP, g_pServerPort);
}

void Hook(HMODULE hModule)
{
	Init(hModule);

	void* dummy = NULL;
	
	if (!g_bNoNGHook)
	{
		DWORD find = FindPattern("\xE8\x00\x00\x00\x00\x84\xC0\x75\x00\xE8\x00\x00\x00\x00\x33\xC0", "x????xxx?x????xx", g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, NULL);
		if (!find)
			MessageBox(NULL, "NGClient_Init == NULL!!!", "Error", MB_OK);
		else
			InlineHookFromCallOpcode((void*)find, NGClient_Return1, dummy, dummy);

		find = FindPattern("\xE8\x00\x00\x00\x00\x33\xC0\xE9\x00\x00\x00\x00\xEB", "x????xxx????x", g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, NULL);
		if (!find)
			MessageBox(NULL, "NGClient_Quit == NULL!!!", "Error", MB_OK);
		else
			InlineHookFromCallOpcode((void*)find, NGClient_Void, dummy, dummy);

		find = FindPattern("\xE8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xEB\x00\x43\x56\x20\x20\x0D", "x????x????x?xxxxx", g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, NULL);
		if (!find)
			MessageBox(NULL, "Packet_Hack_Send == NULL!!!", "Error", MB_OK);
		else
		{
			InlineHookFromCallOpcode((void*)find, NGClient_Void, dummy, dummy);
			InlineHookFromCallOpcode((void*)(find + 0x5), NGClient_Return1, dummy, dummy);
		}

		find = FindPattern(PACKET_HACK_SIG, PACKET_HACK_MASK, g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, NULL);
		if (!find)
			MessageBox(NULL, "Packet_Hack_Receive == NULL!!!", "Error", MB_OK);
		else
			InlineHook((void*)find, Packet_Hack_Parse, dummy);
	}

	IATHook(g_hEngineModule, "nxgsm.dll", "InitializeGameLogManagerA", NXGSM_Dummy, dummy);
	IATHook(g_hEngineModule, "nxgsm.dll", "WriteStageLogA", NXGSM_WriteStageLogA, dummy);
	IATHook(g_hEngineModule, "nxgsm.dll", "WriteErrorLogA", NXGSM_WriteErrorLogA, dummy);
	IATHook(g_hEngineModule, "nxgsm.dll", "FinalizeGameLogManager", NXGSM_Dummy, dummy);
	IATHook(g_hEngineModule, "nxgsm.dll", "SetUserSN", NXGSM_Dummy, dummy);
	if (g_bDumpUDP)
	{
		//InlineHook(GetProcAddress(GetModuleHandleA("WSOCK32.dll"), "recvfrom"), h_Recvfrom, (void*&)g_pfnRecvfrom);
		//InlineHook(GetProcAddress(GetModuleHandleA("WSOCK32.dll"), "sendto"), h_Sendto, (void*&)g_pfnSendto);
	}

	if (!g_bUseOriginalServer)
	{
		DWORD dwCallSocketMgrInitAddr = FindPattern(SOCKETMANAGER_SIG_CSNZ23, SOCKETMANAGER_MASK_CSNZ23, g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, NULL);
		if (!dwCallSocketMgrInitAddr)
			MessageBox(NULL, "dwCallSocketMgrInitAddr == NULL!!!", "Error", MB_OK);
		else
			InlineHookFromCallOpcode((void*)dwCallSocketMgrInitAddr, SockMgr, (void*&)g_pfnSocketManagerConstructor, dummy);

		g_pServerConnect = (void*)FindPattern(SERVERCONNECT_SIG_CSNZ2019, SERVERCONNECT_MASK_CSNZ2019, g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, NULL);
		if (!g_pServerConnect)
			MessageBox(NULL, "g_pServerConnect == NULL!!!", "Error", MB_OK);
		else
			InlineHook(g_pServerConnect, Hook_ServerConnect, (void*&)g_pfnServerConnect);

		auto find = (void*)FindPattern("\x55\x8B\xEC\xB8\x00\x00\x00\x00\x66\xA3", "xxxx????xx", g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, NULL);
		if (!find)
			MessageBox(NULL, "HolePunch__SetServerInfo == NULL!!!", "Error", MB_OK);
		else
			InlineHook(find, Hook_HolePunch__SetServerInfo, (void*&)g_pfnHolePunch__SetServerInfo);

		find = (void*)FindPattern("\x55\x8B\xEC\x83\xEC\x00\x57\x8B\x7D\x00\x85\xFF\x75\x00\x8B\x45", "xxxxx?xxx?xxx?xx", g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, NULL);
		if (!find)
			MessageBox(NULL, "HolePunch__GetUserSocketInfo == NULL!!!", "Error", MB_OK);
		else
			InlineHook(find, Hook_HolePunch__GetUserSocketInfo, (void*&)g_pfnHolePunch__GetUserSocketInfo);

		{
			DWORD pushStr = FindPush(g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, (PCHAR)("resource/zombi/ZombieSkillTable_Dedi.csv"));
			
			// read instruction opcode to know we found valid address
			int opcode = 0;
			ReadMemory((void*)(pushStr + 0xF), (BYTE*)&opcode, 1);

			if (opcode == 0xE8 && pushStr && InlineHookFromCallOpcode((void*)(pushStr + 0xF), CreateStringTable, (void*&)g_pfnCreateStringTable, dummy))
			{
				DWORD parseCsvCallAddr = (DWORD)dummy + 0x71 + 1; // 0x71
				g_pfnParseCSV = (tParseCSV)(parseCsvCallAddr + 4 + *(DWORD*)parseCsvCallAddr);

				// patch LoadZombieSkill function to load csv bypassing filesystem
				DWORD patchAddr = pushStr - 0x1A;
				BYTE patch[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
				WriteMemory((void*)patchAddr, (BYTE*)patch, sizeof(patch));
			}
			else
			{
				MessageBox(NULL, "Failed to patch zombie skill table", "Error", MB_OK);
			}
		}
	}

	//printf("EngineBase: %p\n", g_dwEngineBase);

	auto addr = (void*)FindPattern("\x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\xA1\x00\x00\x00\x00\x33\xC5\x89\x45\x00\x56\x8B\x75\x00\x8D\x45\x00\x50\x6A\x00\xFF\x75\x00\x8D\x85\x00\x00\x00\x00\x68\x00\x00\x00\x00\x50\xE8\x00\x00\x00\x00\x8B\x10\xFF\x70\x00\x83\xCA\x00\x52\xFF\x15\x00\x00\x00\x00\x83\xC4", "xxxxx????x????xxxx?xxx?xx?xx?xx?xx????x????xx????xxxx?xx?xxx????xx", g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, NULL);
	//printf("%p\n", addr);
	if (!addr)
		MessageBox(NULL, "LogToErrorLog == NULL!!!", "Error", MB_OK);
	else
		InlineHook(addr, Hook_LogToErrorLog, (void*&)g_pfnLogToErrorLog);

	g_pEngine = (cl_enginefunc_t*)(PVOID) * (PDWORD)(FindPush(g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, (PCHAR)("ScreenFade")) + 0x0D);
	if (!g_pEngine)
		MessageBox(NULL, "g_pEngine == NULL!!!", "Error", MB_OK);

	// hook Pbuf_AddText to allow any cvar or cmd input from console
	g_pEngine->Pbuf_AddText = Pbuf_AddText;

	g_pPacket_Metadata_Parse = (void*)FindPattern(PACKET_METADATA_PARSE_SIG_CSNZ, PACKET_METADATA_PARSE_MASK_CSNZ, g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, NULL);
	if (g_bDumpMetadata || g_bWriteMetadata)
	{
		if (!g_pPacket_Metadata_Parse)
			MessageBox(NULL, "g_pPacket_Metadata_Parse == NULL!!!", "Error", MB_OK);

		InlineHook(g_pPacket_Metadata_Parse, Packet_Metadata_Parse, (void*&)g_pfnPacket_Metadata_Parse);

		printf("0x%p\n", g_pPacket_Metadata_Parse);
	}

	if (g_bDumpQuest)
	{
		g_pPacket_Quest_Parse = (void*)FindPattern(PACKET_QUEST_PARSE_SIG_CSNZ, PACKET_QUEST_PARSE_MASK_CSNZ, g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, NULL);
		if (!g_pPacket_Quest_Parse)
			MessageBox(NULL, "g_pPacket_Quest_Parse == NULL!!!", "Error", MB_OK);
		else
			InlineHook(g_pPacket_Quest_Parse, Packet_Quest_Parse, (void*&)g_pfnPacket_Quest_Parse);
	}

	if (g_bDumpUMsg)
	{
		g_pPacket_UMsg_Parse = (void*)FindPattern(PACKET_UMSG_PARSE_SIG_CSNZ, PACKET_UMSG_PARSE_MASK_CSNZ, g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, NULL);
		if (!g_pPacket_UMsg_Parse)
			MessageBox(NULL, "g_pPacket_UMsg_Parse == NULL!!!", "Error", MB_OK);

		InlineHook(g_pPacket_UMsg_Parse, Packet_UMsg_Parse, (void*&)g_pfnPacket_UMsg_Parse);

		printf("0x%p\n", g_pPacket_UMsg_Parse);
	}

	if (g_bDumpAlarm)
	{
		g_pPacket_Alarm_Parse = (void*)FindPattern(PACKET_ALARM_PARSE_SIG_CSNZ, PACKET_ALARM_PARSE_MASK_CSNZ, g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, NULL);
		if (!g_pPacket_Alarm_Parse)
			MessageBox(NULL, "g_pPacket_Alarm_Parse == NULL!!!", "Error", MB_OK);

		InlineHook(g_pPacket_Alarm_Parse, Packet_Alarm_Parse, (void*&)g_pfnPacket_Alarm_Parse);

		printf("0x%p\n", g_pPacket_Alarm_Parse);
	}

	if (g_bDumpItem)
	{
		g_pPacket_Item_Parse = (void*)FindPattern(PACKET_ITEM_PARSE_SIG_CSNZ, PACKET_ITEM_PARSE_MASK_CSNZ, g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, NULL);
		if (!g_pPacket_Item_Parse)
			MessageBox(NULL, "g_pPacket_Item_Parse == NULL!!!", "Error", MB_OK);

		InlineHook(g_pPacket_Item_Parse, Packet_Item_Parse, (void*&)g_pfnPacket_Item_Parse);

		printf("0x%p\n", g_pPacket_Item_Parse);
	}

	if (g_bDumpCrypt)
	{
		g_pPacket_Crypt_Parse = (void*)FindPattern(PACKET_CRYPT_SIG, PACKET_CRYPT_MASK, g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, NULL);
		if (!g_pPacket_Crypt_Parse)
			MessageBox(NULL, "g_pPacket_Crypt_Parse == NULL!!!", "Error", MB_OK);

		InlineHook(g_pPacket_Crypt_Parse, Packet_Crypt_Parse, (void*&)g_pfnPacket_Crypt_Parse);

		printf("0x%p\n", g_pPacket_Crypt_Parse);
	}

	if (g_bDumpAll)
	{
		DWORD dwCallAddr = FindPush(g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, (PCHAR)("SocketManager - Initial ReadPacket() failed. (return = %d)\n")) - 0x10;
		if (dwCallAddr == -0x10)
			MessageBox(NULL, "dwCallAddr == 0!!!", "Error", MB_OK);

		InlineHookFromCallOpcode((void*)dwCallAddr, Socket_Read, (void*&)g_pfnSocket_Read, dummy);
	}

	// patch launcher name in hw.dll to fix annoying message box (length of launcher filename must be < original name)
	DWORD strAddr = FindPattern("cstrike-online.exe", strlen("cstrike-online.exe"), g_dwEngineBase, g_dwEngineBase + g_dwEngineSize);
	if (strAddr)
	{
		WriteMemory((void*)strAddr, (BYTE*)"CSOLauncher.exe", strlen("CSOLauncher.exe") + 1);
	}

	if (!g_bUseOriginalServer)
	{
		// hook GetCryptoProtocolName func to make Crypt work
		DWORD dwCallAddr = FindPush(g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, (PCHAR)("CRYPT_ERROR"));
		if (dwCallAddr)
		{
			dwCallAddr -= 0x117;

			InlineHookFromCallOpcode((void*)dwCallAddr, GetCryptoProtocolName, (void*&)g_pfnGetCryptoProtocolName, dummy);
		}

		// hook socket constructor to create ctx objects even if we don't use ssl
		if (!g_bUseSSL)
		{
			dwCallAddr = FindPush(g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, (PCHAR)("new socket()>>"));
			if (dwCallAddr)
			{
				dwCallAddr += 0x62;

				InlineHookFromCallOpcode((void*)dwCallAddr, SocketConstructor, (void*&)g_pfnSocketConstructor, dummy);

				dwCallAddr = FindPush(g_dwEngineBase, g_dwEngineBase + g_dwEngineSize, (PCHAR)("SSL: Failed to load Client's Private Key"), 2);
				if (dwCallAddr)
				{
					dwCallAddr -= 0x1F;

					DWORD dwCreateCtxAddr = dwCallAddr + 1;
					g_pfnEVP_CIPHER_CTX_new = (tEVP_CIPHER_CTX_new)(dwCreateCtxAddr + 4 + *(DWORD*)dwCreateCtxAddr);
				}

				// mega unreliable solution...
				/*DWORD callInitSSLAddr = (DWORD)dummy + 0x142 + 1;
				DWORD initSSLAddr = callInitSSLAddr + 4 + *(DWORD*)callInitSSLAddr;

				DWORD createCtxCallAddr = initSSLAddr + 0x15E + 1;
				g_pfnEVP_CIPHER_CTX_new = (tEVP_CIPHER_CTX_new)(createCtxCallAddr + 4 + *(DWORD*)createCtxCallAddr);*/
			}
		}
	}

	// create thread to wait for other modules
	CreateThread(NULL, 0, HookThread, NULL, 0, 0);
}

void Unhook()
{
	FreeAllHook();
}