#pragma comment (lib, "D3D11.lib")
#include "Hooks.h"
#include "HookUtil.h"
#include "InputMenu.h"
#include "InputUtil.h"
#include "Cicero.h"
#include "Async.h"

#include "F4SE_common/Relocation.h"
#include "F4SE_common/SafeWrite.h"
#include "F4SE_common/Utilities.h"
#include "F4SE_common/BranchTrampoline.h"
#include "F4SE/GameInput.h"
#include "F4SE/ScaleformCallbacks.h"
#include "F4SE/ScaleformMovie.h"
#include "F4SE/ScaleformValue.h"


#include "FW1FontWrapper.h"
#include "xbyak/xbyak.h"
#include <typeinfo>
#include <D3D11.h>
#include "RectDrawer.h"
#include <algorithm>

#define DEBUG	1

#ifdef DEBUG
#include "TlHelp32.h"
#endif

#define WM_IME_SETSTATE 0x0655


WNDPROC					FnWndProc = nullptr;

bool					* isInputEnabled = nullptr;//(0x37AC32C);

uintptr_t				** pMain = nullptr;

using _ProcessCharEvent = void(*)(BSPCKeyboardDevice*, WPARAM, LPARAM);
_ProcessCharEvent		ProcessCharEvent = nullptr;//(0x01B2D3A0); //V1.10.26

using _GetInputRawData = UINT(WINAPI*)(HRAWINPUT, UINT, LPVOID, PUINT, UINT);
//RelocPtr<_GetInputRawData> GetInputRawData(0x2C2E1B0);
_GetInputRawData GetInputRawData_Original = nullptr;

using _InitDevice = HRESULT(*)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, CONST D3D_FEATURE_LEVEL*, UINT, UINT, CONST DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);
_InitDevice InitDevice = nullptr; //V1.10.26 (0x2C2E498)


UINT WINAPI GetInputRawData_Hook(HRAWINPUT rawinput, UINT cmd, void * data, UINT * dataSize, UINT headerSize)
{
	UINT result = GetInputRawData_Original(rawinput, cmd, data, dataSize, headerSize);

	if (data)
	{
		RAWINPUT * input = (RAWINPUT *)data;

		if (cmd == RID_INPUT && input->header.dwType == RIM_TYPEKEYBOARD && *g_inputMgr && (*g_inputMgr)->allowTextInput)
		{
			g_criticalSection.Enter();
			auto pInputMenu = InputMenu::GetSingleton();
			if (InterlockedCompareExchange64(&pInputMenu->enableState, 1, 1) && pInputMenu->compositionContent.size())
			{
				//static std::vector<UInt8> makeCodes = { 0x2F, 0xF, 0xE, 0x1C, 0x29, 0x1, 0x4B, 0x48, 0x4D, 0x50 };
				static std::vector<UInt8> virtualKeys = { 0x56, 0x9, 0x8, 0xD, 0xC0, 0x1B, 0x25, 0x26, 0x27, 0x28 };
				if (std::find(virtualKeys.begin(), virtualKeys.end(), input->data.keyboard.VKey) != virtualKeys.end())
				{
					//pRawInput->data.keyboard.VKey = 0xFF;
					input->data.keyboard.Flags |= RI_KEY_BREAK;
					//_MESSAGE("RAW: %08X   %08X   %d", pRawInput->data.keyboard.MakeCode, pRawInput->data.keyboard.VKey, pRawInput->data.keyboard.Flags);
				}
			}
			g_criticalSection.Leave();
		}
	}
	return result;
}

