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

#define PLUGIN_VERSION	MAKE_EXE_VERSION(1, 0, 3)
#define PLUGIN_NAME		"FloatingDamage"

IDebugLog						gLog;
PluginHandle					g_pluginHandle = kPluginHandle_Invalid;
F4SEScaleformInterface			* g_scaleform = nullptr;
F4SEMessagingInterface			* g_messaging = nullptr;

ICriticalSection					s_uiQueueLock;
std::queue<ITaskDelegate*>			s_uiQueue;

class FloatingDamageDelegate : public ITaskDelegate
{
public:
	FloatingDamageDelegate(UInt32 dmg, float x, float y, float z, bool isEffect) : damage(dmg), hitPoint(x, y, z), isDebuff(isEffect) {}

	virtual ~FloatingDamageDelegate() {}

	virtual void Run()
	{
		NiPoint3 screenPoint{};
		WorldToScreen_Internal(&hitPoint, &screenPoint);
		screenPoint.y = 1.0f - screenPoint.y;//from bottom to top.
		HUDMenu * pHUD = nullptr;
		static BSFixedString menuName("HUDMenu");
		if ((*g_ui) != nullptr && (pHUD = static_cast<HUDMenu*>((*g_ui)->GetMenu(&menuName)), pHUD))
		{
			GFxMovieRoot * movieRoot = pHUD->movie->movieRoot;
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
			movieRoot->Invoke("root.floatingDamageLoader.content.onDamageReceived", nullptr, params, 4);
		}
	}

	static void Register(float dmg, NiPoint3 & location, bool isEffect)
	{
		s_uiQueueLock.Enter();
		auto * pDelegate = new FloatingDamageDelegate(static_cast<UInt32>(dmg), location.x, location.y, location.z, isEffect);
		s_uiQueue.push(pDelegate);
		s_uiQueueLock.Leave();
	}

private:
	UInt32					damage;
	NiPoint3				hitPoint;
	bool					isDebuff;
};




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

struct Settings
{
	static	UInt32			iWidgetColor;
	static	UInt32			iWidgetScale;
	static	UInt32			iWidgetOpacity;
	static	bool			bCanFollowObj;
	static	bool			bShowEffectDamage;
};
UInt32	Settings::iWidgetColor = 0xFFFFFF;
UInt32	Settings::iWidgetScale = 100;
UInt32	Settings::iWidgetOpacity = 100;
bool	Settings::bCanFollowObj = true;
bool	Settings::bShowEffectDamage = false;

//A4F720
//using _GetTopLevelContext = HUDContextArray<BSFixedString> * (*)();
//RelocAddr<_GetTopLevelContext> GetTopLevelContext(0xA4F720);
//
//using _RegisterFloatingQuestMarker = void(*)(HUDMenu *, const char *);
//RelocAddr<_RegisterFloatingQuestMarker> RegisterFloatingQuestMarker(0x127F080);

typedef void(*_UpdateHUDComponents)(HUDMenu * menu);
RelocAddr<_UpdateHUDComponents> UpdateHUDComponents(0x01278A70);
//48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B F1 48 8B 0D ? ? ? ? E8 ? ? ? ?

class HUDFloatingQuestMarkersEx : public HUDComponentBase
{
public:
	class ApplyColorUpdateHandler : public BSTEventSink<ApplyColorUpdateEvent>
	{
	public:
		virtual	EventResult	ReceiveEvent(ApplyColorUpdateEvent * evn, void * dispatcher) override
		{
			if (evn != nullptr)
			{
				componentThreadLock.Lock();
				if (pWidgetContainer != nullptr)
				{
					if (Settings::iWidgetColor != -1)
					{
						FilterColor color; // RRGGBB
						color.r = float((Settings::iWidgetColor >> 16) & 0xFF) / 255.0f;
						color.g = float((Settings::iWidgetColor >> 8) & 0xFF) / 255.0f;
						color.b = float(Settings::iWidgetColor & 0xFF) / 255.0f;
						ApplyColorFilter(pWidgetContainer, &color, 1.0f);
					}
					else
					{
						pWidgetContainer->SetFilterColor(false);//crash here.
					}
				}
				componentThreadLock.Release();
			}
			return kEvent_Continue;
		}

