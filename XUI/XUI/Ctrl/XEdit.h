#pragma once

#include <textserv.h>

#include "..\Base\XFrame.h"

#pragma comment(lib, "Riched20.lib")
#pragma comment(lib, "Imm32.lib")

class CXCaret;
class CXEdit : 
	public CXFrame, 
	public ITextHost
{
	X_DECLEAR_FRAME_XML_RUNTIME(CXEdit, XEdit)
	#pragma comment(linker, "/INCLUDE:?s_xml_runtime_info@CXEdit@@0VCXFrameXMLRuntime_CXEdit@1@A")

	BEGIN_FRAME_EVENT_MAP(CXEdit)
		FRAME_EVENT_HANDLER(EVENT_FRAME_ATTACHED_TO_PARENT, OnOneAncestorAttachedToParent)
		FRAME_EVENT_HANDLER(EVENT_FRAME_DETACHED_FROM_PARENT, OnOneAncestorDetachedFromaParent)
	END_FRAME_EVENT_MAP()

	BEGIN_FRAME_MSG_MAP(CXEdit)
		CHAIN_FRAME_MSG_MAP(CXFrame)
		FRAME_MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		FRAME_MESSAGE_HANDLER(WM_TIMER, OnFrameTimer)
		FRAME_ALL_MESSAGE_HANDLER(OnAllMessage)
	END_FRAME_MSG_MAP()

public:
	BOOL Create(CXFrame * pFrameParent, LayoutParam * pLayout, VISIBILITY visibility = VISIBILITY_NONE,
		BOOL bTransparent = FALSE, BOOL bMultiline = FALSE, BOOL bPasswordMode = FALSE);
	BOOL SetTransparent(BOOL b);
	BOOL SetFont(LPCTSTR pFontName);
	BOOL SetFontSize(UINT nPixel);
	/* Can be a combination of CFE_BOLD, CFE_ITALIC, CFE_UNDERLINE and CFE_STRIKEOUT */
	BOOL SetFontStyle(DWORD dwEffect);
	BOOL SetMultiline(BOOL b);
	BOOL SetPasswordMode(BOOL b);

public:
	LRESULT OnFrameTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnAllMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);

