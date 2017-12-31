#include "Cicero.h"
#include "InputMenu.h"
#include "F4SE/GameAPI.h"

#include <memory>
#include <windows.h>

#define SAFE_RELEASE(p)                             \
{                                                   \
    if (p) {                                        \
        (p)->Release();								\
        (p) = 0;                                    \
	    }                                           \
}

//==========================================================
//					CiceroInputMethod
//==========================================================


CiceroInputMethod* CiceroInputMethod::GetSingleton()
{
	static CiceroInputMethod instance;
	return &instance;
}

CiceroInputMethod::CiceroInputMethod() : m_refCount(1), m_ciceroState(false), m_pProfileMgr(nullptr), m_pProfiles(nullptr), m_pThreadMgrEx(nullptr), m_pThreadMgr(nullptr), m_pBaseContext(nullptr)
{
	m_uiElementSinkCookie = m_inputProfileSinkCookie = m_threadMgrEventSinkCookie = m_textEditSinkCookie = TF_INVALID_COOKIE;
}

CiceroInputMethod::~CiceroInputMethod()
{

}

STDAPI_(ULONG) CiceroInputMethod::AddRef()
{
	return ++m_refCount;
}

STDAPI_(ULONG) CiceroInputMethod::Release()
{
	LONG result = --m_refCount;
	if (result == 0)
		delete this;
	return result;
}

BOOL CiceroInputMethod::SetupSinks()
{
	HRESULT hr;
	hr = CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, __uuidof(ITfThreadMgrEx), (void**)&m_pThreadMgrEx);
	if (FAILED(hr) || FAILED(m_pThreadMgrEx->ActivateEx(&m_clientID, TF_TMAE_UIELEMENTENABLEDONLY)))
		return FALSE;
	ITfSource* source = nullptr;
	if (SUCCEEDED(hr = m_pThreadMgrEx->QueryInterface(__uuidof(ITfSource), (void **)&source)))
	{
		source->AdviseSink(__uuidof(ITfUIElementSink), (ITfUIElementSink*)this, &m_uiElementSinkCookie);
		source->AdviseSink(__uuidof(ITfInputProcessorProfileActivationSink), (ITfInputProcessorProfileActivationSink*)this, &m_inputProfileSinkCookie);
		source->AdviseSink(__uuidof(ITfThreadMgrEventSink), (ITfThreadMgrEventSink*)this, &m_threadMgrEventSinkCookie);

		source->Release();
	}
	if (FAILED(CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfiles, (LPVOID*)&m_pProfiles)))
		return FALSE;
	m_pProfiles->QueryInterface(IID_ITfInputProcessorProfileMgr, (void **)&m_pProfileMgr);

	this->GetCurrentInputMethodName();

	_MESSAGE("[CI] Enable cicero input...");

	return TRUE;
}

void CiceroInputMethod::ReleaseSinks()
{
	ITfSource* source = nullptr;
	if (m_textEditSinkCookie != TF_INVALID_COOKIE)
	{
		if (m_pBaseContext && SUCCEEDED(m_pBaseContext->QueryInterface(&source)))
		{
			HRESULT hr = source->UnadviseSink(m_textEditSinkCookie);
			SAFE_RELEASE(source);
			SAFE_RELEASE(m_pBaseContext);
			m_textEditSinkCookie = TF_INVALID_COOKIE;
		}
	}
	if (m_pThreadMgrEx && SUCCEEDED(m_pThreadMgrEx->QueryInterface(__uuidof(ITfSource), (void**)&source)))
	{
		source->UnadviseSink(m_uiElementSinkCookie);
		source->UnadviseSink(m_inputProfileSinkCookie);
		source->UnadviseSink(m_threadMgrEventSinkCookie);
		m_pThreadMgrEx->Deactivate();
		SAFE_RELEASE(source);
		SAFE_RELEASE(m_pThreadMgr);
		SAFE_RELEASE(m_pProfileMgr);
		SAFE_RELEASE(m_pProfiles);
		SAFE_RELEASE(m_pThreadMgrEx);
	}
	CoUninitialize();
}

