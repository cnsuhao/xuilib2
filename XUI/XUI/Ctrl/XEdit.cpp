#include "StdAfx.h"
#include "XEdit.h"

#include "XCaret.h"

#include "..\..\..\XLib\inc\interfaceS\string\StringCode.h"

#define EDIT_H_SCROLL_BAR_WIDTH 0

#define TEXT_SERVICE_DLL  _T("Msftedit.dll")

X_IMPLEMENT_FRAME_XML_RUNTIME(CXEdit)

CXFrame * CXEdit::CreateFrameFromXML(X_XML_NODE_TYPE xml, CXFrame *pParent)
{
	if (!xml)
		return NULL;

	X_XML_ATTR_TYPE attr = NULL;
	BOOL bTransparent = FALSE;
	BOOL bMultiline = FALSE;
	BOOL bPasswordMode = FALSE;

	attr = xml->first_attribute("transparent", 0, false);
	if (attr && !StrCmpIA(attr->value(), "true")) bTransparent = TRUE;
	attr = xml->first_attribute("multiline", 0, false);
	if (attr && !StrCmpIA(attr->value(), "true")) bMultiline = TRUE;
	attr = xml->first_attribute("password", 0, false);
	if (attr && !StrCmpIA(attr->value(), "true")) bPasswordMode = TRUE;

	LayoutParam *pLayout = pParent ?
		pParent->GenerateLayoutParam(xml) : new CXFrame::LayoutParam(xml);
	if (!pLayout)
	{
		CStringA strError;
		strError.Format("WARNING: Generating the layout parameter for the parent %s failed. \
						Building the frame failed. ", XLibST2A(pParent->GetName()));
		CXFrameXMLFactory::ReportError(strError);
		return NULL;
	}

	CXEdit *pEdit = new CXEdit();
	pEdit->Create(pParent, pLayout, VISIBILITY_NONE,
		bTransparent, bMultiline, bPasswordMode);

	return pEdit;
}

CXEdit::CXEdit(void)
	: m_nRefCount(1),
	m_pTextService(NULL),
	m_bMutiline(FALSE),
	m_bPasswordMode(FALSE),
	m_bTransparent(FALSE),
	m_ptOldDCViewPort(0, 0)
{
}

