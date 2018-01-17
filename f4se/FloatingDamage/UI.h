#pragma once
#include "f4se_common/Utilities.h"
#include "f4se_common/Relocation.h"

#include "f4se/GameTypes.h"
#include "f4se/GameUtilities.h"
#include "f4se/ScaleformCallbacks.h"
#include "f4se/ScaleformValue.h"
#include "f4se/ScaleformLoader.h"
#include "f4se/GameInput.h"

#include <string>
#include <vector>

template<typename T>
inline void Register(GFxValue * dst, const char * name, T value)
{

}

template<>
inline void Register(GFxValue * dst, const char * name, SInt32 value)
{
	GFxValue	fxValue;
	fxValue.SetInt(value);
	dst->SetMember(name, &fxValue);
}

template<>
inline void Register(GFxValue * dst, const char * name, UInt32 value)
{
	GFxValue	fxValue;
	fxValue.SetUInt(value);
	dst->SetMember(name, &fxValue);
}

template<>
inline void Register(GFxValue * dst, const char * name, double value)
{
	GFxValue	fxValue;
	fxValue.SetNumber(value);
	dst->SetMember(name, &fxValue);
}

template<>
inline void Register(GFxValue * dst, const char * name, bool value)
{
	GFxValue	fxValue;
	fxValue.SetBool(value);
	dst->SetMember(name, &fxValue);
}

inline void RegisterString(GFxValue * dst, GFxMovieRoot * view, const char * name, const char * str)
{
	GFxValue	fxValue;
	view->CreateString(&fxValue, str);
	dst->SetMember(name, &fxValue);
}

namespace UIFramework
{
	using _PlayUISound = void(*)(const char *);
	extern RelocAddr<_PlayUISound>	PlayUISound;

	//This function create base BSGFxShaderFXTarget and set filter color to 1, and register color update event.
	using _CreateBaseShaderTarget = void(*)(BSGFxShaderFXTarget * & component, GFxValue & stage);
	extern RelocAddr<_CreateBaseShaderTarget>	CreateBaseShaderTarget;

	using _SetFilterColorType = void(*)(BSGFxShaderFXTarget * component, UInt32 type, float unk);//unk,read float data form RelocAddr<float*>(0x2C71DA0),default value is 1.0f.//F3 0F 10 15 ? ? ? ? BA ? ? ? ? 48 8B CB E8 ? ? ? ? 48 8D BB ? ? ? ?.
	extern RelocAddr<_SetFilterColorType>	SetFilterColorType;

	using _GetChildElement = GFxValue* (*)(GFxValue * parent, GFxValue & child, const char * path);
	extern RelocAddr<_GetChildElement>	GetChildElement;


	class IMenu;
	class ScaleformArgs;
	class DelegateArgs;

	enum MessageType
	{
		kMessage_Refresh = 0,
		kMessage_Open,
		kMessage_Close = 3,
		kMessage_Scaleform = 5,//keydown/up
		kMessage_Message
	};

	class UIMessage
	{
	public:
		virtual ~UIMessage();
		///void						** vtbl; 
		BSFixedString				name;		// 08
		UInt32						type;		// 10
	};

	class UIMessageManager
	{
	public:
		DEFINE_MEMBER_FUNCTION(SendUIMessage, void, 0x204C580, BSFixedString& menuName, UInt32 type);//48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 54 41 56 41 57 48 83 EC 20 44 8B 0D ? ? ? ? 65 48 8B 04 25 ? ? ? ? 48 8B E9 4A 8B 34 C8
		DEFINE_MEMBER_FUNCTION(SendUIMessageEx, void, 0x12BA470, BSFixedString& menuName, UInt32 type, UIMessage * pExtraData);//seems it's a template function,different menu has different ex function.
		//E8 ? ? ? ? 48 8B 4C 24 ? 48 85 C9 74 0A 48 8B 01 BA ? ? ? ? FF 10 48 8B 6C 24 ? 48 8B 7C 24 ?
	};
	extern RelocPtr<UIMessageManager*>	g_uiMessageManager;


	class BSScaleformManager
	{
	public:
		virtual ~BSScaleformManager();
		virtual void Unk_01(void); // Init image loader?

		UInt64					unk08;			// 08 - 0
		GFxStateBag				* stateBag;		// 10
		void					* unk18;		// 18
		void					* unk20;		// 20
		BSScaleformImageLoader	* imageLoader;	// 28

