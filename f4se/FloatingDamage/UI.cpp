#include "UI.h"

namespace UIFramework
{
	RelocAddr<_CreateBaseShaderTarget>	CreateBaseShaderTarget(0xB06C20); //(call)E8 ? ? ? ? 49 8B 9E ? ? ? ? 4C 8D 05 ? ? ? ?

	RelocAddr<_SetFilterColorType>	SetFilterColorType(0x20F1780); //40 53 48 83 EC 40 89 91 ? ? ? ?

	RelocAddr<_GetChildElement>		GetChildElement(0x20F07B0);    //40 53 48 83 EC 30 33 C0 48 8B DA

	RelocAddr <_PlayUISound>		PlayUISound(0x12BDD80); //48 89 5C 24 ? 57 48 83 EC 50 48 8B D9 E8 ? ? ? ? 48 85 C0

	RelocPtr <BSScaleformManager *> g_scaleformManager(0x5916490);//48 8B 0D ? ? ? ? 48 8B 49 18 E8 ? ? ? ? 48 8B 44 24 ?

	RelocPtr <UIMessageManager*>	g_uiMessageManager(0x5908B48); //48 8B 3D ? ? ? ? E8 ? ? ? ? 41 B8 ? ? ? ? 48 8B CF

	RelocPtr <UI*>					g_ui(0x5908918); //48 8B 3D ? ? ? ? E8 ? ? ? ? 48 8B CF 48 8B D0 E8 ? ? ? ? 84 C0 0F 85 ? ? ? ?

	RelocAddr <_RegisterMenuOpenCloseEvent>	RegisterMenuOpenCloseEvent(0x5D1120);//call E8 ? ? ? ? 4C 8B AC 24 ? ? ? ? 4C 8B A4 24 ? ? ? ? 48 8B B4 24 ? ? ? ? 48 8D 5D D0

	RelocAddr<_UnregisterMenuOpenCloseEvent>	UnregisterMenuOpenCloseEvent(0x5D3950);

	RelocPtr <SimpleLock>		globalMenuStackLock(0x65AF4B0);

	RelocPtr <SimpleLock>		globalMenuTableLock(0x65AF4B8);
	//5D3950
	DelegateArgs::DelegateArgs(IMenu * menu, ScaleformArgs * args)
	{
		menuName = menu->menuName.c_str();
		optionID = args->optionID;
		for (UInt32 i = 0; i < args->numArgs; ++i)
		{
			parameters.push_back(args->args[i]);
		}
	}
	ScaleformArgs::ScaleformArgs(IMenu * menu, const DelegateArgs & delegateArgs) : result(nullptr), thisObj(nullptr), unk18(nullptr), optionID(delegateArgs.optionID)
	{
		//call this function within thread lock.
		BSFixedString menuName(delegateArgs.menuName.c_str());
		movie = (menu != nullptr) ? menu->movie : nullptr;
		this->numArgs = delegateArgs.parameters.size();
		this->args = (this->numArgs) ? &delegateArgs.parameters[0] : nullptr;
	}

	IMenu * UI::GetMenu(BSFixedString * menuName)
	{
		if (!menuName || !menuName->data->Get<char>())
			return NULL;

		MenuTableItem * item = menuTable.Find(menuName);

		if (!item)
			return NULL;

		IMenu * menu = item->menuInstance;
		if (!menu)
			return NULL;

		return menu;
	}

	bool UI::IsMenuRegistered(BSFixedString & menuName)
	{
		SimpleLocker locker(globalMenuTableLock);
		MenuTableItem * item = menuTable.Find(&menuName);
		if (item) {
			return true;
		}

		return false;
	}

	IMenu * UI::GetMenuByMovie(GFxMovieView * movie)
	{
		if (!movie)
			return NULL;

		IMenu * menu = NULL;
		menuTable.ForEach([movie, &menu](MenuTableItem * item)
		{
			IMenu * itemMenu = item->menuInstance;
			if (itemMenu) {
				GFxMovieView * view = itemMenu->movie;
				if (view) {
					if (movie == view) {
						menu = itemMenu;
						return false;
					}
				}
			}
			return true;
		});

		return menu;
	}
}