HRESULT STDMETHODCALLTYPE CXEdit::QueryInterface( /* [in] */ REFIID riid, /* [iid_is][out] */
												 __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject )
{
	const IID * pTextHostIID = GetITextHostIID();

	if (!ppvObject)
		return E_INVALIDARG;

	*ppvObject = NULL;

	if (IsEqualIID(riid, IID_IUnknown) || (pTextHostIID && IsEqualIID(riid, *pTextHostIID)))
	{
		AddRef();
		*ppvObject = (ITextHost *)this;
		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CXEdit::AddRef( void )
{
	return ++m_nRefCount;
}

ULONG STDMETHODCALLTYPE CXEdit::Release( void )
{
	return --m_nRefCount;
}

VOID CXEdit::Destroy()
{
	m_pCaret = NULL;

	if (m_pTextService)
	{
		m_pTextService->Release();
		m_pTextService = NULL;
	}

	ResetCharFormat();
	ResetParaFormat();

	SetTransparent(TRUE);
	SetMultiline(FALSE);
	SetPasswordMode(FALSE);

	m_ptOldDCViewPort.x = m_ptOldDCViewPort.y = 0;

	__super::Destroy();
}

HDC CXEdit::TxGetDC()
{
	// Note that all the coordinates of ITextService are
	// base on the window coordinate system. So that IMEs will be
	// on its right place. 

	if (GetVisibility() != VISIBILITY_SHOW)
		return GetEmptyDC();

	HDC dc = GetDC();

	if (dc)
		::SetViewportOrgEx(dc, 0, 0, &m_ptOldDCViewPort);

	return dc;
}

INT CXEdit::TxReleaseDC( HDC hdc )
{
	if (hdc == GetEmptyDC())
		return 1; // SUCCESS;

	if (hdc)
		::SetViewportOrgEx(hdc, m_ptOldDCViewPort.x, m_ptOldDCViewPort.y, NULL);

	if (ReleaseDC(hdc))
		return 1; // SUCCESS

	return 0;
}

BOOL CXEdit::TxShowScrollBar( INT fnBar, BOOL fShow )
{
	// TODO: 

	return TRUE;
}

BOOL CXEdit::TxEnableScrollBar( INT fuSBFlags, INT fuArrowflags )
{
	// TODO: 

	return TRUE;
}

BOOL CXEdit::TxSetScrollRange( INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw )
{
	// TODO: 

	return TRUE;

}

BOOL CXEdit::TxSetScrollPos( INT fnBar, INT nPos, BOOL fRedraw )
{
	// TODO: 

	return TRUE;
}

void CXEdit::TxInvalidateRect( LPCRECT prc, BOOL fMode )
{
	// Note that all the coordinates of ITextService are
	// base on the window coordinate system. So that IMEs will be
	// on its right place. 

	if (!prc)
		InvalidateRect();

	InvalidateRect(RootFrameToThisFrame(prc));
}

void CXEdit::TxViewChange( BOOL fUpdate )
{
	if (fUpdate)
		Update();
}

BOOL CXEdit::TxShowCaret( BOOL fShow )
{
	if (m_pCaret)
		return m_pCaret->SetVisibility(fShow ? VISIBILITY_SHOW : VISIBILITY_HIDE);

	return FALSE;
}

BOOL CXEdit::TxCreateCaret( HBITMAP hbmp, INT xWidth, INT yHeight )
{
	if (m_pCaret)
		return m_pCaret->SetCaretShape(hbmp, xWidth, yHeight);


	return FALSE;
}

INT CXEdit::GetTextHostWidth()
{
	return max(0, GetRect().Width() - EDIT_H_SCROLL_BAR_WIDTH);
}

INT CXEdit::GetTextHostHeight()
{
	return max(0, GetRect().Height());
}

BOOL CXEdit::TxSetCaretPos( INT x, INT y )
{
	CPoint pt;
	GetCaretPos(&pt);

	if (m_pCaret)
	{
		CPoint ptPos = RootFrameToThisFrame(CPoint(x, y));

		CRect rc (m_pCaret->GetRect());
		rc.MoveToXY(ptPos.x, ptPos.y);
		m_pCaret->SetRect(rc);

		return TRUE;
	}

	return FALSE;
}

BOOL CXEdit::TxSetTimer( UINT idTimer, UINT uTimeout )
{
	TxKillTimer(idTimer);

	UINT nFrameTimerId = SetTimer(uTimeout);
	if (!nFrameTimerId)
		return FALSE;

	m_mapFrameTimerIDToHostTimerID.insert(std::make_pair(nFrameTimerId, idTimer));

	return TRUE;
}

void CXEdit::TxKillTimer( UINT idTimer )
{
	for (std::map<UINT, UINT>::const_iterator it = m_mapFrameTimerIDToHostTimerID.begin();
		it != m_mapFrameTimerIDToHostTimerID.end(); it++)
	{
		if (it->second == idTimer)
		{
			KillTimer(it->first);
			m_mapFrameTimerIDToHostTimerID.erase(it);
			break;
		}
	}
}

LRESULT CXEdit::OnFrameTimer( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	bHandled = TRUE;

	std::map<UINT, UINT>::const_iterator it = m_mapFrameTimerIDToHostTimerID.find(wParam);
	if (it == m_mapFrameTimerIDToHostTimerID.end())
		return -1;

	LRESULT lResult = 0;

	if (m_pTextService)
		m_pTextService->TxSendMessage(uMsg, it->second, 0, &lResult);

	return 0;
}

void CXEdit::TxScrollWindowEx( INT dx, INT dy, LPCRECT lprcScroll, 
							  LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll )
{
	InvalidateRect();
}

void CXEdit::TxSetCapture( BOOL fCapture )
{
	CXFrameMsgMgr *pMgr = GetFrameMsgMgr();

	if (!pMgr)
		return;

	if (fCapture)
		pMgr->CaptureMouse(this);
	else
		pMgr->ReleaseCaptureMouse(this);
}

void CXEdit::TxSetFocus()
{
	CXFrameMsgMgr *pMgr = GetFrameMsgMgr();

	if (!pMgr)
		return;

	pMgr->GetFocus(this);
}

void CXEdit::TxSetCursor( HCURSOR hcur, BOOL fText )
{
	::SetCursor(hcur);
}

BOOL CXEdit::TxScreenToClient( LPPOINT lppt )
{
	// Note that all the coordinates of ITextService are
	// base on the window coordinate system. So that IMEs will be
	// on its right place. 

	if (!lppt)
		return FALSE;

	CPoint pt = GetRootFrame()->ScreenToFrame(CPoint(lppt->x, lppt->y));
	*lppt = pt;

	return TRUE;
}

BOOL CXEdit::TxClientToScreen( LPPOINT lppt )
{
	// Note that all the coordinates of ITextService are
	// base on the window coordinate system. So that IMEs will be
	// on its right place. 

	if (!lppt)
		return FALSE;

	CPoint pt = GetRootFrame()->FrameToScreen(CPoint(lppt->x, lppt->y));
	*lppt = pt;

	return TRUE;
}

HRESULT CXEdit::TxActivate( LONG * plOldState )
{
	return S_OK;
}

HRESULT CXEdit::TxDeactivate( LONG lNewState )
{
	return S_OK;
}

HRESULT CXEdit::TxGetClientRect( LPRECT prc )
{
	// Note that all the coordinates of ITextService are
	// base on the window coordinate system. So that IMEs will be
	// on its right place. 

	if (!prc)
		return E_INVALIDARG;

	prc->left = prc->top = 0;
	prc->right = GetTextHostWidth();
	prc->bottom = GetTextHostHeight();

	*prc = ThisFrameToRootFrame(prc);

	return S_OK;
}

HRESULT CXEdit::TxGetViewInset( LPRECT prc )
{
	if (!prc)
		return E_INVALIDARG;

	prc->left = prc->right = prc->top = prc->bottom = 0;

	return S_OK;
}

HRESULT CXEdit::TxGetCharFormat( const CHARFORMATW **ppCF )
{
	if (!ppCF)
		return E_INVALIDARG;
	
	*ppCF = &m_CharFormat;
	return S_OK;
}


HRESULT CXEdit::TxGetParaFormat( const PARAFORMAT **ppPF )
{	
	if (!ppPF)
		return E_INVALIDARG;

	*ppPF = &m_ParaFormat;
	return S_OK;
}


VOID CXEdit::ResetCharFormat()
{
	ZeroMemory(&m_CharFormat, sizeof(m_CharFormat));

	m_CharFormat.cbSize = sizeof(m_CharFormat);
	m_CharFormat.dwEffects = 0;
	m_CharFormat.yHeight = 0;
	m_CharFormat.yOffset = 0;
	m_CharFormat.crTextColor = RGB(0, 0, 0);
	m_CharFormat.bCharSet = DEFAULT_CHARSET;
	m_CharFormat.bPitchAndFamily = 0;
	/*m_CharFromat.szFaceName;*/
	StrCpyW(m_CharFormat.szFaceName, L"Arial");
	m_CharFormat.dwMask = CFM_FACE | CFM_CHARSET | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;
}

VOID CXEdit::ResetParaFormat()
{
	ZeroMemory(&m_ParaFormat, sizeof(m_CharFormat));

	m_ParaFormat.cbSize = sizeof(m_CharFormat);
	m_ParaFormat.wNumbering = 0;
	m_ParaFormat.wEffects = 0;
	m_ParaFormat.dxStartIndent = 0;
	m_ParaFormat.dxRightIndent = 0;
	m_ParaFormat.dxOffset = 0;
	m_ParaFormat.wAlignment = PFA_LEFT;
	m_ParaFormat.cTabCount = 0;
	/*m_ParaFormat.rgxTabs*/
	m_ParaFormat.dwMask = PFM_ALIGNMENT;
}

COLORREF CXEdit::TxGetSysColor( int nIndex )
{
	return ::GetSysColor(nIndex);
}

HRESULT CXEdit::TxGetBackStyle( TXTBACKSTYLE *pstyle )
{
	if (!pstyle)
		return E_INVALIDARG;

	*pstyle = m_bTransparent ? TXTBACK_TRANSPARENT : TXTBACK_OPAQUE;

	return S_OK;
}

HRESULT CXEdit::TxGetMaxLength( DWORD *plength )
{
	if (!plength)
		return E_INVALIDARG;

	*plength = INFINITE;

	return S_OK;
}

HRESULT CXEdit::TxGetScrollBars( DWORD *pdwScrollBar )
{
	if (!pdwScrollBar)
		return E_INVALIDARG;

	*pdwScrollBar = WS_VSCROLL | WS_HSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL;

	return S_OK;
}

HRESULT CXEdit::TxGetPasswordChar( TCHAR *pch )
{
	if (!pch)
		return E_INVALIDARG;

	*pch = _T('*');

	return S_OK;
}

HRESULT CXEdit::TxGetAcceleratorPos( LONG *pcp )
{
	if (!pcp)
		return E_INVALIDARG;

	*pcp = -1;

	return S_OK;
}

HRESULT CXEdit::TxGetExtent( LPSIZEL lpExtent )
{
	if (!lpExtent)
		return E_INVALIDARG;

	HDC dc = ::CreateCompatibleDC(NULL);

	lpExtent->cx = 
		(double)GetTextHostWidth()  / ::GetDeviceCaps(dc, LOGPIXELSX) * 2540;
	lpExtent->cy = 
		(double)GetTextHostHeight() /::GetDeviceCaps(dc, LOGPIXELSY) * 2540;

	::DeleteDC(dc);

	return S_OK;
}

HRESULT CXEdit::OnTxCharFormatChange( const CHARFORMATW * pcf )
{
	return S_OK;
}

HRESULT CXEdit::OnTxParaFormatChange( const PARAFORMAT * ppf )
{
	return S_OK;
}

HRESULT CXEdit::TxGetPropertyBits( DWORD dwMask, DWORD *pdwBits )
{
	if (!pdwBits)
		return E_INVALIDARG;

	*pdwBits = 0;

	if (m_bMutiline)
		*pdwBits |= TXTBIT_MULTILINE;
	if (m_bPasswordMode)
		*pdwBits |= TXTBIT_USEPASSWORD;
	*pdwBits |= TXTBIT_WORDWRAP;
	*pdwBits |= TXTBIT_SAVESELECTION;

	*pdwBits = *pdwBits & dwMask;

	return S_OK;
}

HRESULT CXEdit::TxNotify( DWORD iNotify, void *pv )
{
	return S_OK;
}

HIMC CXEdit::TxImmGetContext()
{
 	HWND hWnd = GetHWND();
	if (!hWnd)
		return NULL;

	return ::ImmGetContext(hWnd);
}

void CXEdit::TxImmReleaseContext( HIMC himc )
{
	HWND hWnd = GetHWND();
	if (!hWnd)
		return;

	::ImmReleaseContext(hWnd, himc);
}

HRESULT CXEdit::TxGetSelectionBarWidth( LONG *lSelBarWidth )
{
	if (!lSelBarWidth)
		return E_INVALIDARG;

	*lSelBarWidth = 0;

	return S_OK;
}

BOOL CXEdit::PaintForeground( HDC hDC, const CRect &rcUpdate )
{
	if (rcUpdate.IsRectEmpty())
		return TRUE;

	CRect rcTextHost(0, 0, GetTextHostWidth(), GetTextHostHeight());
	RECTL rclTextHostOnBmpBuffer = {rcTextHost.left - rcUpdate.left, rcTextHost.top - rcUpdate.top, 
		rcTextHost.right - rcUpdate.left, rcTextHost.bottom - rcUpdate.top};


	BYTE *pBmpBufferByte = NULL;
	HBITMAP hBmpBuffer = Util::CreateDIBSection32(rcUpdate.Width(), rcUpdate.Height(), &pBmpBufferByte);
	if (pBmpBufferByte)
	{
		const UINT nBmpBufferSize = rcUpdate.Width() * rcUpdate.Height() * 4;
		FillMemory(pBmpBufferByte, nBmpBufferSize, 0xCD);
		HDC dcBmpBuffer = ::CreateCompatibleDC(hDC);
		HGDIOBJ hOldBmp = ::SelectObject(dcBmpBuffer, (HGDIOBJ)hBmpBuffer);

		CRect rcTextHostUpdateOnBmpBuffer(0, 0, 0, 0);
		rcTextHostUpdateOnBmpBuffer.IntersectRect(rcTextHost, rcUpdate);
		rcTextHostUpdateOnBmpBuffer.OffsetRect(-rcUpdate.left, -rcUpdate.top);

		if (m_pTextService)
			m_pTextService->TxDraw(DVASPECT_CONTENT, 0, NULL, NULL,
			dcBmpBuffer, NULL, &rclTextHostOnBmpBuffer, NULL, &rcTextHostUpdateOnBmpBuffer, NULL, 0, 0);

		for (UINT i = 0; i < nBmpBufferSize; i += 4)
		{
 			if (pBmpBufferByte[i + 3] == 0xCD)
 				ZeroMemory(pBmpBufferByte + i, 4);
 			else if (pBmpBufferByte[i + 3] != 0xCD)
 				pBmpBufferByte[i + 3] = 0xFF;
		}

		Util::AlaphaBlend(dcBmpBuffer, CRect(0, 0, rcUpdate.Width(), rcUpdate.Height()), hDC, rcUpdate);

		::SelectObject(dcBmpBuffer, hOldBmp);
		::DeleteObject((HGDIOBJ)dcBmpBuffer);
		::DeleteDC(dcBmpBuffer);
	}

	return __super::PaintForeground(hDC, rcUpdate);
}

BOOL CXEdit::Create( CXFrame * pFrameParent, LayoutParam * pLayout,  VISIBILITY visibility/* = VISIBILITY_NONE*/, 
					BOOL bTransparent /*= FALSE*/, BOOL bMultiline /*= FALSE*/, BOOL bPasswordMode /*= FALSE*/)
{
	ATLASSERT(m_pTextService == NULL);

	ResetCharFormat();
	ResetParaFormat();

	SetTransparent(bTransparent);
	SetMultiline(bMultiline);
	SetPasswordMode(bPasswordMode);

	IUnknown *p = NULL;
	if (FAILED(::CreateTextServices(NULL, this, &p)))
		return FALSE;
	const IID *pTextServiceIID = GetITextServiceIID();
	BOOL bRtn = pTextServiceIID && SUCCEEDED(p->QueryInterface(*pTextServiceIID, (void **)&m_pTextService)) && m_pTextService;

	p->Release();

	if (bRtn)
	{
		bRtn = __super::Create(pFrameParent, pLayout, visibility);

		if (bRtn)
		{
			m_pCaret = new CXCaret();
			m_pCaret->Create(this, NULL);
		}
	}

	return bRtn;
}

const IID * CXEdit::GetITextServiceIID()
{
	static IID * pRtn = NULL;
	static IID aIID;
	static BOOL bInit = FALSE;

	if (bInit)
		return pRtn;

	bInit = TRUE;

	HMODULE hDLL = ::LoadLibrary(TEXT_SERVICE_DLL);
	if (!hDLL)
		return NULL;
	
	const IID *pIID = (const IID *)GetProcAddress(hDLL, "IID_ITextServices");
	if (pIID)
	{
		aIID = *pIID;
		pRtn = &aIID;
	}

	::FreeLibrary(hDLL);

	return pRtn;
}

const IID * CXEdit::GetITextHostIID()
{
	static IID * pRtn = NULL;
	static IID aIID;
	static BOOL bInit = FALSE;

	if (bInit)
		return pRtn;

	bInit = TRUE;

	HMODULE hDLL = ::LoadLibrary(TEXT_SERVICE_DLL);
	if (!hDLL)
		return NULL;

	const IID *pIID = (const IID *)GetProcAddress(hDLL, "IID_ITextHost");
	if (pIID)
	{
		aIID = *pIID;
		pRtn = &aIID;
	}

	::FreeLibrary(hDLL);

	return pRtn;
}

BOOL CXEdit::SetTransparent( BOOL b )
{
	if ((m_bTransparent && b)||
		(!m_bTransparent && !b))
		return TRUE;

	m_bTransparent = b;

	if (m_pTextService)
		m_pTextService->OnTxPropertyBitsChange(TXTBIT_BACKSTYLECHANGE, 
			m_bTransparent ?  TXTBACK_TRANSPARENT : TXTBACK_OPAQUE);
	
	return TRUE;
}

BOOL CXEdit::SetFontSize( UINT nPixel )
{
 	HDC dc = ::CreateCompatibleDC(NULL);
	LONG nFontSize = (double)nPixel * 1440 / ::GetDeviceCaps(dc, LOGPIXELSY);
 	::DeleteDC(dc);

	if ((m_CharFormat.dwMask & CFM_SIZE) && nFontSize == m_CharFormat.yHeight)
		return TRUE;

	m_CharFormat.dwMask |= CFM_SIZE;
	m_CharFormat.yHeight = nFontSize;
 
 	if (m_pTextService)
 		m_pTextService->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, 0);

	return TRUE;
}

BOOL CXEdit::SetFont( LPCTSTR pFontName )
{
	HDC dc = ::CreateCompatibleDC(NULL);

	CStringW strFontName = XLibST2W(pFontName);

	if ((m_CharFormat.dwMask & CFM_FACE) && !StrCmpIW(m_CharFormat.szFaceName, strFontName))
		return TRUE;

	m_CharFormat.dwMask |= CFM_FACE;
	StrCpyW(m_CharFormat.szFaceName, strFontName);

	if (m_pTextService)
		m_pTextService->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, 0);

	return TRUE;
}