LRESULT CALLBACK CustomWindowProc_Hook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		//		case WM_INPUT:
		//		{
		//			if (*RelocAddr<bool*>(0x37AC32C) && *g_inputDeviceMgr) //V1.10.26  /5A7D9F8
		//			{
		//				RAWINPUT* pRawInput = nullptr;
		//				UINT iSize = 0;
		//				(*GetInputRawData)(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &iSize, 0x18);
		//				if (iSize <= 0x100)
		//				{
		//					iSize = 0x100;
		//					pRawInput = reinterpret_cast<RAWINPUT*>(RelocAddr<RAWINPUT*>(0x5AC2760).GetUIntPtr()); //V1.10.26
		//				}
		//				else
		//				{
		//#ifdef _DEBUG
		//					RelocAddr<RAWINPUT*(*)(UINT)> fnCreateRawInputData= 0x42EE0; //V1.10.26
		//					pRawInput = fnCreateRawInputData(iSize);
		//#else
		//					return DefWindowProc(hWnd, uMsg, wParam, lParam);
		//#endif
		//				}
		//				(*GetInputRawData)(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, pRawInput, &iSize, 0x18);
		//				if (pRawInput->header.dwType == RIM_TYPEKEYBOARD)
		//				{
		//					auto pInputMenu = InputMenu::GetSingleton();
		//					if (*g_inputMgr && (*g_inputMgr)->allowTextInput && InterlockedCompareExchange64(&pInputMenu->enableState, 1, 1))
		//					{
		//						//static std::vector<UInt8> makeCodes = { 0x2F, 0xF, 0xE, 0x1C, 0x29, 0x1, 0x4B, 0x48, 0x4D, 0x50 };
		//						static std::vector<UInt8> virtualKeys = { 0x56, 0x9, 0x8, 0xD, 0xC0, 0x1B, 0x25, 0x26, 0x27, 0x28 };
		//						if (std::find(virtualKeys.begin(), virtualKeys.end(), pRawInput->data.keyboard.VKey) != virtualKeys.end())
		//						{
		//							//pRawInput->data.keyboard.VKey = 0xFF;
		//							pRawInput->data.keyboard.Flags |= RI_KEY_BREAK;
		//							//_MESSAGE("RAW: %08X   %08X   %d", pRawInput->data.keyboard.MakeCode, pRawInput->data.keyboard.VKey, pRawInput->data.keyboard.Flags);
		//						}
		//					}
		//					auto pKeyboardDevice = (*g_inputDeviceMgr)->keyboardDevice;
		//					RelocAddr<void(*)(BSPCKeyboardDevice*, RAWINPUT*)> fnProcessKeyboardRawInputData = 0x1B2D1C0; //V1.10.26
		//					fnProcessKeyboardRawInputData(pKeyboardDevice, pRawInput);
		//				}
		//				else if (pRawInput->header.dwType == RIM_TYPEMOUSE)
		//				{
		//					auto pMouseDevice = (*g_inputDeviceMgr)->mouseDevice;
		//					RelocAddr<void(*)(BSPCMouseDevice*, RAWINPUT*)> fnProcessMouseRawInputData = 0x01B2D540; //V1.10.26
		//					fnProcessMouseRawInputData(pMouseDevice, pRawInput);
		//				}
		//#ifdef _DEBUG
		//				if (iSize > 0x100 && pRawInput != reinterpret_cast<RAWINPUT*>(RelocAddr<RAWINPUT*>(0x5AC2760).GetUIntPtr()))
		//				{
		//					RelocAddr<void(*)(RAWINPUT*)> fnReleaseRawInputData = 0x021BD0; //V1.10.26
		//					fnReleaseRawInputData(pRawInput);
		//				}
		//#endif // DEBUG
		//				return 0;
		//			}
		//			else
		//			{
		//				return DefWindowProc(hWnd, uMsg, wParam, lParam);
		//			}
		//		}
	case WM_IME_NOTIFY:
	{
		switch (wParam)
		{
		case IMN_OPENCANDIDATE:
		case IMN_SETCANDIDATEPOS:
		case IMN_CHANGECANDIDATE:
			auto pInputMenu = InputMenu::GetSingleton();
			auto pCicero = CiceroInputMethod::GetSingleton();
			InterlockedExchange64(&pInputMenu->enableState, 1);
			if (*g_inputMgr && (*g_inputMgr)->allowTextInput && !pCicero->m_ciceroState)
			{
				InputUtil::GetCandidateListW(hWnd);
			}
		}
		return 0;

	}
	case WM_IME_STARTCOMPOSITION:
	{
		if (*g_inputMgr && (*g_inputMgr)->allowTextInput)
		{
			auto pInputMenu = InputMenu::GetSingleton();
			auto pCicero = CiceroInputMethod::GetSingleton();
			InterlockedExchange64(&pInputMenu->enableState, 1);
			InterlockedExchange64(&pInputMenu->shieldKeyState, 1);
			if (!pCicero->m_ciceroState)
			{
				auto f = [=](UInt32 time)->bool {std::this_thread::sleep_for(std::chrono::milliseconds(time)); InputUtil::GetCandidateListW(hWnd); return true; };
				really_async(f, 200);
				really_async(f, 300);
			}
		}
		return 0;
	}
	case WM_IME_COMPOSITION:
	{
		if (*g_inputMgr && (*g_inputMgr)->allowTextInput)
		{
			if (lParam & GCS_COMPSTR)
				InputUtil::GetCompositionStringW(hWnd);
			if (lParam & GCS_RESULTSTR)
				InputUtil::GetResultString(hWnd);
		}
		return 0;
	}
	case WM_IME_ENDCOMPOSITION:
	{
		auto pInputMenu = InputMenu::GetSingleton();
		InterlockedExchange64(&pInputMenu->enableState, 0);

		g_criticalSection.Enter();
		pInputMenu->candidataContent.clear();
		pInputMenu->compositionContent.clear();
		g_criticalSection.Leave();
		return 0;
	}
	case WM_IME_SETSTATE:
	{
		auto pInputMenu = InputMenu::GetSingleton();
		(lParam) ? ImmAssociateContextEx(pInputMenu->pHandle, NULL, IACE_DEFAULT) : ImmAssociateContextEx(pInputMenu->pHandle, NULL, NULL);
		break;
	}
	case WM_IME_CHAR:
	case WM_SYSKEYUP:
		return 0;
	case WM_IME_SETCONTEXT:
	{
		return DefWindowProc(hWnd, WM_IME_SETCONTEXT, wParam, NULL);
	}
	case WM_CHAR:
	{
		InputUtil::SendUnicodeMessage(wParam);
		return 0;
	}
	}
	return FnWndProc(hWnd, uMsg, wParam, lParam);
}