		static void Register()
		{
			if (g_colorUpdateDispatcher.GetPtr() && (*g_colorUpdateDispatcher))
			{
				static auto * pHandler = new ApplyColorUpdateHandler();
				(*g_colorUpdateDispatcher)->dispatcher.AddEventSink(pHandler);
			}
		}
	};

	static BSGFxShaderFXTarget		* pWidgetContainer;
	static SimpleLock				componentThreadLock;

	using _ReleaseThis = HUDFloatingQuestMarkersEx*(__thiscall HUDFloatingQuestMarkersEx::*)(bool releaseMem);
	static _ReleaseThis				ReleaseThis;

	using _UpdateWidget = void(__thiscall HUDFloatingQuestMarkersEx::*)();
	static _UpdateWidget			UpdateWidget;

	HUDFloatingQuestMarkersEx * ReleaseThis_Hook(bool releaseMem)
	{
		componentThreadLock.Lock();
		if (pWidgetContainer != nullptr)
		{
			delete pWidgetContainer;
			pWidgetContainer = nullptr;
		}
		componentThreadLock.Release();
		return (this->*ReleaseThis)(releaseMem);
	}

	void UpdateWidget_Hook()
	{
		s_uiQueueLock.Enter();
		while (!s_uiQueue.empty())
		{
			ITaskDelegate * cmd = s_uiQueue.front();
			s_uiQueue.pop();
			cmd->Run();
			delete cmd;
		}
		s_uiQueueLock.Leave();
		return (this->*UpdateWidget)();
	}

	static void RegisterComponent(GFxValue * pComponent)
	{
		componentThreadLock.Lock();
		if (pComponent->IsDisplayObject() && !pWidgetContainer)
		{
			pWidgetContainer = new BSGFxShaderFXTarget(pComponent);
			if (Settings::iWidgetColor != -1)
			{
				FilterColor color; // RRGGBB
				color.r = float((Settings::iWidgetColor >> 16) & 0xFF) / 255.0f;
				color.g = float((Settings::iWidgetColor >> 8) & 0xFF) / 255.0f;
				color.b = float(Settings::iWidgetColor & 0xFF) / 255.0f;
				ApplyColorFilter(pWidgetContainer, &color, 1.0f);
			}
			else
			{
				pWidgetContainer->SetFilterColor(false);
			}
		}
		componentThreadLock.Release();
	}

	static void InitHooks()
	{
		ReleaseThis = HookUtil::SafeWrite64(RelocAddr<uintptr_t>(0x02D3ECB8), &ReleaseThis_Hook);
		UpdateWidget = HookUtil::SafeWrite64(RelocAddr<uintptr_t>(0x02D3ECB8) + 4 * 0x8, &UpdateWidget_Hook);
	}
};
SimpleLock									HUDFloatingQuestMarkersEx::componentThreadLock;
BSGFxShaderFXTarget	*						HUDFloatingQuestMarkersEx::pWidgetContainer = nullptr;
HUDFloatingQuestMarkersEx::_ReleaseThis		HUDFloatingQuestMarkersEx::ReleaseThis = nullptr;
HUDFloatingQuestMarkersEx::_UpdateWidget	HUDFloatingQuestMarkersEx::UpdateWidget = nullptr;


