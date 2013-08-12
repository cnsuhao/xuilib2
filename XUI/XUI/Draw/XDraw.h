#pragma once

class IXDraw
{
public:
	virtual BOOL Draw(HDC hDC, const CRect &rcDraw) = 0;
	virtual BOOL SetAlpha(BYTE cAlpha) = 0;
	virtual BOOL SetDstRect(const CRect &rcDst) = 0;

public:
	virtual ~IXDraw(void){};
};


class IXText : public IXDraw
{
public:
	virtual CString GetText() = 0;
};

class IXImage : public IXDraw
{
public:
	enum DrawImageType{DIT_UNKNOW, DIT_NORMAL, DIT_STRETCH, DIT_9PART, DIT_3PARTH, DIT_3PARTV, DIT_CENTER};

public:
	virtual BOOL SetSrcRect(const CRect &rcSrc) = 0;

public:
	virtual BOOL SetDrawType(DrawImageType dit) = 0;
	virtual BOOL SetPartRect(const CRect &rcPart) = 0;

public:
	virtual INT GetImageHeight() = 0;
	virtual INT GetImageWidth() = 0;
};