class DX11
{
public:
	static 	IDXGISwapChain	*			pSwapChain;
	static  ID3D11Device *				pDevice;
	static  ID3D11DeviceContext *		pImmediateContext;
	static	IFW1FontWrapper *			pFontWrapper;
	static	RectDrawer *				pRectDrawer;

	using FnPresent = HRESULT(*)(IDXGISwapChain*, UINT, UINT);
	using FnSetViewports = HRESULT(*)(ID3D11DeviceContext*, UINT, const D3D11_VIEWPORT*);

	static FnPresent					fnPresent;
	static FnSetViewports				fnSetViewports;

	static HRESULT SetViewports_Hook(ID3D11DeviceContext * pImmediateContext, UINT NumViewports, const D3D11_VIEWPORT *pViewports)
	{
		_MESSAGE("[CI] numViewports: %d    pViewports: %016I64X    width: %.2f    height: %.2f    X: %.2f    Y: %.2f", NumViewports, (uintptr_t)pViewports, pViewports->Width, pViewports->Height, pViewports->TopLeftX, pViewports->TopLeftY);
		return fnSetViewports(pImmediateContext, NumViewports, pViewports);
	}

	static HRESULT Present_Hook(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
	{
		auto pInputMenu = InputMenu::GetSingleton();
#ifdef _DEBUG
		_MESSAGE("syncInterval: %d    flags: %d", SyncInterval, Flags);
		RelocPtr<uintptr_t*> pMain = 0x601CF00;
		UInt32 iWidth = *reinterpret_cast<UInt32*>((uintptr_t)(*pMain) + 0x68);
		UInt32 iHeight = *reinterpret_cast<UInt32*>((uintptr_t)(*pMain) + 0x6C);
		_MESSAGE("[CI] width: %d    height: %d", iWidth, iHeight);

#endif 
		static UInt8 lastRecord = 0;
		if (*g_inputMgr != nullptr)
		{
			if ((*g_inputMgr)->allowTextInput && !lastRecord)
			{
				PostMessage(pInputMenu->pHandle, WM_IME_SETSTATE, NULL, 1);
			}
			else if (!(*g_inputMgr)->allowTextInput && lastRecord == 1)
			{
				PostMessage(pInputMenu->pHandle, WM_IME_SETSTATE, NULL, 0);
				g_criticalSection.Enter();
				pInputMenu->candidataContent.clear();
				pInputMenu->compositionContent.clear();
				g_criticalSection.Leave();
				InterlockedExchange64(&pInputMenu->enableState, 0);
			}
			lastRecord = (*g_inputMgr)->allowTextInput;
		}
		pInputMenu->Draw(pImmediateContext, pFontWrapper, pRectDrawer);
		return fnPresent(pSwapChain, SyncInterval, Flags);
	}

	static HRESULT InitDevice_Hook(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, IDXGISwapChain** ppSwapChain, ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext)
	{
		HRESULT result = InitDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
		pSwapChain = *ppSwapChain;
		pDevice = *ppDevice;
		pImmediateContext = *ppImmediateContext;
		if (!fnPresent)
		{
			fnPresent = HookUtil::SafeWrite64(*(uintptr_t*)pSwapChain + 8 * 0x8, Present_Hook);
		}
#ifdef _DEBUG
		if (!fnSetViewports)
		{
			fnSetViewports = HookUtil::SafeWrite64(*(uintptr_t*)pImmediateContext + 44 * 0x8, SetViewports_Hook);
		}
#endif
		IFW1Factory * pFontFactory = nullptr;
		HRESULT hr = FW1CreateFactory(FW1_VERSION, &pFontFactory);
		if (FAILED(hr))
		{
			_MESSAGE("[CI] Failed to create font factory...");
		}

		hr = pFontFactory->CreateFontWrapper(pDevice, Settings::sFontName.c_str(), &pFontWrapper);
		if (FAILED(hr))
		{
			_MESSAGE("[CI] Failed to create font wrapper...");
		}

		pFontFactory->Release();

		pRectDrawer = new RectDrawer;
		if (!pRectDrawer->Init(pDevice))
		{
			_MESSAGE("[CI] Failed to create rect drawer...");
		}

		_MESSAGE("[CI] pSwapChain: %016I64X    pDevice: %016I64X    pImmediateContext: %016I64X", (uintptr_t)pSwapChain, (uintptr_t)pDevice, (uintptr_t)pImmediateContext);
		return result;
	}

	static void InitHook()
	{
		uintptr_t location = RVAScan<uintptr_t>("E8 ? ? ? ? 4C 8B 05 ? ? ? ? 8B 0D ? ? ? ? 49 8D 50 58").GetUIntPtr(); //0x294BB3E V1.10.26
		_MESSAGE("(%016I64X) patched sucessfully...", location - RelocationManager::s_baseAddr);
		auto ReadOffsetData = [](uintptr_t location, SInt32 relOffset, UInt32 len)->uintptr_t
		{
			SInt32 rel32 = 0;
			RVAUtils::ReadMemory(location + relOffset, &rel32, sizeof(SInt32));
			_MESSAGE("(%016I64X) patched sucessfully...", location + len + rel32 - RelocationManager::s_baseAddr);
			return location + len + rel32;
		};

		InitDevice = reinterpret_cast<_InitDevice>(ReadOffsetData(location, 1, 5));
		g_branchTrampoline.Write5Call(location, (uintptr_t)InitDevice_Hook); //V1.10.26
	}
};
RectDrawer *					DX11::pRectDrawer = nullptr;
IDXGISwapChain *				DX11::pSwapChain = nullptr;
ID3D11Device *					DX11::pDevice = nullptr;
ID3D11DeviceContext *			DX11::pImmediateContext = nullptr;
IFW1FontWrapper *				DX11::pFontWrapper = nullptr;
DX11::FnPresent					DX11::fnPresent = nullptr;
DX11::FnSetViewports			DX11::fnSetViewports = nullptr;



class InputManagerEx : public InputManager
{
public:

	void AllowTextInput_Hook(bool condition)
	{
		auto pInputMenu = InputMenu::GetSingleton();
		if (condition && !allowTextInput)
		{
			PostMessage(pInputMenu->pHandle, WM_IME_SETSTATE, NULL, 1);
#ifdef DEBUG
			_MESSAGE("[CI] Enable text input...");
#endif 
		}
		else if (!condition && allowTextInput == 1)
		{
			PostMessage(pInputMenu->pHandle, WM_IME_SETSTATE, NULL, 0);
			g_criticalSection.Enter();
			pInputMenu->candidataContent.clear();
			pInputMenu->compositionContent.clear();
			g_criticalSection.Leave();
			InterlockedExchange64(&pInputMenu->enableState, 0);
#ifdef DEBUG
			_MESSAGE("[CI] Disable text input...");
#endif 
		}
		AllowTextInput(condition);
	}

	static void InitHook()
	{
		//g_branchTrampoline.Write6Branch(RelocAddr<uintptr_t*>(0x1B268F0).GetUIntPtr(),  GetFnAddr(&InputManagerEx::AllowTextInput_Hook)); //V1.10
	}
};




class MainEx
{
public:
	static LPTOP_LEVEL_EXCEPTION_FILTER			fnExceptionfilter;

#ifdef DEBUG
	static void PrintModuleInfo()
	{
		HANDLE	snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
		if (snap != INVALID_HANDLE_VALUE)
		{
			MODULEENTRY32	module;
			module.dwSize = sizeof(module);
			if (Module32First(snap, &module))
			{
				do
				{
					_MESSAGE("%016I64Xx%016I64X %016I64X - %016I64X %s %s", (uintptr_t)module.modBaseAddr, (uintptr_t)module.modBaseSize, (uintptr_t)module.hModule, (uintptr_t)module.modBaseAddr + (uintptr_t)module.modBaseSize, module.szModule, module.szExePath);
				} while (Module32Next(snap, &module));
			}
			else
			{
				_MESSAGE("PrintModuleInfo: Module32First failed (%d)", GetLastError());
			}
			CloseHandle(snap);
		}
		else
		{
			_MESSAGE("PrintModuleInfo: CreateToolhelp32Snapshot failed (%d)", GetLastError());
		}
	}