BOOL CXEdit::SetFontStyle( DWORD dwEffect )
{
	if ((m_CharFormat.dwMask & CFM_BOLD) && 
		(m_CharFormat.dwMask & CFM_ITALIC) && 
		(m_CharFormat.dwMask & CFM_UNDERLINE) && 
		(m_CharFormat.dwMask & CFM_STRIKEOUT) &&
		m_CharFormat.dwEffects == dwEffect)
		return TRUE;

	m_CharFormat.dwMask |= CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;
	m_CharFormat.dwEffects = dwEffect;

	m_pTextService->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, 0);

	return TRUE;
}

LRESULT CXEdit::OnAllMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	LRESULT lResult = 0;

	if (uMsg == WM_KILLFOCUS && m_pCaret)
		m_pCaret->SetVisibility(VISIBILITY_HIDE);

	if (m_pTextService)
	{
		if (m_pTextService->TxSendMessage(uMsg, wParam, lParam, &lResult) == S_OK)
			bHandled = TRUE;
	}

	return lResult;
}

BOOL CXEdit::SetMultiline( BOOL b )
{
	if (m_bMutiline == b)
		return TRUE;

	m_bMutiline = b;

	if (m_pTextService)
		m_pTextService->OnTxPropertyBitsChange(TXTBIT_MULTILINE, TXTBIT_MULTILINE);

	return TRUE;
}

