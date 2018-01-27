#include "common/ICriticalSection.h"

#include "F4SE_common/F4SE_version.h"
#include "f4se_common/BranchTrampoline.h"
#include "f4se/PluginAPI.h"
#include "f4se/NiObjects.h"
#include "f4se/NiNodes.h"
#include "f4se/GameReferences.h"
#include "f4se/GameMenus.h"
#include "f4se/ScaleformCallbacks.h"
#include "f4se/GameEvents.h"
#include "f4se/GameThreads.h"

#include <shlobj.h>
#include <memory>
#include <string>
#include <random>
#include <queue>

#include "HookUtil.h"
#include "UI.h"


#include <thread>

#define PLUGIN_VERSION	MAKE_EXE_VERSION(1, 2, 0)
#define PLUGIN_NAME		"FloatingDamage"

IDebugLog						gLog;
PluginHandle					g_pluginHandle = kPluginHandle_Invalid;
F4SEScaleformInterface			* g_scaleform = nullptr;
F4SEMessagingInterface			* g_messaging = nullptr;

#define GetPrivateProfileFloat(settingName, defalutValue)	\
		GetPrivateProfileStringA(settingsSection, #settingName, #defalutValue, sResult.get(), MAX_PATH, configFile);\
		settingName = std::stof(sResult.get());

struct Settings
{
	static	bool			bShowEffectDamage;
	static	bool			bUseDefaultHUDColor;
	static	bool			bApplyShaderQuad;

	static	UInt32			iWidgetColorR;
	static	UInt32			iWidgetColorG;
	static	UInt32			iWidgetColorB;
	static	UInt32			iWidgetScale;
	static	UInt32			iWidgetOpacity;

	static	float			fMinHorizontalSpeed;
	static	float			fMaxHorizontalSpeed;
	static	float			fMinVerticalRisingSpeed;
	static	float			fMaxVerticalRisingSpeed;
	static	float			fMaxVerticalRisingDist;
	static	float			fMinVerticalRisingDist;
	static	float			fMaxVerticalFallDist;
	static	float			fMinVerticalFallDist;
	static	float			fGravitationalConstant;
	static	float			fEffectDamageRisingSpeed;

	static void LoadSettings()
	{
		constexpr char * configFile = ".\\Data\\MCM\\Settings\\FloatingDamage.ini";
		constexpr char * settingsSection = "Settings";

		bShowEffectDamage = GetPrivateProfileIntA(settingsSection, "bShowEffectDamage", 1, configFile) != 0;
		bUseDefaultHUDColor = GetPrivateProfileIntA(settingsSection, "bUseDefaultHUDColor", 1, configFile) != 0;
		bApplyShaderQuad = GetPrivateProfileIntA(settingsSection, "bApplyShaderQuad", 1, configFile) != 0;

		iWidgetColorR = GetPrivateProfileIntA(settingsSection, "iWidgetColorR", 0xFF, configFile);
		iWidgetColorG = GetPrivateProfileIntA(settingsSection, "iWidgetColorG", 0xFF, configFile);
		iWidgetColorB = GetPrivateProfileIntA(settingsSection, "iWidgetColorB", 0xFF, configFile);
		iWidgetScale = GetPrivateProfileIntA(settingsSection, "iWidgetScale", 100, configFile);
		iWidgetOpacity = GetPrivateProfileIntA(settingsSection, "iWidgetOpacity", 100, configFile);

		std::unique_ptr<char[]> sResult(new char[MAX_PATH]);
		GetPrivateProfileFloat(fMinHorizontalSpeed, 2.5);
		GetPrivateProfileFloat(fMaxHorizontalSpeed, 4.0);
		GetPrivateProfileFloat(fMinVerticalRisingSpeed, 1.5);
		GetPrivateProfileFloat(fMaxVerticalRisingSpeed, 4.5);
		GetPrivateProfileFloat(fMinVerticalRisingDist, 90);
		GetPrivateProfileFloat(fMaxVerticalRisingDist, 105);
		GetPrivateProfileFloat(fMinVerticalFallDist, 80);
		GetPrivateProfileFloat(fMaxVerticalFallDist, 120);
		GetPrivateProfileFloat(fGravitationalConstant, 0.08);
		GetPrivateProfileFloat(fEffectDamageRisingSpeed, 1.5);
	}
};

bool	Settings::bShowEffectDamage = false;
bool	Settings::bUseDefaultHUDColor = true;
bool	Settings::bApplyShaderQuad = true;

UInt32	Settings::iWidgetColorR = 0xFF;
UInt32	Settings::iWidgetColorG = 0xFF;
UInt32	Settings::iWidgetColorB = 0xFF;
UInt32	Settings::iWidgetScale = 100;
UInt32	Settings::iWidgetOpacity = 100;

float	Settings::fMinHorizontalSpeed = 2.5f;
float	Settings::fMaxHorizontalSpeed = 4.0f;
float	Settings::fMinVerticalRisingSpeed = 1.5f;
float	Settings::fMaxVerticalRisingSpeed = 4.5f;
float	Settings::fGravitationalConstant = 0.08f;
float	Settings::fEffectDamageRisingSpeed = 1.5f;
float	Settings::fMaxVerticalRisingDist = 105.0f;
float	Settings::fMinVerticalRisingDist = 90.0f;
float	Settings::fMaxVerticalFallDist = 120.0f;
float	Settings::fMinVerticalFallDist = 80.0f;

class BGSAttackData;

struct DamageFrame
{
	NiPoint3					hitLocation;					// 00
	UInt32						pad0C;							// 0C
	float						unk10[8];						// 10
	bhkNPCollisionObject		* collisionObj;					// 30
	UInt64						unk38;							// 38
	UInt32						attackerHandle;					// 40
	UInt32						victimHandle;					// 44
	UInt64						unk48[(0x50 - 0x48) >> 3];		// 48
	BGSAttackData				* attackData;					// 50 occur when use melee weapon.
	TESForm						* damageSourceForm;				// 58
	TBO_InstanceData			* instanceData;					// 60
	UInt64						unk68[(0x80 - 0x68) >> 3];		// 68
	TESAmmo						* ammo;							// 80 occur when use gun.
	void						* unk88;						// 88
	float						damage2;						// 90 game uses this value to calc final damage.
	float						unk94;							// 94
	float						damage;							// 98
};
STATIC_ASSERT(sizeof(DamageFrame) == 0xA0);

using _Process = void(*)(void *, DamageFrame *);
RelocAddr<_Process>	ProcessDamageFrame = 0xE01090; //48 8B C4 48 89 50 10 55 56 41 56 41 57

using _GetActorValueHolder = ActorValueInfo **(*)();
RelocAddr<_GetActorValueHolder> GetActorValueHolder = 0x006B1F0; //E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? E8 ? ? ? ?


namespace UIFramework
{
	class FloatingDamageDelegate;
	class FloatingDamageMenu;

	ICriticalSection							s_uiQueueLock;
	std::queue<FloatingDamageDelegate*>			s_uiQueue;

	class FloatingDamageDelegate
	{
	public:
		FloatingDamageDelegate(UInt32 dmg, float x, float y, float z, bool isEffect) : damage(dmg), hitPoint(x, y, z), isDebuff(isEffect) {}

		virtual ~FloatingDamageDelegate() {}

		virtual void Run(IMenu * pMenu)
		{
			NiPoint3 screenPoint{};
			WorldToScreen_Internal(&hitPoint, &screenPoint);
			screenPoint.y = 1.0f - screenPoint.y;//from bottom to top.

			GFxMovieRoot * movieRoot = pMenu->movie->movieRoot;
			GFxValue params[4];
			params[0].SetUInt(damage);

			movieRoot->CreateArray(&params[1]);
			GFxValue x(screenPoint.x);
			params[1].PushBack(&x);
			GFxValue y(screenPoint.y);
			params[1].PushBack(&y);
			GFxValue z(screenPoint.z);
			params[1].PushBack(&z);

			movieRoot->CreateArray(&params[2]);
			x.SetNumber(hitPoint.x);
			params[2].PushBack(&x);
			y.SetNumber(hitPoint.y);
			params[2].PushBack(&y);
			z.SetNumber(hitPoint.z);
			params[2].PushBack(&z);

			params[3].SetBool(isDebuff);
			pMenu->stage.Invoke("onDamageReceived", nullptr, params, 4);
		}

		static void Register(float dmg, NiPoint3 & location, bool isEffect)
		{
			s_uiQueueLock.Enter();
			if (s_uiQueue.size() <= 0x64)
			{
				auto * pDelegate = new FloatingDamageDelegate(static_cast<UInt32>(dmg), location.x, location.y, location.z, isEffect);
				s_uiQueue.push(pDelegate);
			}
			s_uiQueueLock.Leave();
		}

	private:
		UInt32					damage;
		NiPoint3				hitPoint;
		bool					isDebuff;
	};

	class FloatingDamageMenu : public GameMenuBase
	{
	public:

		enum
		{
			kMessage_UpdateModSettings = 0x10,
			kMessage_UpdateColorSettings
		};

		FloatingDamageMenu() : GameMenuBase()
		{
			//default menu depth is 6.
			flags = kFlag_DoNotPreventGameSave | kFlag_DisableInteractive | kFlag_Unk800000;
			if ((*g_scaleformManager)->LoadMovie(this, this->movie, "FloatingDamageMenu", "root1.Menu_mc", 2))
			{
				if (Settings::bApplyShaderQuad)
					flags |= kFlag_ApplyDropDownFilter;
				else
					stage.SetMember("showShadowEffect", &GFxValue(true));
				CreateBaseShaderTarget(this->shaderTarget, this->stage);
				if (Settings::bUseDefaultHUDColor)
				{
					SetFilterColorType(this->shaderTarget, 2, 1.0f);
				}
				else
				{
					this->shaderTarget->colorType = BSGFxShaderFXTarget::kColorNoChange;
					FilterColor color; // RRGGBB
					color.r = float(Settings::iWidgetColorR & 0xFF) / 255.0f;
					color.g = float(Settings::iWidgetColorG & 0xFF) / 255.0f;
					color.b = float(Settings::iWidgetColorB & 0xFF) / 255.0f;
					ApplyColorFilter(this->shaderTarget, &color, 1.0f);
				}
				if (this->flags & kFlag_ApplyDropDownFilter)
				{
					this->subcomponents.Push(this->shaderTarget);
				}
			}
		}

		virtual void *	ReleaseThis(bool releaseMem) final
		{
			return this->GameMenuBase::ReleaseThis(releaseMem);
		};

		virtual void	Invoke(Args * args) final 
		{
			switch (args->optionID)
			{
			case 0:
			{
				NiPoint3 worldPos{};
				worldPos.x = static_cast<float>(args->args[0].GetNumber());
				worldPos.y = static_cast<float>(args->args[1].GetNumber());
				worldPos.z = static_cast<float>(args->args[2].GetNumber());
				NiPoint3 screenPos;
				WorldToScreen_Internal(&worldPos, &screenPos);
				screenPos.y = 1.0f - screenPos.y;//from bottom to top.
				args->movie->movieRoot->CreateArray(args->result);
				GFxValue x(screenPos.x);
				args->result->PushBack(&x);
				GFxValue y(screenPos.y);
				args->result->PushBack(&y);
				GFxValue z(screenPos.z);
				args->result->PushBack(&z);
				break;
			}
			case 1:
			{
				if (g_ui && (*g_ui) != nullptr)
					args->result->SetBool((*g_ui)->numPauseGame);
				else
					args->result->SetBool(false);
				break;
			}
			case 2:
			{
				auto * movieRoot = args->movie->movieRoot;
				auto * settings = args->result;
				movieRoot->CreateObject(settings);
				Register<double>(settings, "fWidgetScale", static_cast<float>(Settings::iWidgetScale) / 100);
				Register<double>(settings, "fWidgetOpacity", static_cast<float>(Settings::iWidgetOpacity) / 100);
				Register<double>(settings, "fMinHorizontalSpeed", Settings::fMinHorizontalSpeed);
				Register<double>(settings, "fMaxHorizontalSpeed", Settings::fMaxHorizontalSpeed);
				Register<double>(settings, "fMinVerticalRisingSpeed", Settings::fMinVerticalRisingSpeed);
				Register<double>(settings, "fMaxVerticalRisingSpeed", Settings::fMaxVerticalRisingSpeed);
				Register<double>(settings, "fGravitationalConstant", Settings::fGravitationalConstant);
				Register<double>(settings, "fEffectDamageRisingSpeed", Settings::fEffectDamageRisingSpeed);
				Register<double>(settings, "fMaxVerticalRisingDist", Settings::fMaxVerticalRisingDist);
				Register<double>(settings, "fMinVerticalRisingDist", Settings::fMinVerticalRisingDist);
				Register<double>(settings, "fMaxVerticalFallDist", Settings::fMaxVerticalFallDist);
				Register<double>(settings, "fMinVerticalFallDist", Settings::fMinVerticalFallDist);
				break;
			}
			default:
				break;
			}
		}

		virtual void	RegisterFunctions() final 
		{
			this->RegisterFunction("WorldtoScreen", 0);
			this->RegisterFunction("IsInMenuMode", 1);
			this->RegisterFunction("GetModSettings", 2);
		}

		virtual UInt32	ProcessMessage(UIMessage * msg) final 
		{ 
			switch (msg->type)
			{
			case kMessage_Open:
			case kMessage_Close:
			{
				s_uiQueueLock.Enter();
				while (!s_uiQueue.empty())
				{
					FloatingDamageDelegate * cmd = s_uiQueue.front();
					s_uiQueue.pop();
					delete cmd;
				}
				s_uiQueueLock.Leave();
				break;
			}
			case kMessage_UpdateModSettings:
			{
				GFxMovieRoot * movieRoot = movie->movieRoot;
				GFxValue settings;
				movieRoot->CreateObject(&settings);
				Register<double>(&settings, "fWidgetScale", static_cast<float>(Settings::iWidgetScale) / 100);
				Register<double>(&settings, "fWidgetOpacity", static_cast<float>(Settings::iWidgetOpacity) / 100);
				Register<double>(&settings, "fMinHorizontalSpeed", Settings::fMinHorizontalSpeed);
				Register<double>(&settings, "fMaxHorizontalSpeed", Settings::fMaxHorizontalSpeed);
				Register<double>(&settings, "fMinVerticalRisingSpeed", Settings::fMinVerticalRisingSpeed);
				Register<double>(&settings, "fMaxVerticalRisingSpeed", Settings::fMaxVerticalRisingSpeed);
				Register<double>(&settings, "fGravitationalConstant", Settings::fGravitationalConstant);
				Register<double>(&settings, "fEffectDamageRisingSpeed", Settings::fEffectDamageRisingSpeed);
				Register<double>(&settings, "fMaxVerticalRisingDist", Settings::fMaxVerticalRisingDist);
				Register<double>(&settings, "fMinVerticalRisingDist", Settings::fMinVerticalRisingDist);
				Register<double>(&settings, "fMaxVerticalFallDist", Settings::fMaxVerticalFallDist);
				Register<double>(&settings, "fMinVerticalFallDist", Settings::fMinVerticalFallDist);
				stage.Invoke("onModSettingChanged", nullptr, &settings, 1);
				break;
			}
			case kMessage_UpdateColorSettings:
			{
				if (Settings::bUseDefaultHUDColor)
					this->shaderTarget->SetFilterColor(false);
				else
				{
					this->shaderTarget->colorType = BSGFxShaderFXTarget::kColorNoChange;
					FilterColor color; // RRGGBB
					color.r = float(Settings::iWidgetColorR & 0xFF) / 255.0f;
					color.g = float(Settings::iWidgetColorG & 0xFF) / 255.0f;
					color.b = float(Settings::iWidgetColorB & 0xFF) / 255.0f;
					ApplyColorFilter(this->shaderTarget, &color, 1.0f);
				}
				break;
			}
			default:
				break;
			}
			return this->GameMenuBase::ProcessMessage(msg); 
		};

		virtual void	DrawNextFrame(float unk0, void * unk1) final
		{
			if (stage.IsDisplayObject())
			{
				s_uiQueueLock.Enter();
				while (!s_uiQueue.empty())
				{
					FloatingDamageDelegate * cmd = s_uiQueue.front();
					s_uiQueue.pop();
					cmd->Run(this);
					delete cmd;
				}
				s_uiQueueLock.Leave();
			}
			return this->GameMenuBase::DrawNextFrame(unk0, unk1);
		}; 

		static IMenu * CreateFloatingDamageMenu()
		{
			void * ptr = (*g_scaleformHeap)->Allocate(sizeof(FloatingDamageMenu));
			return (ptr != nullptr) ? new (ptr) FloatingDamageMenu() : nullptr;
		}

		static void UpdateModSettings()
		{
			static BSFixedString menuName("FloatingDamageMenu");
			(*g_uiMessageManager)->SendUIMessage(menuName, kMessage_UpdateModSettings);
		}

		static void UpdateColorSettings()
		{
			static BSFixedString menuName("FloatingDamageMenu");
			(*g_uiMessageManager)->SendUIMessage(menuName, kMessage_UpdateColorSettings);
		}

		static void OpenMenu()
		{
			static BSFixedString menuName("FloatingDamageMenu");
			(*g_uiMessageManager)->SendUIMessage(menuName, kMessage_Open);
		}

		static void CloseMenu()
		{
			static BSFixedString menuName("FloatingDamageMenu");
			(*g_uiMessageManager)->SendUIMessage(menuName, kMessage_Close);
		}

		static void RegisterMenu()
		{
			static BSFixedString menuName("FloatingDamageMenu");
			if ((*g_ui) != nullptr && !(*g_ui)->IsMenuRegistered(menuName))
			{
				(*g_ui)->RegisterMenu("FloatingDamageMenu", CreateFloatingDamageMenu, 0);
			}
		}
	};


	class MenuOpenCloseHandler : public BSTEventSink<MenuOpenCloseEvent>
	{
	public:
		virtual ~MenuOpenCloseHandler() { };
		virtual	EventResult	ReceiveEvent(MenuOpenCloseEvent * evn, void * dispatcher) override
		{
			static BSFixedString HUDMenu("HUDMenu");
			static BSFixedString faderMenu("FaderMenu");
			static BSFixedString floatingDamageMenu("FloatingDamageMenu");
			if (evn->menuName == HUDMenu)
			{
				if (evn->isOpen)
					FloatingDamageMenu::OpenMenu();
				else
					FloatingDamageMenu::CloseMenu();
			}
			if (!evn->isOpen && evn->menuName == faderMenu && (*g_ui) != nullptr\
				&& (*g_ui)->IsMenuOpen(HUDMenu) && !(*g_ui)->IsMenuOpen(floatingDamageMenu))
			{
				FloatingDamageMenu::OpenMenu();
			}
			//_MESSAGE("%s=%d", evn->menuName.c_str(), evn->isOpen);
			return kEvent_Continue;
		};

		static void Register()
		{
			if ((*g_ui) != nullptr)
			{
				static auto * pHandler = new MenuOpenCloseHandler();
				RegisterMenuOpenCloseEvent((*g_ui)->menuOpenCloseEventSource, pHandler);
			}
		}
	};


	void InitData()
	{
		// global
		RELOC_MEMBER_FN(BSFixedString, ctor, "E8 ? ? ? ? 49 8B CF 48 8B D0 E8 ? ? ? ? 48 8D 8D ? ? ? ? 8B D8", 0, 1, 5);
		RELOC_MEMBER_FN(BSFixedString, Set, "E8 ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? 83 FB 02", 0, 1, 5);
		RELOC_MEMBER_FN(BSFixedString, Release_Imp, "E8 ? ? ? ? 48 8B 0E E8 ? ? ? ? 48 C7 06 ? ? ? ?", 0, 1, 5);

		RELOC_MEMBER_FN(Heap, Allocate, "E8 ? ? ? ? 48 85 C0 74 08 48 8B C8 E8 ? ? ? ? 49 8B 0E", 0, 1, 5);
		RELOC_MEMBER_FN(Heap, Free, "E8 ? ? ? ? EB 1E 41 8D 46 FF 48 8B D3 83 E0 01", 0, 1, 5);

		RELOC_MEMBER_FN(GFxValue::ObjectInterface, SetMember, "25 ? ? ? ? 4C 8D 4D F0 4C 8D 05 ? ? ? ? 3C 0A 0F 94 C0", 0x19, 1, 5);
		RELOC_MEMBER_FN(GFxValue::ObjectInterface, ReleaseManaged_Internal, "E8 ? ? ? ? 48 8D 15 ? ? ? ? 45 33 C0 49 8B CE 41 C6 86 ? ? ? ? ?", 0, 1, 5);
		RELOC_MEMBER_FN(GFxValue::ObjectInterface, Invoke, "E8 ? ? ? ? 49 8B CC E8 ? ? ? ? 49 8B 54 24 ?", 0, 1, 5);
		RELOC_MEMBER_FN(GFxValue::ObjectInterface, AddManaged_Internal, "8B 42 08 25 ? ? ? ? 83 F8 06 74 2E");
		RELOC_MEMBER_FN(GFxValue::ObjectInterface, PushBack, "40 53 48 83 EC 40 48 8B 41 08 4D 8B C8");

		RELOC_GLOBAL_VAL(CalculateCRC32_64, "E8 ? ? ? ? 4C 8B 83 ? ? ? ? 4D 85 C0 74 51 8B 53 0C 8B 4C 24 50", 0, 1, 5);
		RELOC_GLOBAL_VAL(WorldToScreen_Internal, "E8 ? ? ? ? F3 0F 10 17 F3 0F 10 6F ?", 0, 1, 5);
		RELOC_GLOBAL_VAL(ProcessDamageFrame, "48 8B C4 48 89 50 10 55 56 41 56 41 57");
		RELOC_GLOBAL_VAL(GetActorValueHolder, "E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? E8 ? ? ? ?", 0, 1, 5);
		RELOC_GLOBAL_VAL(LookupREFRByHandle, "E8 ? ? ? ? 48 8B 5D 67 48 85 DB 0F 84 ? ? ? ?", 0, 1, 5);

		RELOC_GLOBAL_VAL(g_player, "48 8B 05 ? ? ? ? 48 85 C0 74 0C F0 FF 40 28", 0, 3, 7);
		RELOC_GLOBAL_VAL(g_mainHeap, "48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 55 AF", 0, 3, 7);
		RELOC_MEMBER_FN_EX(PlayerCharacter, HasDetectionLOS, "41 C6 00 00 48 85 C9");

		// ui framework
		RELOC_MEMBER_FN_EX(UIMessageManager, SendUIMessage, "E8 ? ? ? ? 48 8B 0D ? ? ? ? 89 1C 2E", 0, 1, 5);
		RELOC_MEMBER_FN_EX(UIMessageManager, SendUIMessageEx, "E8 ? ? ? ? 48 8B 4C 24 ? 48 85 C9 74 0A 48 8B 01 BA ? ? ? ? FF 10 48 8B 6C 24 ? 48 8B 7C 24 ?", 0, 1, 5);
		RELOC_MEMBER_FN_EX(BSScaleformManager, LoadMovie, "48 8B C4 4C 89 40 18 48 89 48 08 55 56 57 41 54 41 57");
		RELOC_MEMBER_FN_EX(SWFToCodeFunctionHandler, RegisterFunction, "40 53 48 83 EC 50 48 8B DA 48 8B D1 48 8B 0D ? ? ? ? 48 85 C9");

		RELOC_MEMBER_FN_EX(UI, IsMenuOpen, "E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 48 8B 3D ? ? ? ?", 0, 1, 5);
		RELOC_MEMBER_FN_EX(UI, RegisterMenu, "40 53 56 57 41 56 41 57 48 83 EC 50 44 8B 15 ? ? ? ?");
		RELOC_MEMBER_FN_EX(UI, ProcessMessage, "E8 ? ? ? ? B1 01 E8 ? ? ? ? 48 8B 0D ? ? ? ? 33 D2 C6 05 ? ? ? ? ? C6 05 ? ? ? ? ?", 0, 1, 5);

		RELOC_MEMBER_FN_EX(GameMenuBase, DrawNextFrame_Imp, "40 53 48 83 EC 20 48 83 79 ? ? 48 8B D9 74 1E");
		RELOC_MEMBER_FN_EX(GameMenuBase, ProcessMessage_Imp, "40 57 48 83 EC 20 44 8B 42 10");
		RELOC_MEMBER_FN_EX(GameMenuBase, InitializeThis, "E8 ? ? ? ? 33 FF 48 8D 05 ? ? ? ? 48 89 06 48 8D 05 ? ? ? ?", 0, 1, 5);
		RELOC_MEMBER_FN_EX(GameMenuBase, ReleaseParent, "48 89 5C 24 ? 57 48 83 EC 20 48 8D 05 ? ? ? ? 48 8B D9 48 89 01 48 8D 05 ? ? ? ? 48 89 41 10 80 B9 ? ? ? ? ?");
		uintptr_t * location = reinterpret_cast<uintptr_t*>(RELOC_RUNTIME_ADDR("48 8D 05 ? ? ? ? 49 83 CD FF", 0, 3, 7));
		RELOC_MEMBER_FN_EX(GameMenuBase, Unk07_Imp, { { plugin_info.runtime_version, location[7] - RelocationManager::s_baseAddr } });
		RELOC_MEMBER_FN_EX(GameMenuBase, Unk08_Imp, { { plugin_info.runtime_version, location[8] - RelocationManager::s_baseAddr } });
		RELOC_MEMBER_FN_EX(GameMenuBase, Unk09_Imp, { { plugin_info.runtime_version, location[9] - RelocationManager::s_baseAddr } });
		RELOC_MEMBER_FN_EX(GameMenuBase, Unk0A_Imp, { { plugin_info.runtime_version, location[0xA] - RelocationManager::s_baseAddr } });
		RELOC_MEMBER_FN_EX(GameMenuBase, Unk0B_Imp, { { plugin_info.runtime_version, location[0xB] - RelocationManager::s_baseAddr } });
		RELOC_MEMBER_FN_EX(GameMenuBase, Unk0C_Imp, { { plugin_info.runtime_version, location[0xC] - RelocationManager::s_baseAddr } });
		RELOC_MEMBER_FN_EX(GameMenuBase, Unk0D_Imp, { { plugin_info.runtime_version, location[0xD] - RelocationManager::s_baseAddr } });
		RELOC_MEMBER_FN_EX(GameMenuBase, Unk10_Imp, { { plugin_info.runtime_version, location[0x10] - RelocationManager::s_baseAddr } });
		RELOC_MEMBER_FN_EX(GameMenuBase, Unk11_Imp, { { plugin_info.runtime_version, location[0x11] - RelocationManager::s_baseAddr } });
		RELOC_MEMBER_FN_EX(GameMenuBase, Unk12_Imp, { { plugin_info.runtime_version, location[0x12] - RelocationManager::s_baseAddr } });
		RELOC_MEMBER_FN_EX(GameMenuBase, Unk13_Imp, { { plugin_info.runtime_version, location[0x13] - RelocationManager::s_baseAddr } });

		RELOC_GLOBAL_VAL(RegisterMenuOpenCloseEvent, "E8 ? ? ? ? E9 ? ? ? ? 4D 85 E4 0F 84 ? ? ? ? 4C 8D 0D ? ? ? ?", 0, 1, 5);
		RELOC_GLOBAL_VAL(UnregisterMenuOpenCloseEvent, "E8 ? ? ? ? 80 BE ? ? ? ? ? 74 07 C6 86 ? ? ? ? ? 4C 8D B6 ? ? ? ?", 0, 1, 5);
		RELOC_GLOBAL_VAL(CreateBaseShaderTarget, "E8 ? ? ? ? 49 8B 9E ? ? ? ? 4C 8D 05 ? ? ? ?", 0, 1, 5);
		RELOC_GLOBAL_VAL(SetFilterColorType, "40 53 48 83 EC 40 89 91 ? ? ? ?");
		RELOC_GLOBAL_VAL(ApplyColorFilter, "E8 ? ? ? ? 48 8B 5C 24 ? F6 47 3C 01", 0, 1, 5);
		RELOC_GLOBAL_VAL(GetFilterColorByType, "33 C0 48 89 02 89 42 08 48 63 81 ? ? ? ?");

		RELOC_GLOBAL_VAL(g_ui, "48 8B 3D ? ? ? ? E8 ? ? ? ? 48 8B CF 48 8B D0 E8 ? ? ? ? 84 C0 0F 85 ? ? ? ?", 0, 3, 7);
		RELOC_GLOBAL_VAL(g_scaleformHeap, "48 8B 0D ? ? ? ? 48 8B 01 FF 50 60 48 83 EF 28 48 FF CE 75 BA 48 8B 55 58", 0, 3, 7);
		RELOC_GLOBAL_VAL(g_scaleformManager, "48 8B 0D ? ? ? ? 48 8B 49 18 E8 ? ? ? ? 48 8B 44 24 ?", 0, 3, 7);
		RELOC_GLOBAL_VAL(g_uiMessageManager, "48 8B 3D ? ? ? ? E8 ? ? ? ? 41 B8 ? ? ? ? 48 8B CF", 0, 3, 7);

		RELOC_GLOBAL_VAL(globalMenuStackLock, "48 8D 0D ? ? ? ? 4C 89 74 24 ? 41 8B FC", 0, 3, 7);
		RELOC_GLOBAL_VAL(globalMenuTableLock, "48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 96 ? ? ? ? 4C 8B 7C 24 ?", 0, 3, 7);
	}
}

class FloatingDamage_OnModSettingChanged : public GFxFunctionHandler
{
public:
	virtual void Invoke(Args * args) override
	{
		Settings::LoadSettings();
		UIFramework::FloatingDamageMenu::UpdateModSettings();
	}
};

class FloatingDamage_OnColorSettingChanged : public GFxFunctionHandler
{
public:
	virtual void Invoke(Args * args) override
	{
		Settings::LoadSettings();
		UIFramework::FloatingDamageMenu::UpdateColorSettings();
	}
};

SimpleLock			globalDamageLock;
class ActorEx : public Actor
{
public:
	using _DamageHealth = void(__thiscall ActorEx::*)(Actor *, float);
	static _DamageHealth	DamageHealth;

	void DamageHealth_Hook(Actor * attacker, float damage)
	{
		if (!Settings::bShowEffectDamage)
		{
			return (this->*DamageHealth)(attacker, damage);
		}
		float damageReceived = abs(damage);
		if (damageReceived < 1.0f)
		{
			static std::random_device random_device;
			static std::mt19937 generator(random_device());
			static std::uniform_real_distribution<double> distribution(0.0f, 1.0f);
			damageReceived = (distribution(generator) < damageReceived) ? 1.0f : damageReceived;
		}
		damageReceived = std::round(damageReceived);
		if (damageReceived >= 1.0f && this->formID != 0x14 && (*g_player)->HasLOS(this) && !this->IsDead(true))
		{
			globalDamageLock.Lock();
			if (globalDamageLock.lockCount == 1)
			{
				NiPoint3 hitPoint{}, screenPoint{};
				this->GetMarkerPosition(&hitPoint);
				hitPoint.z += 27;
				NiNode * pRootNode = this->GetObjectRootNode();
				if (pRootNode != nullptr)
				{
					hitPoint.x = pRootNode->m_worldTransform.pos.x;
					hitPoint.y = pRootNode->m_worldTransform.pos.y;
				}
				UIFramework::FloatingDamageDelegate::Register(damageReceived, hitPoint, true);
			}
			globalDamageLock.Release();
		}
		return (this->*DamageHealth)(attacker, damage);
	}

	static void ProcessDamageFrame_Hook(Actor * pObj, DamageFrame * pDamageFrame)
	{

		TESObjectREFR * pRef = nullptr;

		if (pDamageFrame != nullptr && (LookupREFRByHandle(&pDamageFrame->victimHandle, &pRef), pRef != nullptr) \
			&& pRef->formType == FormType::kFormType_ACHR && pRef->formID != 0x14 && (*g_player)->HasLOS(pRef) && !pRef->IsDead(true))
		{
			globalDamageLock.Lock();
			float healthA = 0.0f, healthB = 0.0f;
			ActorValueInfo*	pHealth = *reinterpret_cast<ActorValueInfo**>(GetActorValueHolder() + 0x1B);
			healthA = pRef->actorValueOwner.GetValue(pHealth);
			ProcessDamageFrame(pObj, pDamageFrame);
			healthB = pRef->actorValueOwner.GetValue(pHealth);
			float damageReceived = std::round(healthA - healthB);
			if (damageReceived > 0.0f)
			{
				NiPoint3 hitPoint{}, screenPoint{};
				auto & hitLocation = pDamageFrame->hitLocation;
				if (hitLocation.x || hitLocation.y || hitLocation.z)
				{
					hitPoint.x = hitLocation.x;
					hitPoint.y = hitLocation.y;
					hitPoint.z = hitLocation.z;
				}
				else
				{
					pRef->GetMarkerPosition(&hitPoint);
					hitPoint.z -= 5;
					NiNode * pRootNode = pRef->GetObjectRootNode();
					if (pRootNode != nullptr)
					{
						hitPoint.x = pRootNode->m_worldTransform.pos.x;
						hitPoint.y = pRootNode->m_worldTransform.pos.y;
					}
				}
				UIFramework::FloatingDamageDelegate::Register(damageReceived, hitPoint, false);
			}
			globalDamageLock.Release();
		}
		else
		{
			ProcessDamageFrame(pObj, pDamageFrame);
		}
		if (pRef != nullptr)
		{
			pRef->handleRefObject.DecRefHandle();
		}
	}

	static void InitHooks()
	{
		UIFramework::InitData();

		g_branchTrampoline.Write5Call(RELOC_RUNTIME_ADDR("E8 ? ? ? ? 48 85 FF 74 36 48 8B CF"), (uintptr_t)ProcessDamageFrame_Hook);
		DamageHealth = HookUtil::SafeWrite64(RELOC_RUNTIME_ADDR("41 56 41 57 48 83 EC 20 80 B9 ? ? ? ? ? 48 8D 05 ? ? ? ? 48 8B D9", 0xF, 3, 7) + 0x880, &DamageHealth_Hook);
	}

};
ActorEx::_DamageHealth	ActorEx::DamageHealth = nullptr;



void MessageCallback(F4SEMessagingInterface::Message* msg)
{
	if (msg->type == F4SEMessagingInterface::kMessage_GameLoaded)
	{
		UIFramework::FloatingDamageMenu::RegisterMenu();
		UIFramework::MenuOpenCloseHandler::Register();
	}
}


bool ScaleformCallback(GFxMovieView * view, GFxValue * value)
{
	RegisterFunction<FloatingDamage_OnModSettingChanged>(value, view->movieRoot, "onModSettingChanged");
	RegisterFunction<FloatingDamage_OnColorSettingChanged>(value, view->movieRoot, "onColorSettingChanged");
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


		plugin_info.plugin_name = PLUGIN_NAME;
		plugin_info.runtime_version = f4se->runtimeVersion;

		//if (f4se->runtimeVersion != RUNTIME_VERSION_1_10_50)
		//{
		//	MessageBoxA(nullptr, "UNSUPPORTED GAME VERSION. REQUIRED VERSION IS: V1.10.50", PLUGIN_NAME, MB_OK);
		//	return false;
		//}

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
		if (!g_branchTrampoline.Create(1024 * 64))
		{
			_FATALERROR("couldn't create branch trampoline. this is fatal. skipping remainder of init process.");
			return false;
		}
		Settings::LoadSettings();

		try
		{
			sig_scan_timer timer;
			ActorEx::InitHooks();
		}
		catch (const no_result_exception & exception)
		{
			_MESSAGE(exception.what());
			MessageBoxA(nullptr, "Signature scan failed, please deactive FloatingDamage and download newer version from Nexus.", "FloatingDamage", MB_OK);
			return false;
		}

		if (g_scaleform)
			g_scaleform->Register("FloatingDamage", ScaleformCallback);

		if (g_messaging != nullptr)
			g_messaging->RegisterListener(g_pluginHandle, "F4SE", MessageCallback);
		return true;
	}
};