STDAPI CiceroInputMethod::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
		return E_INVALIDARG;
	*ppvObj = nullptr;
	if (IsEqualIID(riid, IID_IUnknown))
		*ppvObj = reinterpret_cast<IUnknown*>(this);
	else if (IsEqualIID(riid, IID_ITfUIElementSink))
		*ppvObj = (ITfUIElementSink *)this;
	else if (IsEqualIID(riid, IID_ITfInputProcessorProfileActivationSink))
		*ppvObj = (ITfInputProcessorProfileActivationSink*)this;
	else if (IsEqualIID(riid, IID_ITfThreadMgrEventSink))
		*ppvObj = (ITfThreadMgrEventSink*)this;
	else if (IsEqualIID(riid, IID_ITfTextEditSink))
		*ppvObj = (ITfTextEditSink*)this;
	else
		return E_NOINTERFACE;
	AddRef();
	return S_OK;
}

STDAPI CiceroInputMethod::OnActivated(DWORD dwProfileType, LANGID langid, REFCLSID clsid, REFGUID catid, REFGUID guidProfile, HKL hkl, DWORD dwFlags)
{
	if (!(dwFlags & TF_IPSINK_FLAG_ACTIVE))
		return S_OK;
	this->GetCurrentInputMethodName();

	if (dwProfileType & TF_PROFILETYPE_INPUTPROCESSOR)
	{
		m_ciceroState = true;
	}
	else if (dwProfileType & TF_PROFILETYPE_KEYBOARDLAYOUT)
	{
		m_ciceroState = false;
	}
	else
	{
		m_ciceroState = false;
	}
	return S_OK;
}


STDAPI CiceroInputMethod::BeginUIElement(DWORD dwUIElementId, BOOL *pbShow)
{
	*pbShow = FALSE;
	InputMenu* mm = InputMenu::GetSingleton();
	InterlockedExchange64(&mm->enableState, 1);
	ITfUIElement* pElement = GetUIElement(dwUIElementId);
	if (pElement != nullptr)
	{
		ITfCandidateListUIElement* lpCandidate = nullptr;
		if (SUCCEEDED(pElement->QueryInterface(__uuidof(ITfCandidateListUIElement), (void**)&lpCandidate)))
		{
			this->GetCandidateStrings(lpCandidate);
			lpCandidate->Release();
		}
		pElement->Release();
	}
	return S_OK;
}

STDAPI CiceroInputMethod::UpdateUIElement(DWORD dwUIElementId)
{
	ITfUIElement* pElement = GetUIElement(dwUIElementId);
	if (pElement != nullptr)
	{
		ITfCandidateListUIElement* lpCandidate = nullptr;
		if (SUCCEEDED(pElement->QueryInterface(__uuidof(ITfCandidateListUIElement), (void**)&lpCandidate)))
		{
			GetCandidateStrings(lpCandidate);
			lpCandidate->Release();
		}
		pElement->Release();
	}
	return S_OK;
}

STDAPI CiceroInputMethod::EndUIElement(DWORD dwUIElementId)
{
	ITfUIElement* pElement = GetUIElement(dwUIElementId);
	if (pElement != nullptr)
	{
		ITfCandidateListUIElement* lpCandidate = nullptr;
		if (SUCCEEDED(pElement->QueryInterface(__uuidof(ITfCandidateListUIElement), (void**)&lpCandidate)))
		{
			lpCandidate->Release();
		}
		pElement->Release();
	}
	return S_OK;
}


STDAPI CiceroInputMethod::OnInitDocumentMgr(ITfDocumentMgr *pdim)
{
	return S_OK;
}

STDAPI CiceroInputMethod::OnUninitDocumentMgr(ITfDocumentMgr *pdim)
{
	return S_OK;
}