BOOL CXEdit::SetPasswordMode( BOOL b )
{
	if (m_bPasswordMode == b)
		return TRUE;

	m_bPasswordMode = b;

	if (m_pTextService)
		m_pTextService->OnTxPropertyBitsChange(TXTBIT_USEPASSWORD, TXTBIT_USEPASSWORD);

	return TRUE;
}

CRect CXEdit::ThisFrameToRootFrame(const CRect &rect)
{
	CPoint ptTopLeft(rect.TopLeft());
	CPoint ptRightBottom(rect.BottomRight());

	ptTopLeft = ThisFrameToRootFrame(ptTopLeft);
	ptRightBottom =	ThisFrameToRootFrame(ptRightBottom);

	return CRect(ptTopLeft.x, ptTopLeft.y, ptRightBottom.x, ptRightBottom.y);
}

CPoint CXEdit::ThisFrameToRootFrame( const CPoint &pt )
{
	CPoint ptRst(pt);

	CXFrame *pRootFrame = this;
	while (pRootFrame->GetParent())
	{
		ptRst = pRootFrame->ChildToParent(ptRst);
		pRootFrame = pRootFrame->GetParent();
	}

	return ptRst;
}

CRect CXEdit::RootFrameToThisFrame( const CRect &rect )
{
	CPoint ptTopLeft(rect.TopLeft());
	CPoint ptRightBottom(rect.BottomRight());

	ptTopLeft = RootFrameToThisFrame(ptTopLeft);
	ptRightBottom =	RootFrameToThisFrame(ptRightBottom);

	return CRect(ptTopLeft.x, ptTopLeft.y, ptRightBottom.x, ptRightBottom.y);

}

