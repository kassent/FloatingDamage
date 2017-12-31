#include "InputMenu.h"
#include "F4SE/GameAPI.h"
#include "F4SE/GameInput.h"
#include "RectDrawer.h"
#include "FW1FontWrapper.h"


uint32_t		Settings::iOffsetX = 100;
uint32_t		Settings::iOffsetY = 50;
uint32_t		Settings::iLineHeight = 30;
uint32_t		Settings::iLineWidth = 350;
uint32_t		Settings::iFontSize = 20;
uint32_t		Settings::iSeperatorHeight = 2;
uint32_t		Settings::iBackgroundColor = 0x00000000;
uint32_t		Settings::iCompositionTextColor = 0xFFFFFFFF;
uint32_t		Settings::iInputStateTextColor = 0xFFFFFFFF;
uint32_t		Settings::iSeperatorColor = 0xFFFFFFFF;
uint32_t		Settings::iNormalCandidateColor = 0xFFFFFFFF;
uint32_t		Settings::iSelectedCandidateColor = 0xFF0099FF;
uint32_t		Settings::iDisalbeMicrosoftWarning = 0;

std::wstring	Settings::sFontName = L"微软雅黑";


ICriticalSection g_criticalSection;




InputMenu* InputMenu::GetSingleton()
{
	static InputMenu* pInstance = new InputMenu();
	return pInstance;
}

InputMenu::InputMenu() : enableState(false), shieldKeyState(false)
{

}

extern uintptr_t	** pMain;

void InputMenu::Draw(ID3D11DeviceContext * pContext, IFW1FontWrapper * pFontWrapper, RectDrawer * pRectDrawer)
{
	if (pContext && pFontWrapper)
	{
		if (*g_inputMgr && (*g_inputMgr)->allowTextInput)
		{
			uintptr_t iSelectedIndex = InterlockedCompareExchange64(&selectedIndex, selectedIndex, -1);
			uintptr_t iPageStartIndex = InterlockedCompareExchange64(&pageStartIndex, pageStartIndex, -1);

			g_criticalSection.Enter();

			if (!InterlockedCompareExchange64(&enableState, 1, 1) || !compositionContent.size())
			{
				if (Settings::iDisalbeMicrosoftWarning && (inputStateInfo.find(L"微软拼音") != std::wstring::npos || inputStateInfo.find(L"微软五笔") != std::wstring::npos))
				{
					//RelocPtr<uintptr_t*> pMain = 0x601CF00;
					float iWidth = *reinterpret_cast<UInt32*>((uintptr_t)(*pMain) + 0x68) / 2;
					float iHeight = *reinterpret_cast<UInt32*>((uintptr_t)(*pMain) + 0x6C) / 2;
					FW1_RECTF warningRect = { 0, iHeight - 60, iWidth * 2, iHeight + 60 };
					pRectDrawer->DrawRect(pContext, warningRect.Left, warningRect.Top, warningRect.Right, warningRect.Bottom, 0xFFFF9911);
					pFontWrapper->DrawString(pContext, L"使用微软输入法频繁输入会有很大几率导致CTD，请使用其它输入法如手心输入法等。", Settings::sFontName.c_str(), 30, &warningRect, Settings::iCompositionTextColor, nullptr, NULL, FW1_CENTER | FW1_VCENTER);
				}
				g_criticalSection.Leave();
				return;
			}
			FW1_RECTF textRect = { static_cast<float>(Settings::iOffsetX + 5), static_cast<float>(Settings::iOffsetY), static_cast<float>(Settings::iOffsetX + 5 + Settings::iLineWidth),static_cast<float>(Settings::iOffsetY + Settings::iLineHeight) };

			if ((Settings::iBackgroundColor >> 24))
			{
				FW1_RECTF backgroundRect = { static_cast<float>(Settings::iOffsetX - 5), static_cast<float>(Settings::iOffsetY - 10), static_cast<float>(Settings::iOffsetX + 15 + Settings::iLineWidth),static_cast<float>(Settings::iOffsetY + Settings::iLineHeight * (candidataContent.size() + 2) + Settings::iSeperatorHeight + 20) };
				pRectDrawer->DrawRect(pContext, backgroundRect.Left, backgroundRect.Top, backgroundRect.Right, backgroundRect.Bottom, Settings::iBackgroundColor);
			}

			static const wchar_t* sFontName = Settings::sFontName.c_str();
			static const uint32_t iFontSize = (Settings::iFontSize < Settings::iLineHeight) ? Settings::iFontSize : Settings::iLineHeight;

			pFontWrapper->DrawString(pContext, compositionContent.c_str(), sFontName, iFontSize, &textRect, Settings::iCompositionTextColor, &textRect, NULL, FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP | FW1_CLIPRECT);
			pRectDrawer->DrawRect(pContext, textRect.Left - 5, textRect.Bottom, textRect.Right + 5, textRect.Bottom + Settings::iSeperatorHeight, Settings::iSeperatorColor);

			textRect.Top += Settings::iLineHeight + Settings::iSeperatorHeight;
			textRect.Bottom += Settings::iLineHeight + Settings::iSeperatorHeight;

			pFontWrapper->DrawString(pContext, inputStateInfo.c_str(), sFontName, iFontSize, &textRect, Settings::iInputStateTextColor, &textRect, NULL, FW1_LEFT | FW1_TOP | FW1_NOWORDWRAP | FW1_CLIPRECT);

			textRect.Top += 6;
			textRect.Bottom += 6;

			for (size_t i = 0; i < candidataContent.size(); ++i)
			{
				textRect.Top += Settings::iLineHeight;
				textRect.Bottom += Settings::iLineHeight;
				if (iSelectedIndex != (i + iPageStartIndex))
				{
					pFontWrapper->DrawString(pContext, candidataContent[i].c_str(), sFontName, iFontSize, &textRect, Settings::iNormalCandidateColor, &textRect, NULL, FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP | FW1_CLIPRECT);
				}
				else
				{
					pFontWrapper->DrawString(pContext, candidataContent[i].c_str(), sFontName, iFontSize, &textRect, Settings::iSelectedCandidateColor, &textRect, NULL, FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP | FW1_CLIPRECT);
				}
			}
			g_criticalSection.Leave();
		}
	}
}