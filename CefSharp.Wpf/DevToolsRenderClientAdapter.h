#pragma once
#include "renderclientadapter.h"
#include "WebViewEx.h"

namespace CefSharp
{
	namespace Wpf
	{

		public class DevToolsRenderClientAdapter : public RenderClientAdapter
		{
		private:
					
			gcroot<WebViewEx^> _webView;
		public:
			DevToolsRenderClientAdapter(WebViewEx^ offscreenBrowserControl) : RenderClientAdapter((IRenderWebBrowser^)offscreenBrowserControl)
			{
				_webView = offscreenBrowserControl;
			}
        
			virtual DECL void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
		};

	}
}