		DEFINE_MEMBER_FUNCTION(LoadMovie, bool, 0x21104C0, IMenu * menu, GFxMovieView *&, const char * name, const char * stagePath, UInt32 flags); //48 8B C4 4C 89 40 18 48 89 48 08 55 56 57 41 54 41 57
	};
	extern RelocPtr <BSScaleformManager *> g_scaleformManager;

	class DelegateArgs
	{
	public:
		DelegateArgs(IMenu * menu, ScaleformArgs * args);
		std::string				menuName;
		UInt32					optionID;
		std::vector<GFxValue>	parameters;
	};

	class ScaleformArgs
	{
	public:

		ScaleformArgs(IMenu * menu, const DelegateArgs & delegateArgs);

		GFxValue		* result;	// 00
		GFxMovieView	* movie;	// 08
		GFxValue		* thisObj;	// 10
		GFxValue		* unk18;	// 18
		const GFxValue	* args;		// 20
		UInt32			numArgs;	// 28
		UInt32			pad2C;		// 2C
		UInt32			optionID;	// 30 pUserData
	};
	STATIC_ASSERT(offsetof(ScaleformArgs, optionID) == 0x30);

	class GFxFunctionHandler
	{
	public:
		using Args = ScaleformArgs;

		GFxFunctionHandler() : refCount(1) {};
		virtual void *	ReleaseThis(bool releaseMem) = 0;
		virtual void	Invoke(Args * args) = 0; //redefine GFxValue !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

												 //	void	** _vtbl;			// 00
		volatile	SInt32	refCount;	// 08
		UInt32		pad0C;				// 0C
	};

	class SWFToCodeFunctionHandler : public GFxFunctionHandler
	{
	public:
		virtual void	RegisterFunctions() = 0;	// 02

		DEFINE_MEMBER_FUNCTION(RegisterFunction, void, 0x210FD80, const char * name, UInt32 index); //40 53 48 83 EC 50 48 8B DA 48 8B D1 48 8B 0D ? ? ? ? 48 85 C9
	};

