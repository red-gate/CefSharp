#pragma once
#include "webviewex.h"

namespace CefSharp
{
	namespace Wpf
	{
		ref class WebViewEx;
		public ref class DevToolsWebView :public WebViewEx
		{
		private:
			WebViewEx^ _parent;
		protected:
			virtual void CreateBrowser() override{
			}
			
			virtual CefRefPtr<RenderClientAdapter> CreateClientAdapter() override;
		public:
			DevToolsWebView(WebViewEx^ parent) {
				_parent = parent;
			}
			virtual bool PopupShowing(CefRefPtr<CefBrowser> parentBrowser, 
									  const CefPopupFeatures& popupFeatures, 
									  CefWindowInfo& windowInfo,
									  const CefString& url, 
									  CefRefPtr<CefClient>& client, 
									  CefBrowserSettings& settings) override{
				return false;
			}
			virtual void PopupShown(CefRefPtr<CefBrowser> browser) override{
				
				Dispatcher->Invoke( gcnew DevToolsShownHandler(_parent, &WebViewEx::OnShowDevTools), this);
			}
		
			
		};
	}
}