	static LONG __stdcall CustomExceptionFilter(EXCEPTION_POINTERS * info)
	{
		//_MESSAGE("try to record exception...");
		static bool bSkip = false;
		if (!bSkip)
		{
			bSkip = true;
			__try
			{
				_MESSAGE("CRASHLOG:");
				_MESSAGE("RIP(%016I64X)", info->ContextRecord->Rip);
				_MESSAGE("RAX(%016I64X)", info->ContextRecord->Rax);
				_MESSAGE("RBX(%016I64X)", info->ContextRecord->Rbx);
				_MESSAGE("RCX(%016I64X)", info->ContextRecord->Rcx);
				_MESSAGE("RDX(%016I64X)", info->ContextRecord->Rdx);
				_MESSAGE("RSI(%016I64X)", info->ContextRecord->Rsi);
				_MESSAGE("RDI(%016I64X)", info->ContextRecord->Rdi);
				_MESSAGE("RBP(%016I64X)", info->ContextRecord->Rbp);
				_MESSAGE("RSP(%016I64X)", info->ContextRecord->Rsp);
			}
			__except (1)
			{
				_MESSAGE("failed to write log...");
			}
		}
		//if (fnExceptionfilter != nullptr)
		//	fnExceptionfilter(info); 

		return (true) ? EXCEPTION_CONTINUE_SEARCH : EXCEPTION_EXECUTE_HANDLER;
	}
#endif
	static void InitCicero(const HWND hWnd)
	{
		if (*pMain != nullptr)
		{
			*reinterpret_cast<HWND*>(*pMain + 0xB) = hWnd;
			ImmAssociateContextEx(hWnd, 0, 0);
			auto pCicero = CiceroInputMethod::GetSingleton();
			pCicero->SetupSinks();
			auto inputMenu = InputMenu::GetSingleton();
			inputMenu->pHandle = hWnd;
#ifdef DEBUG
			fnExceptionfilter = SetUnhandledExceptionFilter(&CustomExceptionFilter);
			PrintModuleInfo();
#endif
		}
	}