	class IMenu : public SWFToCodeFunctionHandler, //02EDDBD8
		public BSInputEventUser
	{
	public:
		enum
		{
			//Confirmed
			kFlag_PauseGame = 0x01,
			kFlag_DoNotDeleteOnClose = 0x02,
			kFlag_ShowCursor = 0x04,
			kFlag_EnableMenuControl = 0x08, // 1, 2
			kFlag_ShaderdWorld = 0x20,
			kFlag_Open = 0x40,//set it after open.
			kFlag_DoNotPreventGameSave = 0x800,
			kFlag_ApplyDropDownFilter = 0x8000, //
			kFlag_BlurBackground = 0x400000,

			//Unconfirmed
			kFlag_Modal = 0x10,
			kFlag_PreventGameLoad = 0x80,
			kFlag_Unk0100 = 0x100,
			kFlag_HideOther = 0x200,
			kFlag_DisableInteractive = 0x4000,
			kFlag_Unk0400 = 0x400,
			kFlag_Unk1000 = 0x1000,
			kFlag_ItemMenu = 0x2000,
			kFlag_Unk10000 = 0x10000,	// mouse cursor
			kFlag_Unk800000 = 0x800000
		};
		virtual UInt32	ProcessMessage(UIMessage * msg) = 0;//???
		virtual void	DrawNextFrame(float unk0, void * unk1) = 0; //210E8C0
		virtual void *	Unk_05(void) { return nullptr; }; //return 0;
		virtual void *	Unk_06(void) { return nullptr; }; //return 0;
		virtual bool	Unk_07(UInt32 unk0, void * unk1) = 0;
		virtual void	Unk_08(UInt8 unk0) = 0;
		virtual void	Unk_09(BSFixedString & menuName, bool unk1) = 0;            //UInt64 = 0;            //UInt64
		virtual void	Unk_0A(void) = 0;
		virtual void	Unk_0B(void) = 0;
		virtual void	Unk_0C(void) = 0;
		virtual bool	Unk_0D(bool unk0) = 0;
		virtual bool	Unk_0E(void) { return false; };
		virtual bool	CanProcessControl(BSFixedString & controlID) { return false; };
		virtual bool	Unk_10(void) = 0;
		virtual void	Unk_11(void) = 0;
		virtual void	Unk_12(void * unk0) = 0;

		GFxValue		stage;			// 20
		GFxMovieView	* movie;		// 40
		BSFixedString	unk48;			// 48
		BSFixedString	menuName;		// 50
		UInt32			flags;			// 58

										/*
																A A A A A A A A B B B B B B B B C C C C C C C C D D D D D D D D
										LoadingMenu				0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 1 0 1 0 1 1 0 0 0 0 0 1		depth: 000E		context: 0003
										Console					0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 1 1 0 0 0 1 1 1		depth: 0013		context: 0006
										LevelUpMenu				0 0 0 0 0 1 0 0 0 0 0 0 1 1 0 1 0 0 0 0 0 1 0 0 1 1 0 0 0 1 1 1		depth: 0009		context: 0022
										FaderMenu				0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1 0		depth: 0006		context: 0022
										CursorMenu				0 0 0 0 0 0 1 0 1 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 1 0 0 0 0 0 0		depth: 0014		context: 0022
										VignetteMenu			0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 1 0 0 0 0 0 0		depth: 0003		context: 0022
										MessageBoxMenu			0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 1 0 0 1 1 0 1 1 1 0 1		depth: 000A		context: 0022
										ContainerMenu			0 0 0 0 1 0 0 0 0 0 0 0 1 1 0 1 1 0 1 0 0 1 0 1 0 1 0 0 1 1 0 1		depth: 0006		context: 0022
										ExamineMenu				0 0 0 0 1 0 0 0 0 0 0 0 1 1 0 0 1 0 1 0 0 1 0 1 0 1 0 0 0 1 0 1		depth: 0009		context: 0022
										CookingMenu				0 0 0 0 1 0 0 0 0 0 0 0 1 1 0 0 1 0 1 0 0 1 0 1 0 1 0 0 0 1 0 0		depth: 0009		context: 0022
										ExamineConfirmMenu		0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 1 0 0 0 1 0 1 1 1 0 1		depth: 0011		context: 0022
										RobotModMenu			0 0 0 0 1 0 0 0 0 0 0 0 1 1 0 0 1 0 1 0 0 1 0 1 0 1 0 0 0 1 0 0		depth: 0009		context: 0022
										PowerArmorModMenu		0 0 0 0 1 0 0 0 0 0 0 0 1 1 0 0 1 0 1 0 0 1 0 1 0 1 0 0 0 1 0 0		depth: 0009		context: 0022
										WorkshopMenu			0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 1 0 0 0 0 0 0 1 0 1 0 0 0 0 0 0		depth: 0006		context: 0010
										PromptMenu				0 0 0 0 0 0 0 0 1 0 0 0 0 1 0 0 1 1 0 0 1 0 0 0 0 1 0 0 0 0 0 0		depth: 0005		context: 0022
										SitWaitMenu				0 0 0 0 0 0 0 0 1 0 0 0 1 1 0 0 1 1 0 0 1 0 0 0 0 1 0 0 0 0 0 0		depth: 0006		context: 0012
										SleepWaitMenu			0 0 0 0 1 0 0 0 0 1 0 0 1 1 0 1 1 0 0 0 1 0 0 1 1 1 0 0 1 1 0 1		depth: 000A		context: 0022
										DialogueMenu			0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 1 0 0 0 0 1 0 0 0 0 0 0		depth: 0006		context: 0022
										BarterMenu				0 0 0 0 1 0 0 0 0 0 0 0 1 1 0 1 1 0 1 0 0 1 0 1 0 1 0 0 1 1 0 1		depth: 0006		context: 0022
										LockpickingMenu			0 0 0 0 0 0 0 0 0 1 0 0 1 1 0 0 1 0 0 0 0 0 0 0 0 1 1 0 0 0 0 1		depth: 0006		context: 000C
										BookMenu				0 0 0 0 1 0 0 0 0 1 1 0 1 1 0 0 1 0 0 0 0 0 0 1 0 1 1 0 1 0 0 1		depth: 0009		context: 0008
										SPECIALMenu				0 0 0 0 1 0 0 0 0 1 0 0 1 1 0 1 1 0 0 0 0 1 0 0 1 1 1 0 1 1 0 1		depth: 0006		context: 0022
										FavoritesMenu			0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 1 1 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0		depth: 0006		context: 0001
										HUDMenu					0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 1 1 0 0 1 0 0 0 0 1 0 0 0 0 0 0		depth: 0005		context: 0022
										PowerArmorHUDMenu		0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 1 1 0 0 1 0 0 0 0 1 0 0 0 0 0 0		depth: 0005		context: 0022
										PauseMenu				0 0 0 0 1 0 0 0 0 1 0 0 1 1 0 0 1 0 0 0 1 1 1 0 0 1 0 1 1 1 0 1		depth: 000B		context: 0022
										VATSMenu				0 0 0 0 0 0 0 0 1 0 0 0 1 1 0 1 1 0 0 0 0 1 0 0 0 1 0 0 0 1 0 0		depth: 0006		context: 000D
										PipboyMenu				0 0 0 0 0 0 0 0 1 0 1 0 1 1 0 0 1 0 1 0 0 0 0 1 0 1 0 0 0 1 0 1		depth: 0008		context: 0022
										PipboyHolotapeMenu		0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 0 0 0 1 0 0 1 0 0 1		depth: 0009		context: 0022
										*/
		UInt32			unk5C;			// 5C
		UInt32			unk60;			// 60	init'd as DWord then Byte
		UInt8			depth;			// 64   defalut is 6.
		UInt32			context;		// 68	init'd in IMenu::IMenu
		UInt32			pad6C;			// 6C
	};
	STATIC_ASSERT(offsetof(IMenu, movie) == 0x40);
	STATIC_ASSERT(offsetof(IMenu, flags) == 0x58);

