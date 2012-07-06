#include "stdafx.h"

#include "IRenderWebBrowser.h"
#include "RenderClientAdapter.h"


namespace CefSharp
{
    void RenderClientAdapter::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
    {
		Console::WriteLine("Popup show : " + show);
		_renderBrowserControl->SetPopupIsOpened(show);
    }

    void RenderClientAdapter::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
    {
		Console::WriteLine("Popup size: x:{0}, y:{1} w:{2}, h:{3}", rect.x, rect.y, rect.width, rect.height);
		_renderBrowserControl->SetPopupSizeAndPosition(rect);
    }

    void RenderClientAdapter::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer)
    {
        if (type == PET_VIEW)
        {
            int width, height;
            browser->GetSize(type, width, height);

            _renderBrowserControl->SetBuffer(width, height, buffer);
        }
        else if (type == PET_POPUP)
        {
            int width, height;
            browser->GetSize(type, width, height);
			
			Console::WriteLine("(x :{0}, y:{1}) - (w:{2}, h:{3})", dirtyRects[0].x, dirtyRects[0].y, dirtyRects[0].width, dirtyRects[0].height);
			
			Console::WriteLine("(width :{0}, height:{1})", width, height);
            _renderBrowserControl->SetPopupBuffer(width, height, buffer);
        }
    }

    void RenderClientAdapter::OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor)
    {
        _renderBrowserControl->SetCursor(cursor);
    }

	bool RenderClientAdapter::GetScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
    {
		Console::WriteLine("GetScreenRect");
		return false;
    }
	bool RenderClientAdapter::GetScreenPoint(CefRefPtr<CefBrowser> browser,
                              int viewX,
                              int viewY,
                              int& screenX,
                              int& screenY)
    {
		Console::WriteLine("GetScreenPoint");
		return false;
    }
	bool RenderClientAdapter::GetViewRect(CefRefPtr<CefBrowser> browser,
                           CefRect& rect)
    {
		Console::WriteLine("GetViewRect");
		return false;
    }
}