	static void InitHook()
	{
		{
			static uintptr_t location = RVAScan<uintptr_t>("48 8D 05 ? ? ? ? 89 4C 24 68").GetUIntPtr(); //0x294BB3E V1.10.26
			_MESSAGE("(%016I64X) patched sucessfully...", location - RelocationManager::s_baseAddr);
			auto ReadOffsetData = [](uintptr_t location, SInt32 relOffset, UInt32 len)->uintptr_t
			{
				SInt32 rel32 = 0;
				RVAUtils::ReadMemory(location + relOffset, &rel32, sizeof(SInt32));
				_MESSAGE("(%016I64X) patched sucessfully...", location + len + rel32 - RelocationManager::s_baseAddr);
				return location + len + rel32;
			};

			FnWndProc = reinterpret_cast<WNDPROC>(ReadOffsetData(location, 3, 7));

			//static const RelocAddr <uintptr_t> CreateWndProc_Ent = 0xD4038A; //00D402FA         V1.10.26

			struct CreateWndProc_Code : Xbyak::CodeGenerator {
				CreateWndProc_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;

					mov(rax, (uintptr_t)CustomWindowProc_Hook);
					jmp(ptr[rip + retnLabel]);

					L(retnLabel);
					dq(location + 7);

				}
			};

			void * codeBuf = g_localTrampoline.StartAlloc();
			CreateWndProc_Code code(codeBuf);
			g_localTrampoline.EndAlloc(code.getCurr());

			g_branchTrampoline.Write6Branch(location, uintptr_t(code.getCode()));
		}

		{

			static uintptr_t location = RVAScan<uintptr_t>("48 8B 0D ? ? ? ? 48 89 41 58").GetUIntPtr(); //0x294BB3E V1.10.26
			_MESSAGE("(%016I64X) patched sucessfully...", location - RelocationManager::s_baseAddr);
			auto ReadOffsetData = [](uintptr_t location, SInt32 relOffset, UInt32 len)->uintptr_t
			{
				SInt32 rel32 = 0;
				RVAUtils::ReadMemory(location + relOffset, &rel32, sizeof(SInt32));
				_MESSAGE("(%016I64X) patched sucessfully...", location + len + rel32 - RelocationManager::s_baseAddr);
				return location + len + rel32;
			};
			pMain = reinterpret_cast<uintptr_t**>(ReadOffsetData(location, 3, 7));

			//static const RelocAddr <uintptr_t> InitCicero_Ent = 0x1D1641D; //V1.10.26

			struct InitCicero_Code : Xbyak::CodeGenerator
			{
				InitCicero_Code(void * buf, uintptr_t funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label funcLabel;
					Xbyak::Label retnLabel;

					mov(rcx, rax);
					call(ptr[rip + funcLabel]);
					jmp(ptr[rip + retnLabel]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel);
					dq(location + 0xB);
				}
			};

			void * codeBuf = g_localTrampoline.StartAlloc();
			InitCicero_Code code(codeBuf, (uintptr_t)InitCicero);
			g_localTrampoline.EndAlloc(code.getCurr());

			g_branchTrampoline.Write6Branch(location, uintptr_t(code.getCode()));
		}
	}
};

