#pragma once

namespace InputUtil
{
#ifdef UNICODE
	void SendUnicodeMessage(uintptr_t wCharCode);
	void GetResultString(const HWND& hWnd);
	void GetInputStringW(const HWND& hWnd);
	void GetInputStringA(const HWND& hWnd);
#else
	void SendUnicodeMessage(uintptr_t wCharCode);
	void GetCompositionStringW(const HWND& hWnd);
	void GetResultString(const HWND& hWnd);
	void GetCandidateListW(const HWND& hWnd);
#endif // 
}