#include "F4SE/PluginAPI.h"
#include "F4SE/GameMenus.h"

#include "F4SE_common/F4SE_version.h"
#include "F4SE_common/Relocation.h"
#include "F4SE_common/SafeWrite.h"

#include <shlobj.h>
#include <memory>
#include <string>

#define PLUGIN_VERSION	MAKE_EXE_VERSION(1, 2, 0)
#define PLUGIN_NAME		"ClockWidget"

IDebugLog						gLog;
PluginHandle					g_pluginHandle = kPluginHandle_Invalid;
F4SEScaleformInterface			* g_scaleform = nullptr;

#ifdef _DEBUG
class ScaleformWidget_WriteLog : public GFxFunctionHandler
{
public:
	virtual void Invoke(Args * args)
	{
		ASSERT(args->numArgs >= 1);
		ASSERT(args->args[0].GetType() == GFxValue::kType_String);
		_MESSAGE(args->args[0].GetString());
	}
};


#endif

bool bEnalbe24HourClock = false;

class ScaleformWidget_GetSettings : public GFxFunctionHandler
{
public:
	virtual void Invoke(Args * args)
	{
		ASSERT(args->numArgs >= 1);
		ASSERT(args->args[0].GetType() == GFxValue::kType_String);
		args->result->SetBool(bEnalbe24HourClock);
	}
};
//GetSettings
#define InjectFlashFile(a)																				\
	GFxValue loader;																					\
	movieRoot->CreateObject(&loader, "flash.display.Loader");											\
	GFxValue loadArgs[2];																				\
	movieRoot->CreateObject(&loadArgs[0], "flash.net.URLRequest", &GFxValue("ClockWidget.swf"), 1);		\
	loadArgs[1].SetNull();																				\
	movieRoot->Invoke((a), nullptr, &loader, 1);														\
	if (!loader.Invoke("load", nullptr, loadArgs, 2)){													\
		_MESSAGE("Failed to inject clock widget...");													\
	}


bool ScaleformCallback(GFxMovieView * view, GFxValue * value)
{
#ifdef _DEBUG
	RegisterFunction<ScaleformWidget_WriteLog>(value, view->movieRoot, "log");
#endif
	RegisterFunction<ScaleformWidget_GetSettings>(value, view->movieRoot, "GetSettings");
	GFxMovieRoot * movieRoot = view->movieRoot;
	if (movieRoot)
	{
		GFxValue loaderInfo;
		if (movieRoot->GetVariable(&loaderInfo, "root.loaderInfo.url"))
		{
			std::string sResult = loaderInfo.GetString();
			//_MESSAGE("%s", sResult.c_str());
			if (sResult.find("LoadingMenu.swf") != std::string::npos)
			{
				InjectFlashFile("root.FilterHolder_mc.Menu_mc.addChild");
			}
			else if (sResult.find("FaderMenu.swf") != std::string::npos)
			{
				InjectFlashFile("root.Menu_mc.SpinnerIcon_mc.addChild");
			}
			else if (sResult.find("MainMenu.swf") != std::string::npos)
			{
				InjectFlashFile("root.Menu_mc.Spinner_mc.addChild");
			}
		}
	}
	return true;
}

extern "C"
{
	bool F4SEPlugin_Query(const F4SEInterface * f4se, PluginInfo * info)
	{
		std::unique_ptr<char[]> sPath(new char[MAX_PATH]);
		sprintf_s(sPath.get(), MAX_PATH, "%s%s.log", "\\My Games\\Fallout4\\F4SE\\", PLUGIN_NAME);
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, sPath.get());

		_MESSAGE("%s: %08X", PLUGIN_NAME, PLUGIN_VERSION);

		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = PLUGIN_NAME;
		info->version = PLUGIN_VERSION;

		g_pluginHandle = f4se->GetPluginHandle();

		if (f4se->runtimeVersion != RUNTIME_VERSION_1_10_50)
		{
			MessageBox(nullptr, "UNSUPPORTED GAME VERSION.THE REQUIRED VERSION IS: V1.10.40", PLUGIN_NAME, MB_OK);
			return false;
		}

		if (f4se->isEditor)
		{
			_FATALERROR("loaded in editor, marking as incompatible");
			return false;
		}

		g_scaleform = (F4SEScaleformInterface *)f4se->QueryInterface(kInterface_Scaleform);
		if (!g_scaleform)
		{
			_FATALERROR("couldn't get scaleform interface");
			return false;
		}
		return true;
	}

	bool F4SEPlugin_Load(const F4SEInterface * f4se)
	{
		constexpr char* configFile = ".\\Data\\F4SE\\Plugins\\ClockWidget.ini";
		constexpr char* settingsSection = "Settings";

		if (GetPrivateProfileInt(settingsSection, "iEnable24HourClock", 0, configFile))
		{
			bEnalbe24HourClock = true;
		}

		if (g_scaleform)
			g_scaleform->Register("ClockWidget", ScaleformCallback);

		return true;
	}
};