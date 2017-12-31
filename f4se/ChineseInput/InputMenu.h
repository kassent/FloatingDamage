#pragma once
#include <vector>
#include <string>
#include "common/ICriticalSection.h"

#define MAX_CANDLIST 10

struct ID3D11DeviceContext;
struct IFW1FontWrapper;
class  RectDrawer;


struct Settings
{
	static uint32_t				iOffsetX;
	static uint32_t				iOffsetY;
	static uint32_t				iLineHeight;
	static uint32_t				iLineWidth;
	static uint32_t				iFontSize;
	static uint32_t				iSeperatorHeight;
	static uint32_t				iBackgroundColor;
	static uint32_t				iCompositionTextColor;
	static uint32_t				iInputStateTextColor;
	static uint32_t				iSeperatorColor;
	static uint32_t				iNormalCandidateColor;
	static uint32_t				iSelectedCandidateColor;

	static uint32_t				iDisalbeMicrosoftWarning;
	static std::wstring			sFontName;
};

class InputMenu
{
public:
	InputMenu();

	void	Draw(ID3D11DeviceContext * pImmediateContext, IFW1FontWrapper *	 pFontWrapper, RectDrawer * pRectDrawer);

	static	InputMenu*	GetSingleton();

	HWND							pHandle;
	volatile intptr_t				enableState;
	volatile intptr_t				shieldKeyState;
	volatile intptr_t				pageStartIndex;
	volatile intptr_t				pageSize;
	volatile intptr_t				selectedIndex;
	std::wstring					inputStateInfo;
	std::wstring					compositionContent;
	std::vector<std::wstring>		candidataContent;
};



extern ICriticalSection g_criticalSection;