STDAPI CiceroInputMethod::OnSetFocus(ITfDocumentMgr *pdimFocus, ITfDocumentMgr *pdimPrevFocus)
{
	if (!pdimFocus)
		return S_OK;
	ITfSource* source = nullptr;
	if (m_textEditSinkCookie != TF_INVALID_COOKIE)
	{
		if (m_pBaseContext && SUCCEEDED(m_pBaseContext->QueryInterface(&source)))
		{
			HRESULT hr = source->UnadviseSink(m_textEditSinkCookie);
			SAFE_RELEASE(source);
			if (FAILED(hr))
				return S_OK;
			SAFE_RELEASE(m_pBaseContext);
			m_textEditSinkCookie = TF_INVALID_COOKIE;
		}
	}
	if (SUCCEEDED(pdimFocus->GetBase(&m_pBaseContext)) && SUCCEEDED(m_pBaseContext->QueryInterface(&source)) && SUCCEEDED(source->AdviseSink(IID_ITfTextEditSink, static_cast<ITfTextEditSink *>(this), &m_textEditSinkCookie))) {}
	SAFE_RELEASE(source);
	return S_OK;
}

STDAPI CiceroInputMethod::OnPushContext(ITfContext *pic)
{
	return S_OK;
}
STDAPI CiceroInputMethod::OnPopContext(ITfContext *pic)
{
	return S_OK;
}

STDAPI CiceroInputMethod::OnEndEdit(ITfContext *cxt, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)//Get composition string...
{
	ITfContextComposition* pContextComposition;
	if (FAILED(cxt->QueryInterface(&pContextComposition)))
		return S_OK;
	IEnumITfCompositionView *pEnumComposition;
	if (FAILED(pContextComposition->EnumCompositions(&pEnumComposition)))
	{
		pContextComposition->Release();
		return S_OK;
	}
	ITfCompositionView* pCompositionView = nullptr;
	std::string result;
	ULONG fetchCount = NULL;
	while (SUCCEEDED(pEnumComposition->Next(1, &pCompositionView, &fetchCount)) && fetchCount > NULL)
	{
		ITfRange* pRange;
		WCHAR buffer[MAX_PATH];
		ULONG bufferSize = NULL;
		if (SUCCEEDED(pCompositionView->GetRange(&pRange)))
		{
			pRange->GetText(ecReadOnly, TF_TF_MOVESTART, buffer, MAX_PATH, &bufferSize);
			buffer[bufferSize] = '\0';
			auto menu = InputMenu::GetSingleton();
			g_criticalSection.Enter();
			menu->compositionContent = buffer;
			g_criticalSection.Leave();
			pRange->Release();
		}
		pCompositionView->Release();
	}
	pEnumComposition->Release();
	pContextComposition->Release();
	return S_OK;
}


ITfUIElement* CiceroInputMethod::GetUIElement(DWORD dwUIElementId)
{
	ITfUIElementMgr* pElementMgr = nullptr;
	ITfUIElement* pElement = nullptr;
	if (SUCCEEDED(m_pThreadMgrEx->QueryInterface(__uuidof(ITfUIElementMgr), (void**)&pElementMgr)))
	{
		pElementMgr->GetUIElement(dwUIElementId, &pElement);
		pElementMgr->Release();
	}
	return pElement;
}