CPoint CXEdit::RootFrameToThisFrame( const CPoint &pt )
{
	CPoint ptDelta(0, 0);

	CXFrame *pRootFrame = this;
	while (pRootFrame->GetParent())
	{
		ptDelta = pRootFrame->ChildToParent(ptDelta);
		pRootFrame = pRootFrame->GetParent();
	}

	CPoint ptRst(pt);
	ptRst.Offset(-ptDelta.x, -ptDelta.y);
	
	return ptRst;
}

BOOL CXEdit::NeedPrepareMessageForThisFrame()
{
	return FALSE;
}

CXFrame * CXEdit::GetRootFrame()
{
	CXFrame *pRootFrame = this;
	while(pRootFrame->GetParent())
		pRootFrame = pRootFrame->GetParent();

	return pRootFrame;
}

VOID CXEdit::OnAttachedToParent( CXFrame *pParent )
{
	__super::OnAttachedToParent(pParent);

	while (pParent)
	{
		pParent->AddEventListener(this);
		pParent = pParent->GetParent();
	}

	if (GetHWND() && m_pTextService)
	{
		CRect rcClient(0, 0, 0, 0);
		TxGetClientRect(&rcClient);
		m_pTextService->OnTxInPlaceActivate(&rcClient);
	}
}

VOID CXEdit::OnDetachedFromParent()
{
	CXFrame *pCurrentParent = CXFrame::GetParent();
	ATLASSERT(pCurrentParent);

	while (pCurrentParent)
	{
		pCurrentParent->RemoveEventListener(this);
		pCurrentParent = pCurrentParent->GetParent();
	}

	if (m_pTextService)
		m_pTextService->OnTxInPlaceDeactivate();

	__super::OnDetachedFromParent();
}

