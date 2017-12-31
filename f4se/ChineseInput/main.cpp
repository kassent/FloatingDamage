#include "F4SE/PluginAPI.h"
#include "F4SE/GameThreads.h"
#include "F4SE/GameAPI.h"

#include "F4SE_common/F4SE_version.h"
#include "F4SE_common/Relocation.h"
#include "F4SE_common/SafeWrite.h"
#include "F4SE_common/BranchTrampoline.h"
#ifdef _DEBUG
#include "F4SE/GameMenus.h"
#endif
#include "Hooks.h"
#include "InputMenu.h"

#include <memory>
#include <shlobj.h>

IDebugLog	gLog;

PluginHandle				  g_pluginHandle = kPluginHandle_Invalid;

#ifdef _DEBUG
F4SETaskInterface			* g_taskInterface = nullptr;

#endif
F4SEMessagingInterface		* g_messaging = nullptr;



#include "F4SE/GameData.h"
#include "f4se/PapyrusVM.h"
#include "f4se/GameObjects.h"
//VirtualMachine * vm = (*g_gameVM)->m_virtualMachine;
void F4SEMessageHandler(F4SEMessagingInterface::Message* msg)
{

	switch (msg->type)
	{
	case F4SEMessagingInterface::kMessage_GameDataReady:
	{
		if (*g_dataHandler != nullptr)
		{
			for (UInt32 i = 0; i < (*g_dataHandler)->arrDMGT.count; i++)
			{
				BGSDamageType * pDT = (*g_dataHandler)->arrDMGT[i];
				_MESSAGE("+%08X", pDT->formID);
				DumpClass(pDT, sizeof(BGSDamageType) / 8);
				_MESSAGE(" ");
			}

			for (UInt32 i = 0; i < (*g_dataHandler)->arrARMO.count; i++)
			{
				TESObjectARMO * pArmor = (*g_dataHandler)->arrARMO[i];
				//auto p = pArmor->instanceData.valueModifier->count;
				_MESSAGE("+%08X", pArmor->formID);
				_MESSAGE("%d | %d", (pArmor->instanceData.valueModifier) ? pArmor->instanceData.valueModifier->count : 0, (pArmor->instanceData.damageTypes) ? pArmor->instanceData.damageTypes->count: 0);

				_MESSAGE(" ");
			}
		}
	}

	break;
	}

}


void LoadSettings()
{
	constexpr char* configFile = "Data\\F4se\\Plugins\\ChineseInput.ini";
	constexpr char* settingsSection = "Settings";

	Settings::iOffsetX = GetPrivateProfileInt(settingsSection, "iOffsetX", 100, configFile);
	Settings::iOffsetY = GetPrivateProfileInt(settingsSection, "iOffsetY", 50, configFile);
	Settings::iLineHeight = GetPrivateProfileInt(settingsSection, "iLineHeight", 30, configFile);
	Settings::iLineWidth = GetPrivateProfileInt(settingsSection, "iLineWidth", 350, configFile);
	Settings::iFontSize = GetPrivateProfileInt(settingsSection, "iFontSize", 20, configFile);
	Settings::iSeperatorHeight = GetPrivateProfileInt(settingsSection, "iSeperatorHeight", 2, configFile);
	Settings::iBackgroundColor = GetPrivateProfileInt(settingsSection, "iBackgroundColor", 0x00000000, configFile);
	Settings::iCompositionTextColor = GetPrivateProfileInt(settingsSection, "iCompositionTextColor", 0xFFFFFFFF, configFile);
	Settings::iInputStateTextColor = GetPrivateProfileInt(settingsSection, "iInputStateTextColor", 0xFFFFFFFF, configFile);
	Settings::iSeperatorColor = GetPrivateProfileInt(settingsSection, "iSeperatorColor", 0xFFFFFFFF, configFile);
	Settings::iNormalCandidateColor = GetPrivateProfileInt(settingsSection, "iNormalCandidateColor", 0xFFFFFFFF, configFile);
	Settings::iSelectedCandidateColor = GetPrivateProfileInt(settingsSection, "iSelectedCandidateColor", 0xFF0099FF, configFile);
	Settings::iDisalbeMicrosoftWarning = GetPrivateProfileInt(settingsSection, "iDisalbeMicrosoftWarning", 1, configFile);

	std::unique_ptr<char[]> sResult(new char[60]);

	GetPrivateProfileString(settingsSection, "sFontName", "Î¢ÈíÑÅºÚ", sResult.get(), 60, configFile);

	UInt32 unicodeLen = MultiByteToWideChar(CP_UTF8, 0, sResult.get(), -1, nullptr, 0);
	std::unique_ptr<wchar_t[]> sWchar(new wchar_t[unicodeLen + 1]);
	memset(sWchar.get(), 0, (unicodeLen + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, sResult.get(), -1, sWchar.get(), unicodeLen);

	Settings::sFontName = sWchar.get();
}

extern "C"
{
	bool F4SEPlugin_Query(const F4SEInterface * f4se, PluginInfo * info)
	{
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Fallout4\\F4SE\\ChineseInput.log");
		_MESSAGE("ChineseInput: %d.%d.%d", 1, 5, 0);

		// populate info structure
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "ChineseInput";
		info->version = 1;

		// store plugin handle so we can identify ourselves later
		g_pluginHandle = f4se->GetPluginHandle();

		if (f4se->runtimeVersion < RUNTIME_VERSION_1_10_40)
		{
			MessageBox(nullptr, "UNSUPPORTED GAME VERSION.", "ChineseInput", MB_OK);
			return false;
		}

		if (f4se->isEditor)
		{
			_FATALERROR("loaded in editor, marking as incompatible");
			return false;
		}
#ifdef _DEBUG
		g_taskInterface = (F4SETaskInterface *)f4se->QueryInterface(kInterface_Task);
		if (!g_taskInterface)
		{
			_FATALERROR("couldn't get task interface");
			return false;
		}
#endif // DEBUG
		g_messaging = (F4SEMessagingInterface *)f4se->QueryInterface(kInterface_Messaging);
		if (!g_messaging)
		{
			_FATALERROR("couldn't get messaging interface");
			return false;
		}




		return true;
	}


	bool F4SEPlugin_Load(const F4SEInterface * f4se)
	{
		//if (g_messaging != nullptr)
		//	g_messaging->RegisterListener(g_pluginHandle, "F4SE", F4SEMessageHandler);

		if (!g_branchTrampoline.Create(1024 * 64))
		{
			_ERROR("couldn't create branch trampoline. this is fatal. skipping remainder of init process.");
			return false;
		}

		if (!g_localTrampoline.Create(1024 * 64, nullptr))
		{
			_ERROR("couldn't create codegen buffer. this is fatal. skipping remainder of init process.");
			return false;
		}
		LoadSettings();

		InitHooks();

		return true;
	}

};