void CiceroInputMethod::GetCandidateStrings(ITfCandidateListUIElement* lpCandidate)
{
	InputMenu* mm = InputMenu::GetSingleton();

	if (InterlockedCompareExchange64(&mm->enableState, 1, 1))
	{
		UINT uIndex = 0, uCount = 0, uCurrentPage = 0, uPageCount = 0;
		DWORD dwPageStart = 0, dwPageSize = 0, dwPageSelection = 0;

		WCHAR* result = nullptr;

		if (lpCandidate->GetSelection(&uIndex) == S_FALSE)
		{
			uIndex = -1;
		}
		lpCandidate->GetCount(&uCount);
		lpCandidate->GetCurrentPage(&uCurrentPage);

		dwPageSelection = static_cast<DWORD>(uIndex);
		lpCandidate->GetPageIndex(nullptr, 0, &uPageCount);
		if (uPageCount > 0)
		{
			std::unique_ptr<UINT[]> indexList(reinterpret_cast<UINT*>(new char[sizeof(UINT) * uPageCount]));
			lpCandidate->GetPageIndex(indexList.get(), uPageCount, &uPageCount);
			dwPageStart = indexList[uCurrentPage];
			dwPageSize = (uCurrentPage < uPageCount - 1) ? min(uCount, indexList[uCurrentPage + 1]) - dwPageStart : uCount - dwPageStart;
		}

		dwPageSize = min(dwPageSize, MAX_CANDLIST);
		if (dwPageSize)
			InterlockedExchange64(&mm->shieldKeyState, 1);

		wchar_t sPrefix[8];

		InterlockedExchange64(&mm->selectedIndex, dwPageSelection);
		InterlockedExchange64(&mm->pageStartIndex, dwPageStart);

		g_criticalSection.Enter();
		mm->candidataContent.clear();
		for (int i = 0; i < dwPageSize; i++)
		{
			if (SUCCEEDED(lpCandidate->GetString(i + dwPageStart, &result)))
			{
				if (result != nullptr)
				{
					wsprintfW(sPrefix, L"%d.", i + 1);
					std::wstring sContent(sPrefix);
					sContent += result;
					mm->candidataContent.push_back(sContent);
					SysFreeString(result);
				}
			}
		}
		g_criticalSection.Leave();
	}

}

bool CiceroInputMethod::DisableInputMethod(LANGID langID = CHS_KB)
{
	bool result = false;
	GUID clsid = GUID_NULL;
	HRESULT hr = m_pProfiles->ActivateLanguageProfile(clsid, langID, clsid);
	if (SUCCEEDED(hr))
		result = true;
	return result;
}

bool CiceroInputMethod::GetLayoutName(const WCHAR* kl, WCHAR* nm)
{
	HKEY pKey;
	static wchar_t sData[64];
	DWORD iSize;
	wchar_t sKeyPath[200];
	wsprintfW(sKeyPath, L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts\\%s", kl);
	long result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, sKeyPath, 0, KEY_QUERY_VALUE, &pKey);
	if (result == ERROR_SUCCESS)
	{
		iSize = sizeof(sData);
		result = RegQueryValueExW(pKey, L"Layout Text", NULL, NULL, (LPBYTE)sData, &iSize);
	}
	RegCloseKey(pKey);
	if (result == ERROR_SUCCESS && wcslen(nm) < 64)
	{
		wcscpy_s(nm, 64, sData);
		return true;
	}
	return false;
}

void CiceroInputMethod::GetCurrentInputMethodName()
{
	static wchar_t lastTipName[64];
	ZeroMemory(lastTipName, sizeof(lastTipName));

	TF_INPUTPROCESSORPROFILE inputProfile;
	m_pProfileMgr->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, &inputProfile);
	if (inputProfile.dwProfileType == TF_PROFILETYPE_INPUTPROCESSOR)
	{
		wchar_t* sName = nullptr;
		m_pProfiles->GetLanguageProfileDescription(inputProfile.clsid, inputProfile.langid, inputProfile.guidProfile, &sName);
		if (sName != nullptr && wcslen(sName) < 64)
		{
			wcscpy_s(lastTipName, 64, sName);
		}
		if (sName != nullptr)	SysFreeString(sName);
	}
	else if (inputProfile.dwProfileType == TF_PROFILETYPE_KEYBOARDLAYOUT)
	{
		static wchar_t sLayoutName[KL_NAMELENGTH];
		if (GetKeyboardLayoutNameW(sLayoutName))
		{
			GetLayoutName(sLayoutName, lastTipName);
		}
	}
	std::wstring sInputName = lastTipName, sCurrentState = L"\u72b6\u6001: ";
	size_t pos = sInputName.find(L"-", 0);
	if (pos == std::wstring::npos)
		sCurrentState.append(sInputName);
	else
		sCurrentState.append(sInputName, pos + 2, sInputName.size());

	auto menu = InputMenu::GetSingleton();
	g_criticalSection.Enter();
	menu->inputStateInfo = sCurrentState;
	g_criticalSection.Leave();
} 