VOID CXEdit::OnOneAncestorDetachedFromaParent(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (!IsAncester(pSrcFrame))
		return;

	CXFrame *pParentDetachedFrom = (CXFrame *)wParam;

	while (pParentDetachedFrom)
	{
		pParentDetachedFrom->RemoveEventListener(this);
		pParentDetachedFrom = pParentDetachedFrom->GetParent();
	}

	if (m_pTextService)
		m_pTextService->OnTxInPlaceDeactivate();
}

VOID CXEdit::OnOneAncestorAttachedToParent(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (!IsAncester(pSrcFrame))
		return;

	CXFrame *pParentAttachedTo = (CXFrame *)wParam;

	while (pParentAttachedTo)
	{
		pParentAttachedTo->AddEventListener(this);
		pParentAttachedTo = pParentAttachedTo->GetParent();
	}

	if (GetHWND() && m_pTextService)
	{
		CRect rcClient(0, 0, 0, 0);
		TxGetClientRect(&rcClient);
		m_pTextService->OnTxInPlaceActivate(&rcClient);
	}
}

BOOL CXEdit::IsAncester( CXFrame *pFrame )
{
	CXFrame *p = GetParent();
	while (p)
	{
		if (pFrame == p)
			return TRUE;

		p = p->GetParent();
	}

	return FALSE;
}

