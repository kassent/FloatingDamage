#include "InputUtil.h"
#include "InputMenu.h"
#include "F4SE/GameAPI.h"
#include "F4SE/GameInput.h"
#include <memory>

#pragma comment (lib, "imm32.lib")
#include "imm.h"

//#define MAX_CANDLIST 10
extern bool					* isInputEnabled;

using _ProcessCharEvent = void(*)(BSPCKeyboardDevice*, WPARAM, LPARAM);
extern _ProcessCharEvent	ProcessCharEvent; //V1.10.26

namespace InputUtil
{
	void Convert(const char* sIn, char* sOut, int sourceCodepage, int targetCodepage)
	{
		UInt32 unicodeLen = MultiByteToWideChar(sourceCodepage, 0, sIn, -1, nullptr, 0);
		std::unique_ptr<wchar_t[]> sWchar(new wchar_t[unicodeLen + 1]);
		memset(sWchar.get(), 0, (unicodeLen + 1) * sizeof(wchar_t));
		MultiByteToWideChar(sourceCodepage, 0, sIn, -1, sWchar.get(), unicodeLen);
		UInt32 sResultLength = WideCharToMultiByte(targetCodepage, 0, sWchar.get(), -1, nullptr, 0, nullptr, nullptr);
		std::unique_ptr<char[]> sResult(new char[sResultLength + 1]);
		memset(sResult.get(), 0, sResultLength + 1);
		WideCharToMultiByte(targetCodepage, 0, sWchar.get(), -1, sResult.get(), sResultLength, nullptr, nullptr);
		lstrcpy(sOut, sResult.get());
	}



	void SendUnicodeMessage(uintptr_t wCharCode)
	{
		if (*g_inputDeviceMgr && *isInputEnabled && *g_inputMgr && (*g_inputMgr)->allowTextInput)
		{
#ifdef _DEBUG
			_MESSAGE("[CI] wCharCode: %016I64X", wCharCode);
#endif
			ProcessCharEvent((*g_inputDeviceMgr)->keyboardDevice, wCharCode, 0);
		}
	}


	void GetCandidateListW(const HWND& hWnd)
	{
		auto pInputMenu = InputMenu::GetSingleton();
		if (!InterlockedCompareExchange64(&pInputMenu->enableState, 1, 1))
			return;
		auto pContext = ImmGetContext(hWnd);
		if (pContext != nullptr)
		{
			unsigned long candidateSize = ImmGetCandidateListW(pContext, 0, nullptr, 0);
			if (candidateSize)
			{
				std::unique_ptr<CANDIDATELIST> pList(reinterpret_cast<LPCANDIDATELIST>(new char[candidateSize]));
				if (!pList)
				{
					ImmReleaseContext(hWnd, pContext);
					return;
				}
				ImmGetCandidateListW(pContext, 0, pList.get(), candidateSize);
				if (pList->dwStyle != IME_CAND_CODE)
				{
					wchar_t prefix[0x8];

					InterlockedExchange64(&pInputMenu->selectedIndex, pList->dwSelection);
					InterlockedExchange64(&pInputMenu->pageStartIndex, pList->dwPageStart);

					g_criticalSection.Enter();
					pInputMenu->candidataContent.clear();
					for (int i = 0; i < pList->dwCount && i < pList->dwPageSize && i < MAX_CANDLIST; ++i)
					{
						wsprintfW(prefix, L"%d.", i + 1);
						wchar_t* pSubStr = (wchar_t*)((char*)(pList.get()) + pList->dwOffset[i + pList->dwPageStart]);
						std::wstring temp(prefix);
						temp += pSubStr;
						pInputMenu->candidataContent.push_back(std::move(temp));
					}
					g_criticalSection.Leave();
				}
			}
			ImmReleaseContext(hWnd, pContext);
		}
	}




	void GetCompositionStringW(const HWND& hWnd)
	{
		HIMC pContext = ImmGetContext(hWnd);
		long unicodeLen = 0;
		unicodeLen = ImmGetCompositionStringW(pContext, GCS_COMPSTR, nullptr, 0);
#ifdef _DEBUG
		_MESSAGE("[CI] Input wstring length: %d", unicodeLen);
#endif 
		if (unicodeLen > 0 && !(unicodeLen & 1)) //it is an odd number
		{
			unicodeLen = (unicodeLen & 1) ? unicodeLen + 1 : unicodeLen;
			std::unique_ptr<wchar_t[]> sResult(new wchar_t[unicodeLen / 2 + 1]);
			sResult[unicodeLen / 2] = '\0';
			ImmGetCompositionStringW(pContext, GCS_COMPSTR, sResult.get(), unicodeLen + sizeof(wchar_t));

			auto pInputMenu = InputMenu::GetSingleton();
			g_criticalSection.Enter();
			pInputMenu->compositionContent = sResult.get();
			g_criticalSection.Leave();
		}
	}



	void GetResultString(const HWND& hWnd)
	{
		HIMC pContext = ImmGetContext(hWnd);
		if (pContext)
		{
			DWORD bufferSize = ImmGetCompositionStringW(pContext, GCS_RESULTSTR, nullptr, 0);
			if (bufferSize)
			{
				bufferSize += sizeof(WCHAR);
				std::unique_ptr<WCHAR[]> wCharBuffer(reinterpret_cast<WCHAR*>(new char[bufferSize]));
				if (!wCharBuffer)
				{
					ImmReleaseContext(hWnd, pContext);
					return;
				}
				ZeroMemory(wCharBuffer.get(), bufferSize);
				ImmGetCompositionStringW(pContext, GCS_RESULTSTR, wCharBuffer.get(), bufferSize);
				size_t len = bufferSize / sizeof(WCHAR);

				for (size_t i = 0; i < len; ++i)
				{
					uintptr_t unicode = static_cast<uintptr_t>(wCharBuffer[i]);
					if (unicode > 0)
					{
						SendUnicodeMessage(unicode);
					}
				}
			}
		}
		ImmReleaseContext(hWnd, pContext);
	}
}