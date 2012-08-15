#include "DevToolsRenderClientAdapter.h"

namespace CefSharp
{
	namespace Wpf
	{	
		void DevToolsRenderClientAdapter::OnAfterCreated(CefRefPtr<CefBrowser> browser)
		{
			if(!_webView->IsBrowserInitialized)
			{
				_browserHwnd = browser->GetWindowHandle();
				_cefBrowser = browser;

				_browserControl->OnInitialized();
				_webView->PopupShown(browser);
			}			
		}		

	}
}