public:
	VOID OnOneAncestorAttachedToParent(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	VOID OnOneAncestorDetachedFromaParent(CXFrame *pSrcFrame, UINT uEvent, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
	CXEdit(void);

public:
	virtual BOOL SetRect(const CRect & rcNewFrameRect);
	virtual BOOL ConfigFrameByXML(X_XML_NODE_TYPE xml);
	virtual BOOL PaintForeground(HDC hDC, const CRect &rect);
	virtual VOID Destroy();
	virtual BOOL NeedPrepareMessageForThisFrame();

protected:
	virtual VOID OnDetachedFromParent();
	virtual VOID OnAttachedToParent(CXFrame *pParent);

public:
	static const IID * GetITextServiceIID();
	static const IID * GetITextHostIID();

	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);

	virtual ULONG STDMETHODCALLTYPE AddRef( void);

	virtual ULONG STDMETHODCALLTYPE Release( void);

	//@cmember Get the DC for the host
	virtual HDC 		TxGetDC();

	//@cmember Release the DC gotten from the host
	virtual INT			TxReleaseDC(HDC hdc);

	//@cmember Show the scroll bar
	virtual BOOL 		TxShowScrollBar(INT fnBar, BOOL fShow);

	//@cmember Enable the scroll bar
	virtual BOOL 		TxEnableScrollBar (INT fuSBFlags, INT fuArrowflags);

	//@cmember Set the scroll range
	virtual BOOL 		TxSetScrollRange(
		INT fnBar,
		LONG nMinPos,
		INT nMaxPos,
		BOOL fRedraw);

	//@cmember Set the scroll position
	virtual BOOL 		TxSetScrollPos (INT fnBar, INT nPos, BOOL fRedraw);

	//@cmember InvalidateRect
	virtual void		TxInvalidateRect(LPCRECT prc, BOOL fMode);

	//@cmember Send a WM_PAINT to the window
	virtual void 		TxViewChange(BOOL fUpdate);

	//@cmember Create the caret
	virtual BOOL		TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight);

	//@cmember Show the caret
	virtual BOOL		TxShowCaret(BOOL fShow);

	//@cmember Set the caret position
	virtual BOOL		TxSetCaretPos(INT x, INT y);

	//@cmember Create a timer with the specified timeout
	virtual BOOL 		TxSetTimer(UINT idTimer, UINT uTimeout);

	//@cmember Destroy a timer
	virtual void 		TxKillTimer(UINT idTimer);

	//@cmember Scroll the content of the specified window's client area
	virtual void		TxScrollWindowEx (
		INT dx,
		INT dy,
		LPCRECT lprcScroll,
		LPCRECT lprcClip,
		HRGN hrgnUpdate,
		LPRECT lprcUpdate,
		UINT fuScroll);

	//@cmember Get mouse capture
	virtual void		TxSetCapture(BOOL fCapture);

	//@cmember Set the focus to the text window
	virtual void		TxSetFocus();

	//@cmember Establish a new cursor shape
	virtual void 	TxSetCursor(HCURSOR hcur, BOOL fText);

	//@cmember Converts screen coordinates of a specified point to the client coordinates
	virtual BOOL 		TxScreenToClient (LPPOINT lppt);

	//@cmember Converts the client coordinates of a specified point to screen coordinates
	virtual BOOL		TxClientToScreen (LPPOINT lppt);

	//@cmember Request host to activate text services
	virtual HRESULT		TxActivate( LONG * plOldState );

	//@cmember Request host to deactivate text services
	virtual HRESULT		TxDeactivate( LONG lNewState );

	//@cmember Retrieves the coordinates of a window's client area
	virtual HRESULT		TxGetClientRect(LPRECT prc);

	//@cmember Get the view rectangle relative to the inset
	virtual HRESULT		TxGetViewInset(LPRECT prc);

	//@cmember Get the default character format for the text
	virtual HRESULT 	TxGetCharFormat(const CHARFORMATW **ppCF );

	//@cmember Get the default paragraph format for the text
	virtual HRESULT		TxGetParaFormat(const PARAFORMAT **ppPF);

	//@cmember Get the background color for the window
	virtual COLORREF	TxGetSysColor(int nIndex);

	//@cmember Get the background (either opaque or transparent)
	virtual HRESULT		TxGetBackStyle(TXTBACKSTYLE *pstyle);

	//@cmember Get the maximum length for the text
	virtual HRESULT		TxGetMaxLength(DWORD *plength);

	//@cmember Get the bits representing requested scroll bars for the window
	virtual HRESULT		TxGetScrollBars(DWORD *pdwScrollBar);

	//@cmember Get the character to display for password input
	virtual HRESULT		TxGetPasswordChar(TCHAR *pch);

	//@cmember Get the accelerator character
	virtual HRESULT		TxGetAcceleratorPos(LONG *pcp);

	//@cmember Get the native size
	virtual HRESULT		TxGetExtent(LPSIZEL lpExtent);

	//@cmember Notify host that default character format has changed
	virtual HRESULT 	OnTxCharFormatChange (const CHARFORMATW * pcf);

	//@cmember Notify host that default paragraph format has changed
	virtual HRESULT		OnTxParaFormatChange (const PARAFORMAT * ppf);

	//@cmember Bulk access to bit properties
	virtual HRESULT		TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits);

	//@cmember Notify host of events
	virtual HRESULT		TxNotify(DWORD iNotify, void *pv);

	// Far East Methods for getting the Input Context
	//#ifdef WIN95_IME
	virtual HIMC		TxImmGetContext();
	virtual void		TxImmReleaseContext( HIMC himc );
	//#endif

	//@cmember Returns HIMETRIC size of the control bar.
	virtual HRESULT		TxGetSelectionBarWidth (LONG *lSelBarWidth);

private:
	VOID ResetCharFormat();
	VOID ResetParaFormat();
	INT GetTextHostWidth();
	INT GetTextHostHeight();
	CXFrame * GetRootFrame(); /* never returns NULL */
	CPoint ThisFrameToRootFrame(const CPoint &pt);
	CRect  ThisFrameToRootFrame(const CRect &rc);
	RECTL  ThisFrameToRootFrame(const RECTL &rcl);
	CPoint RootFrameToThisFrame(const CPoint &pt);
	CRect  RootFrameToThisFrame(const CRect &rc);
	BOOL IsAncester(CXFrame *pFrame);
	HDC GetEmptyDC();

private:
	ULONG m_nRefCount;

	ITextServices *m_pTextService;

	std::map<UINT, UINT> m_mapFrameTimerIDToHostTimerID;

	CHARFORMATW m_CharFormat;
	PARAFORMAT m_ParaFormat;
	BOOL m_bMutiline;
	BOOL m_bPasswordMode;
	BOOL m_bTransparent;

	CXCaret *m_pCaret;

	CPoint m_ptOldDCViewPort;
};