SimpleLock			globalComponentLock;
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
			globalComponentLock.Lock();
			if (globalComponentLock.lockCount == 1)
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
				FloatingDamageDelegate::Register(damageReceived, hitPoint, true);
			}
			globalComponentLock.Release();
		}
		return (this->*DamageHealth)(attacker, damage);
	}
	static void InitHooks()
	{
		DamageHealth = HookUtil::SafeWrite64(RelocAddr<uintptr_t>(0x02D68398) + 0x880, &DamageHealth_Hook);
	}
};
ActorEx::_DamageHealth	ActorEx::DamageHealth = nullptr;

void ProcessDamageFrame(Actor * pObj, DamageFrame * pDamageFrame)
{
	//_MESSAGE(">>>> %s", pObj->GetReferenceName());
	//DumpClass(pDamageFrame, 0x100 >> 3);

	using _Process = void(*)(void *, DamageFrame *);
	RelocAddr<_Process>	Process = 0xE01090; //48 8B C4 48 89 50 10 55 56 41 56 41 57

	using _GetActorValueHolder = ActorValueInfo **(*)();
	RelocAddr<_GetActorValueHolder> GetActorValueHolder = 0x006B1F0; //E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? E8 ? ? ? ?

	TESObjectREFR * pRef = nullptr;

	if (pDamageFrame != nullptr && (LookupREFRByHandle(&pDamageFrame->victimHandle, &pRef), pRef != nullptr) \
		&& pRef->formType == FormType::kFormType_ACHR && pRef->formID != 0x14 && (*g_player)->HasLOS(pRef) && !pRef->IsDead(true))
	{
		globalComponentLock.Lock();
		float healthA = 0.0f, healthB = 0.0f;
		ActorValueInfo*	pHealth = *reinterpret_cast<ActorValueInfo**>(GetActorValueHolder() + 0x1B);
		healthA = pRef->actorValueOwner.GetValue(pHealth);
		Process(pObj, pDamageFrame);
		healthB = pRef->actorValueOwner.GetValue(pHealth);
		float damageReceived = std::round(healthA - healthB);
		if (damageReceived != 0.0f)
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
			FloatingDamageDelegate::Register(damageReceived, hitPoint, false);
		}
		globalComponentLock.Release();
	}
	else
	{
		Process(pObj, pDamageFrame);
	}
	if (pRef != nullptr)
	{
		pRef->handleRefObject.DecRefHandle();
	}
}

class FloatingDamage_WorldtoScreen : public GFxFunctionHandler
{
public:
	virtual void Invoke(Args * args) override
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
	}
};

class FloatingDamage_IsInMenuMode : public GFxFunctionHandler
{
public:
	virtual void Invoke(Args * args) override
	{
		if (g_ui && (*g_ui) != nullptr)
			args->result->SetBool((*g_ui)->menuMode);
		else
			args->result->SetBool(false);
	}
};

class FloatingDamage_GetModSettings : public GFxFunctionHandler
{
public:
	virtual void Invoke(Args * args) override
	{
		auto * movieRoot = args->movie->movieRoot;
		auto * settings = args->result;
		movieRoot->CreateObject(settings);
		GFxValue	scale;
		scale.SetNumber(static_cast<float>(Settings::iWidgetScale) / 100);
		settings->SetMember("scale", &scale);
		GFxValue	opacity;
		opacity.SetNumber(static_cast<float>(Settings::iWidgetOpacity) / 100);
		settings->SetMember("opacity", &opacity);
		GFxValue	canFollow;
		canFollow.SetBool(Settings::bCanFollowObj);
		settings->SetMember("canFollow", &canFollow);
	}
};

class FloatingDamage_RegisterComponent : public GFxFunctionHandler
{
public:
	virtual void Invoke(Args * args) override
	{
		HUDFloatingQuestMarkersEx::RegisterComponent(args->args);
	}
};


void MessageCallback(F4SEMessagingInterface::Message* msg)
{
	if (msg->type == F4SEMessagingInterface::kMessage_GameDataReady)
	{
		HUDFloatingQuestMarkersEx::ApplyColorUpdateHandler::Register();
	}
}