BOOL CXEdit::SetRect( const CRect & rcNewFrameRect )
{
	if (GetRect() == rcNewFrameRect)
		return TRUE;

	BOOL bRtn = __super::SetRect(rcNewFrameRect);

	if (m_pTextService)
		m_pTextService->OnTxPropertyBitsChange(TXTBIT_CLIENTRECTCHANGE, 0);

	return TRUE;
}

HDC CXEdit::GetEmptyDC()
{
	static HDC dcEmtpy = ::CreateCompatibleDC(NULL);
	return dcEmtpy;
}

BOOL CXEdit::ConfigFrameByXML( X_XML_NODE_TYPE xml )
{
	X_XML_ATTR_TYPE attr = NULL;
	
	attr = xml->first_attribute("font", 0, false);
	if (attr) SetFont(XLibSA2T(attr->value()));
	attr = xml->first_attribute("font_size", 0, false);
	if (attr) SetFontSize(strtoul(attr->value(), NULL, 10));
	attr = xml->first_attribute("font_style", 0, false);
	if (attr)
	{
		DWORD dwStyle = 0;

		const char *pValue = attr->value();
		if (StrStrIA(pValue, "bold"))
			dwStyle |= CFE_BOLD;
		if (StrStrIA(pValue, "italic"))
			dwStyle |= CFE_ITALIC;
		if (StrStrIA(pValue, "strikeout"))
			dwStyle |= CFE_STRIKEOUT;
		if (StrStrIA(pValue, "underline"))
			dwStyle |= CFE_UNDERLINE;

		SetFontStyle(dwStyle);
	}
	
	return __super::ConfigFrameByXML(xml);
}

LRESULT CXEdit::OnSetCursor( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble )
{
	bHandled = TRUE;

	::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));

	return TRUE;
}
