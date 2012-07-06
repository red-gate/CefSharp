#include "stdafx.h"
#pragma once

#include "IWebBrowser.h"

namespace CefSharp
{
    public interface class IRenderWebBrowser : IWebBrowser
    {
    public:
        void SetCursor(CefCursorHandle cursor);
        void SetBuffer(int width, int height, const void* buffer);

        void SetPopupBuffer(int width, int height, const void* buffer);
		void SetPopupIsOpened(bool isOpened);

		void SetPopupSizeAndPosition(const CefRect& rect);

		void GetScreenPoint(int relativeX, int relativeY, int& screenX, int& screenY);
    };
}