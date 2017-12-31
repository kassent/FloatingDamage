#include "Defination.h"
RelocAddr<_DisplayBookMenu>				DisplayBookMenu(0x12580E0);//40 53 55 56 57 41 54 41 55 41 56 41 57 48 83 EC 48 44 8B 15 ? ? ? ?
RelocAddr<_CalcInstanceData>			CalcInstanceData(0x2F7A30); ///48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 89 11 49 8B F8

//RelocAddr<HUDDataModel *>				hudDataModel(0x5A812E0);
RelocPtr<HUDDataModel *>				g_HUDDataModel(0x5A98A80);//48 8B 3D ? ? ? ? 0F 57 C0 66 0F 7F 45 ? 48 8D 45 A8
RelocPtr<HUDModeEventSource *>			g_HUDModeEventSource(0x5A98EC0); //RelocAddr 5A7F220 //48 8B 05 ? ? ? ? 48 85 C0 75 1E 48 8B 15 ? ? ? ? 48 8D 0D ? ? ? ? 48 83 C2 10 E8 ? ? ? ? 48 89 05 ? ? ? ? 48 8D 48 10 48 8B D7 E8 ? ? ? ? 48 8D 4F 08

RelocPtr<ViewCasterUpdateEventSource *>	g_viewCasterUpdateEventSource(0x591AB80); //48 8B 0D ? ? ? ? 48 8D 55 A8 E8 ? ? ? ? 48 8D 54 24 ?

RelocPtr<PipboyManager *>				g_pipboyManager(0x5A11E20); //48 8B 05 ? ? ? ? 49 8B D9 F3 0F 10 80 ? ? ? ?

RelocAddr<_PlayHolotape>				PlayHolotape(0xB905C0); // called by pipboy menu. 40 53 48 83 EC 20 48 8B D9 48 8B 0D ? ? ? ? 48 8B 81 ? ? ? ?

//RelocAddr<HUDQuickContainerDataModel *> g_qucikContainerDataModel(0x5A81318); //5A81318

RelocAddr<void *>						g_HUDQuickContainerVTBL(0x2D41588); //48 8D 05 ? ? ? ? 4C 8D 05 ? ? ? ? 48 89 06 48 8D 05 ? ? ? ? 48 8D 55 E0 48 8D 4E 08 48 89 46 50
//2D42528