bool ScaleformCallback(GFxMovieView * view, GFxValue * value)
{
	GFxMovieRoot * movieRoot = view->movieRoot;
	if (movieRoot)
	{
		GFxValue result;
		GFxValue stage;
		movieRoot->GetVariable(&stage, "stage");
		GFxValue firstChild;
		stage.Invoke("getChildAt", &firstChild, &GFxValue((SInt32)0), 1);
		movieRoot->Invoke("flash.utils.getQualifiedClassName", &result, &firstChild, 1);
		if (result.IsString())
		{
			const char * clipName = result.GetString();
			if (strcmp("HUDMenu", clipName) == 0)
			{
				//GFxValue root;
				//movieRoot->GetVariable(&root, "root");
				RegisterFunction<FloatingDamage_WorldtoScreen>(value, view->movieRoot, "WorldtoScreen");
				RegisterFunction<FloatingDamage_IsInMenuMode>(value, view->movieRoot, "IsInMenuMode");
				RegisterFunction<FloatingDamage_RegisterComponent>(value, view->movieRoot, "RegisterComponent");
				RegisterFunction<FloatingDamage_GetModSettings>(value, view->movieRoot, "GetModSettings");

				GFxValue loader;
				movieRoot->CreateObject(&loader, "flash.display.Loader");
				firstChild.SetMember("floatingDamageLoader", &loader);

				GFxValue loadArgs[2];
				movieRoot->CreateObject(&loadArgs[0], "flash.net.URLRequest", &GFxValue("FloatingDamage.swf"), 1);
				loadArgs[1].SetNull();
				//movieRoot->Invoke("root.addChild", nullptr, &loader, 1);

				firstChild.Invoke("addChild", nullptr, &loader, 1);

				if (!loader.Invoke("load", nullptr, loadArgs, 2))
				{
					_MESSAGE("%s >> failed to inject flash widget...", __FUNCTION__);
				}
			}
		}
	}
	return true;
}
void LoadSettings()
{
	constexpr char* configFile = "Data\\F4se\\Plugins\\FloatingDamage.ini";
	constexpr char* settingsSection = "Settings";

	Settings::iWidgetColor = GetPrivateProfileIntA(settingsSection, "iWidgetColor", 0xFFFFFF, configFile);
	Settings::iWidgetScale = GetPrivateProfileIntA(settingsSection, "iWidgetScale", 100, configFile);
	Settings::iWidgetOpacity = GetPrivateProfileIntA(settingsSection, "iWidgetOpacity", 100, configFile);
	Settings::bCanFollowObj = GetPrivateProfileIntA(settingsSection, "bCanFollowObj", 1, configFile) != 0;
	Settings::bShowEffectDamage = GetPrivateProfileIntA(settingsSection, "bShowEffectDamage", 1, configFile) != 0;
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
			MessageBoxA(nullptr, "UNSUPPORTED GAME VERSION. REQUIRED VERSION IS: V1.10.50", PLUGIN_NAME, MB_OK);
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
			_ERROR("couldn't create branch trampoline. this is fatal. skipping remainder of init process.");
			return false;
		}
		LoadSettings();

		g_branchTrampoline.Write5Call(RelocAddr<uintptr_t>(0x0D607C1), (uintptr_t)ProcessDamageFrame); //E8 ? ? ? ? 48 85 FF 74 36 48 8B CF
		//g_branchTrampoline.Write5Call(RelocAddr<uintptr_t>(0x12786F1), (uintptr_t)RegisterFloatingQuestMarker_Hook);//E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 8D 97 ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ?
		HUDFloatingQuestMarkersEx::InitHooks();
		ActorEx::InitHooks();

		if (g_scaleform)
			g_scaleform->Register("FloatingDamage", ScaleformCallback);

		if (g_messaging != nullptr)
			g_messaging->RegisterListener(g_pluginHandle, "F4SE", MessageCallback);

		return true;
	}
};




