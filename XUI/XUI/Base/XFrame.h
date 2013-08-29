#pragma once

#include "../Draw/XDraw.h"

#include "RemoteRef.h"
#include "XFrameMsgMgr.h"
#include "XFrameEvent.h"
#include "XFrameXMLFactory.h"

#include <set>

#include <mmsystem.h>

#define INVALID_FRAME_INDEX ((UINT)-1)

#define WM_DELAY_UPDATE_LAYOUT_PARAM (WM_USER + 0x120)

class CXFrame :
	public IXFrameEventListener,
	SUPPORT_REMOTE_REFERENCE
{
	X_DECLEAR_FRAME_XML_RUNTIME(CXFrame, XFrame)
	#pragma comment(linker, "/INCLUDE:?s_xml_runtime_info@CXFrame@@0VCXFrameXMLRuntime_CXFrame@1@A")

	BEGIN_FRAME_EVENT_MAP(CXFrame)
	END_FRAME_EVENT_MAP()

	BEGIN_FRAME_MSG_MAP(CXFrame)
		FRAME_MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		FRAME_MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDown)
		FRAME_MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		FRAME_MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		FRAME_MESSAGE_HANDLER(WM_X_MOUSEENTER, OnMouseEnter)
		FRAME_MESSAGE_HANDLER(WM_DELAY_UPDATE_LAYOUT_PARAM, OnDelayUpdateLayoutParam)
	END_FRAME_MSG_MAP()

public:
	enum VISIBILITY 
	{
		VISIBILITY_NONE = -1,    // Do not hold place and invisible. 
		VISIBILITY_HIDE = 0,     // Hold place but invisible. 
		VISIBILITY_SHOW = 1		 // Hold place and visible. 
	};

public:
	class LayoutParam
	{
	public:
		enum SPECIAL_METRICS 
			{ METRIC_REACH_PARENT = -1,  METRIC_WRAP_CONTENT = -2};
	public:
		INT m_nX, m_nY;
		union
		{
			INT					m_nWidth;
			SPECIAL_METRICS		m_mWidth;
		};
		union
		{
			INT					m_nHeight;
			SPECIAL_METRICS		m_mHeight;
		};

	public:
		INT m_nMarginLeft;
		INT m_nMarginTop;
		INT m_nMarginRight;
		INT m_nMarginBottom;

	public:
		LayoutParam () :
		  m_nX(0), m_nY(0), m_nWidth(0), m_nHeight(0),
		  m_nMarginLeft(0), m_nMarginTop(0), m_nMarginRight(0), m_nMarginBottom(0) {}
	    LayoutParam (X_XML_NODE_TYPE xml);

	public:
		virtual BOOL FillByXML(X_XML_NODE_TYPE xml);

	private:
		INT GetSpecialMetrics (LPCSTR pStr);

	public:
		virtual ~LayoutParam() {}
	};

	class MeasureParam
	{
	public:
		enum MEASURE_SPEC { MEASURE_UNRESTRICTED, MEASURE_ATMOST, MEASURE_EXACT };

	public:
		MEASURE_SPEC m_Spec;
		INT m_nNum;

	public:
		MeasureParam() : m_Spec(MEASURE_UNRESTRICTED), m_nNum(0) {}
		BOOL Reset() { m_Spec = MEASURE_UNRESTRICTED; m_nNum = 0; return TRUE;}

	public:
		BOOL operator == (const MeasureParam & other) const;
		BOOL operator != (const MeasureParam & other) const
			{ return this->operator == (other); }
	};


public:
	virtual BOOL ConfigFrameByXML(X_XML_NODE_TYPE xml);
	virtual BOOL HandleXMLChildNodes(X_XML_NODE_TYPE xml);

public:
	BOOL SetName(LPCTSTR pName);
	CString GetName();
	CXFrame * GetFrameByName(LPCTSTR pName);
	BOOL GetFrameByName(LPCTSTR pName, std::vector<CXFrame *> *pvFrames);