LPTOP_LEVEL_EXCEPTION_FILTER	MainEx::fnExceptionfilter = nullptr;


void InitHooks()
{
	try
	{
		uintptr_t location = 0;
		location = RVAScan<uintptr_t>("C6 05 ? ? ? ? ? 0F B6 43 48", 0, 2, 7).GetUIntPtr(); //0x294BB3E V1.10.26
		_MESSAGE("(%016I64X) patched sucessfully...", location - RelocationManager::s_baseAddr);
		isInputEnabled = reinterpret_cast<bool*>(location);

		location = RVAScan<uintptr_t>("48 8B 0D ? ? ? ? E8 ? ? ? ? 4C 8B C7 48 8B D3 48 8B C8", 0x15, 1, 5).GetUIntPtr();
		_MESSAGE("(%016I64X) patched sucessfully...", location - RelocationManager::s_baseAddr);
		ProcessCharEvent = reinterpret_cast<_ProcessCharEvent>(location);

		location = RVAScan<uintptr_t>("FF 15 ? ? ? ? 8B 85 ? ? ? ? 3D ? ? ? ? 76 0C", 0, 2, 6).GetUIntPtr();
		_MESSAGE("(%016I64X) patched sucessfully...", location - RelocationManager::s_baseAddr);
		GetInputRawData_Original = HookUtil::SafeWrite64(location, GetInputRawData_Hook);

		location = RVAScan<uintptr_t>("48 8B 0D ? ? ? ? 84 DB 48 8B 5C 24 ?", 0, 3, 7).GetUIntPtr();
		_MESSAGE("(%016I64X) patched sucessfully...", location - RelocationManager::s_baseAddr);
		g_inputMgr = location - RelocationManager::s_baseAddr;

		//48 8B 0D ? ? ? ? 41 B0 01 41 8B D5
		location = RVAScan<uintptr_t>("48 8B 0D ? ? ? ? 41 B0 01 41 8B D5", 0, 3, 7).GetUIntPtr();
		_MESSAGE("(%016I64X) patched sucessfully...", location - RelocationManager::s_baseAddr); //0x05A976B8
		g_inputDeviceMgr = location - RelocationManager::s_baseAddr;


		//skip LookMenu's SpecialNameCharacter check.
		location = RVAScan<uintptr_t>("74 4A 48 8B 0D ? ? ? ? 40 88 7C 24 ?").GetUIntPtr();
		_MESSAGE("(%016I64X) patched sucessfully...", location - RelocationManager::s_baseAddr);
		SafeWrite8(location, 0xEB); //V1.10.26

		DX11::InitHook();
		//InputManagerEx::InitHook();
		//GFxMovieRootEx::InitHook();
		MainEx::InitHook();

#ifdef DEBUG
		//VirtualMachineEx::InitHook();
		//disalbe game's exception filter;
		location = RVAScan<uintptr_t>("48 8D 0D ? ? ? ? F3 0F 10 05 ? ? ? ? F3 0F 11 05 ? ? ? ? 48 89 05 ? ? ? ?", 0, 3, 7).GetUIntPtr() + 1;
		_MESSAGE("(%016I64X) patched sucessfully...", location - RelocationManager::s_baseAddr);
		SafeWrite8(location, 0); //v1.10.26

		location = RVAScan<uintptr_t>("FF 15 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? E8 ? ? ? ?").GetUIntPtr();
		_MESSAGE("(%016I64X) patched sucessfully...", location - RelocationManager::s_baseAddr);
		UInt8 nops[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
		SafeWriteBuf(location, nops, 6); //V1.10.26
#endif
	}
	catch (...)
	{
		MessageBox(nullptr, "Failed to initialize, please deactive this plugin and install newer version.", "ChineseInput", MB_OK);
	}

}