	// E0
	class GameMenuBase : public IMenu
	{
	public:
		GameMenuBase() { this->InitializeThis(); }
		virtual void *	ReleaseThis(bool releaseMem) override
		{
			this->ReleaseParent();
			if (releaseMem)
				(*g_scaleformHeap)->Free(this);
			return this;
		};
		virtual void	Invoke(Args * args) override { }
		virtual void	RegisterFunctions() override { } //210FAA0
		virtual UInt32	ProcessMessage(UIMessage * msg) override { return this->ProcessMessage_Imp(msg); };//???
		virtual void	DrawNextFrame(float unk0, void * unk1) override { return this->DrawNextFrame_Imp(unk0, unk1); }; //render,HUD menu uses this function to update its HUD components.
		virtual bool	Unk_07(UInt32 unk0, void * unk1) override { return this->Unk07_Imp(unk0, unk1); };
		virtual void	Unk_08(UInt8 unk0) override { return this->Unk08_Imp(unk0); };
		virtual void	Unk_09(BSFixedString & menuName, bool unk1)  override { return this->Unk09_Imp(menuName, unk1); };            //UInt64
		virtual void	Unk_0A(void) override { return this->Unk0A_Imp(); };
		virtual void	Unk_0B(void) override { return this->Unk0B_Imp(); }
		virtual void	Unk_0C(void) override { return this->Unk0C_Imp(); };
		virtual bool	Unk_0D(bool unk0) override { return this->Unk0D_Imp(unk0); }
		virtual bool	Unk_0E(void) override { return false; };
		virtual bool	CanProcessControl(BSFixedString & controlID) override { return false; };
		virtual bool	Unk_10(void) override { return this->Unk10_Imp(); } //90 - E0
		virtual void	Unk_11(void) override { return this->Unk11_Imp(); };
		virtual void	Unk_12(void * unk0) override { return this->Unk12_Imp(unk0); }
		virtual void	Unk_13(void * unk0, void * unk1) { return this->Unk13_Imp(unk0, unk1); }

		tArray<BSGFxDisplayObject*>		subcomponents;					// 70
		BSGFxShaderFXTarget				* shaderTarget;					// 88
		void							* unk90;						// 90
		UInt64							unk98[(0xE0 - 0x98) >> 3];		// 98

	private:
		DEFINE_MEMBER_FUNCTION(DrawNextFrame_Imp, void, 0x210E8C0, float unk0, void * unk1); //40 53 48 83 EC 20 48 83 79 ? ? 48 8B D9 74 1E
		DEFINE_MEMBER_FUNCTION(ProcessMessage_Imp, UInt32, 0x210E840, UIMessage * msg);//
		DEFINE_MEMBER_FUNCTION(Unk07_Imp, bool, 0x210ED00, UInt32 unk0, void * unk1);
		DEFINE_MEMBER_FUNCTION(Unk08_Imp, void, 0xB328D0, UInt8 unk0);
		DEFINE_MEMBER_FUNCTION(Unk09_Imp, void, 0x210EF40, BSFixedString & menuName, bool unk1);
		DEFINE_MEMBER_FUNCTION(Unk0A_Imp, void, 0xB32940);
		DEFINE_MEMBER_FUNCTION(Unk0B_Imp, void, 0xB32A00);
		DEFINE_MEMBER_FUNCTION(Unk0C_Imp, void, 0xB32A40);
		DEFINE_MEMBER_FUNCTION(Unk0D_Imp, bool, 0x210F090, bool unk0);
		DEFINE_MEMBER_FUNCTION(Unk10_Imp, bool, 0xB326F0);
		DEFINE_MEMBER_FUNCTION(Unk11_Imp, void, 0xB32780);
		DEFINE_MEMBER_FUNCTION(Unk12_Imp, void, 0xB327F0, void * unk0);
		DEFINE_MEMBER_FUNCTION(Unk13_Imp, void, 0xB32840, void * unk0, void * unk1);
		DEFINE_MEMBER_FUNCTION(InitializeThis, void *, 0xB32360);
	protected:
		DEFINE_MEMBER_FUNCTION(ReleaseParent, void *, 0xB32420);
	};
	STATIC_ASSERT(offsetof(GameMenuBase, shaderTarget) == 0x88);