public:
	CXFrame(void);
	// We can't call the virtual Destroy in the C++ destructor, so, we MUST destroy a frame before destroy the frame object. 
	virtual ~CXFrame(void);

public:
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnMouseEnter(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	LRESULT OnDelayUpdateLayoutParam(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled, BOOL& bCancelBabble);
	

public:
	BOOL Create(CXFrame * pFrameParent, LayoutParam * pLayout, VISIBILITY visibility = VISIBILITY_NONE);
	virtual VOID Destroy();
	BOOL IsFrameActive();

	UINT GetFrameCount();
	CXFrame * GetFrameByIndex(UINT nIndex);
	UINT GetFrameIndex(CXFrame * pFrame);
	virtual BOOL InsertFrame(CXFrame * pFrame, UINT nIndex);
	BOOL AddFrame(CXFrame * pFrame);
	virtual CXFrame * RemoveFrame(UINT nIndex);
	BOOL RemoveFrame(CXFrame * pFrame);

public:
	BOOL SetParent(CXFrame * pFrameParent);
	CXFrame * GetParent();

	virtual BOOL SetRect(const CRect & rcNewFrameRect);
	BOOL Move(const CPoint &pt);
	CRect GetRect();
	virtual INT GetVCenter();

	CXFrame * GetTopFrameFromPoint(const CPoint &pt);

public:
	virtual BOOL SetVisibility(VISIBILITY visibility);
	VISIBILITY GetVisibility();

	BOOL GetFocus();
	BOOL KillFocus();

public:
	virtual IXImage * SetBackground(IXImage * pDrawBackground);
	IXImage * GetBackground();

public:
	BOOL SetTouchable(BOOL b);
	IXImage * SetMouseOverLayer(IXImage *pLayer);
	IXImage * SetMouseDownLayer(IXImage *pLayer);
	BOOL SetSelectable(BOOL b);
	BOOL SetSelectWhenMouseClick(BOOL b);
	BOOL SetUnselectWhenMouseClick(BOOL b);
	BOOL SetSelectedState(BOOL b);
	IXImage * SetSelectedLayer(IXImage *pLayer);

public:
	virtual LayoutParam * GenerateLayoutParam (LayoutParam *pCopyFrom = NULL);
	LayoutParam * GenerateLayoutParam (X_XML_NODE_TYPE xml);

	LayoutParam * GetLayoutParam();
	LayoutParam * BeginUpdateLayoutParam();
	BOOL BeginUpdateLayoutParam(LayoutParam *pLayoutParam);
	virtual BOOL EndUpdateLayoutParam();

	virtual BOOL RequestLayout();
	virtual BOOL IsLayouting();

	BOOL MeasureWidth(const MeasureParam & param);
	BOOL MeasureHeight(const MeasureParam & param);
	INT GetMeasuredWidth();
	INT GetMeasuredHeight();

	BOOL Layout(const CRect & rcRect);

	virtual BOOL OnMeasureWidth(const MeasureParam & param);
	virtual BOOL OnMeasureHeight(const MeasureParam & param);
	virtual BOOL OnLayout(const CRect & rcRect);


public:
	virtual BOOL InvalidateRect(const CRect & rect);
	BOOL InvalidateRect();
	BOOL InvalidateAfterLayout();
	virtual BOOL Update();

public:
	virtual	HWND GetHWND();
	virtual CXFrameMsgMgr *GetFrameMsgMgr();

public:
	virtual BOOL PaintUI(HDC hDC, const CRect &rect);
	virtual BOOL PaintBackground(HDC hDC, const CRect &rect);
	virtual BOOL PaintForeground(HDC hDC, const CRect &rect);
	virtual HDC GetDC();
	virtual BOOL ReleaseDC(HDC dc, BOOL bUpdate = TRUE);

public:
	CPoint ParentToChild(const CPoint &pt);
	CRect ParentToChild(const CRect &rc);
	CPoint ChildToParent(const CPoint &pt);
	CRect ChildToParent(const CRect &rc);

	CPoint OtherFrameToThisFrame(CXFrame * pFrameOther, const CPoint &pt);
	CRect OtherFrameToThisFrame(CXFrame * pFrameOther, const CRect &rect);
	
	CRect FrameToWindow(const CRect &rc);
	virtual CPoint FrameToWindow(const CPoint &pt);
	CRect FrameToScreen(const CRect &rc);
	virtual CPoint FrameToScreen(const CPoint &pt);
	CRect ScreenToFrame(const CRect &rc);
	virtual CPoint ScreenToFrame(const CPoint &pt);

public:
	BOOL PostFrameMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
	UINT SetTimer(UINT uElapse);
	BOOL KillTimer(UINT nID);

public:
	BOOL AddEventListener(IXFrameEventListener *pListener);
	BOOL RemoveEventListener(IXFrameEventListener *pListener);
	VOID ThrowEvent(UINT uEvent, WPARAM wParam, LPARAM lParam);

public:
	virtual BOOL NeedPrepareMessageForThisFrame();

public:
	BOOL SetToolTip(CString strToolTip);
	BOOL ToolTipInitial(CPoint pt);
	BOOL ToolTipDestroy();
	BOOL ToolTipInitialEx(CPoint pt);
	BOOL ToolTipDestroyEx();

protected:
	BOOL SetMeasuredWidth(INT width);
	BOOL SetMeasuredHeight(INT height);

protected:
	virtual VOID OnDetachedFromParent();
	virtual VOID OnAttachedToParent(CXFrame *pParent);

protected:
	MeasureParam GetDefaultMeasureParam(LayoutParam * pLayoutParam);

private:
	BOOL OnMeasureLayoutDirection (
		const MeasureParam & param, INT *pMeasuredSize,
		BOOL (CXFrame::*pfChildMeasureProc)(const MeasureParam &),
		INT (CXFrame::*pfChildGetMeasurdProc)(),
		INT LayoutParam::*pLayoutParamPos, 
		INT LayoutParam::*pnLayoutParamSize, 
		LayoutParam::SPECIAL_METRICS LayoutParam::*pmLayoutParamSize,
		INT LayoutParam::*pLayoutMarginEnd);

private:
	BOOL FillMouseOverLayer();
	BOOL FillMouseDownLayer();
	BOOL FillSelectedLayer();

private:
	CString m_strFrameName;

	std::vector<CXFrame *> m_vecFrameChild;
	CXFrame * m_pFrameParent;
	IXImage * m_pDrawBackground;

	BOOL m_bTouchable;
	IXImage * m_pMouseOverLayer;
	IXImage * m_pMouseDownLayer;
	BOOL m_bSelectable;
	BOOL m_bSelectWhenMouseCilck;
	BOOL m_bUnselectWhenMouseCilck;
	BOOL m_bSelectedState;
	IXImage * m_pSelectedLayer;

	BOOL m_bMouseOver;
	BOOL m_bMouseDown;

	LayoutParam * m_pLayoutParam;
	LayoutParam * m_pDelayLayoutParam;
	BOOL m_bDelayUpdateLayoutParamScheduled;
	MeasureParam m_LastMeasureWidthParam;
	MeasureParam m_LastMeasureHeightParam;
	INT m_nMeasuredWidth, m_nMeasuredHeight;
	BOOL m_bLayoutInvaild;

	CRect m_rcFrame;
	VISIBILITY m_Visibility;

	BOOL m_bIsFrameAlive;

	CToolTipCtrl m_ToolTip;
	CString m_strToolTip;
	HWND m_hToolTip;

private:
	std::map<CRemoteRef<IXFrameEventListener>, DWORD> /* listener -> add count. */ m_mapEventListener;

private:
	static DWORD RegisterInstanceID();
	static BOOL UnregisterInstanceID(DWORD dwInstanceID);
	DWORD m_dwInstanceID;

public:
	static BOOL IsInstanceActive(DWORD dwInstanceID);
	DWORD GetInstanceID();	

private:
	static std::vector<BOOL> s_bActiveInstances;
};