	// 00C
	class MenuTableItem
	{
	public:
		typedef IMenu * (*CallbackType)(void);
		BSFixedString	name;				// 000
		IMenu			* menuInstance;		// 008	0 if the menu is not currently open
		CallbackType	menuConstructor;	// 010
		void			* unk18;			// 018

		bool operator==(const MenuTableItem & rhs) const { return name == rhs.name; }
		bool operator==(const BSFixedString a_name) const { return name == a_name; }
		operator UInt64() const { return (UInt64)name.data->Get<char>(); }

		static inline UInt32 GetHash(BSFixedString * key)
		{
			UInt32 hash;
			CalculateCRC32_64(&hash, (UInt64)key->data, 0);
			return hash;
		}

		void Dump(void)
		{
			_MESSAGE("\t\tname: %s", name.data->Get<char>());
			_MESSAGE("\t\tinstance: %08X", menuInstance);
			_MESSAGE("\t\tconstructor: %08X", (uintptr_t)menuConstructor - RelocationManager::s_baseAddr);
		}
	};

	// 250 ?
	class UI
	{
	public:
		virtual ~UI();

		virtual void	Unk_01(void);

		typedef IMenu*	(*CreateFunc)(void);
		typedef tHashSet<MenuTableItem, BSFixedString> MenuTable;


		IMenu * GetMenu(BSFixedString * menuName);
		IMenu * GetMenuByMovie(GFxMovieView * movie);

		template<typename T>
		void ForEachMenu(T & menuFunc)
		{
			menuTable.ForEach(menuFunc);
		}

		UInt64									unk08;
		UInt64									unk10;
		BSTEventDispatcher<MenuOpenCloseEvent>	menuOpenCloseEventSource;
		UInt64									unk70[(0x190 - 0x70) / 8];	// 458
		tArray<IMenu*>							menuStack;		// 190
		MenuTable								menuTable;		// 1A8
		UInt64									unk1D8;         // 1D8
		UInt32									numPauseGame;   // 1E0 isInMenuMode
		volatile	SInt32						numFlag2000;	// 1E4
		volatile	SInt32						numFlag80;		// 1E8
		UInt32									numFlag20;		// 1EC													// ...


		void DumpMenuTable()
		{
			ForEachMenu([](MenuTableItem * item)->bool {
				item->Dump();
				_MESSAGE("");
				return true;
			});
		}


		DEFINE_MEMBER_FUNCTION(RegisterMenu, void, 0x020436E0, const char * name, CreateFunc creator, UInt64 unk1); //40 53 56 57 41 56 41 57 48 83 EC 50 44 8B 15 ? ? ? ?
		DEFINE_MEMBER_FUNCTION(IsMenuOpen, bool, 0x2041B50, BSFixedString & name); //E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 48 8B 3D ? ? ? ?
		DEFINE_MEMBER_FUNCTION(ProcessMessage, void, 0x2041E20);//40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 8B 15 ? ? ? ? 65 48 8B 04 25 ? ? ? ? 4C 8B E9
	};
	STATIC_ASSERT(sizeof(UI) == 0x1F0);
	extern RelocPtr <UI*> g_ui;

	using _RegisterMenuOpenCloseEvent = void(*)(BSTEventDispatcher<MenuOpenCloseEvent> & dispatcher, BSTEventSink<MenuOpenCloseEvent> * pHandler);
	extern RelocAddr<_RegisterMenuOpenCloseEvent>	RegisterMenuOpenCloseEvent;//call E8 ? ? ? ? 4C 8B AC 24 ? ? ? ? 4C 8B A4 24 ? ? ? ? 48 8B B4 24 ? ? ? ? 48 8D 5D D0

	using _UnregisterMenuOpenCloseEvent = void(*)(BSTEventDispatcher<MenuOpenCloseEvent> & dispatcher, BSTEventSink<MenuOpenCloseEvent> * pHandler);
	extern RelocAddr<_UnregisterMenuOpenCloseEvent>	UnregisterMenuOpenCloseEvent;
	//5D3950
}


