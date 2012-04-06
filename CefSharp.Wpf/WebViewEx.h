#pragma once

#include "WebView.h"

namespace CefSharp
{
	namespace Wpf
	{
		
		public delegate void RequestResourceHandler(IWebBrowser^ browserControl, IRequestResponse^ requestResponse);
		
		public ref class DevToolsControl : public HwndHost
		{
		private:
			HandleRef _childHandle;
			
		protected:	
			virtual	HandleRef BuildWindowCore(HandleRef hwndParent) override
			{
				return _childHandle;
			}
			virtual	void DestroyWindowCore(HandleRef hwndParent) override
			{
			}
		public:
			DevToolsControl(HandleRef ref) : HwndHost()
			{
				_childHandle = ref;
			}			
		};
		
		public delegate void ShowDevToolsHandler(DevToolsControl^ devToolsControl);

		public ref class WebViewEx : public WebView, IBeforeResourceLoad
		{
		private:
			bool showingDevTools;
			delegate void ActionHandler();
			delegate IntPtr GetMainWindowHandleHandler();
			delegate DevToolsControl^ CreateDevToolsControlHandler(IntPtr handle);
			
			IntPtr GetMainWindowHandle();
			DevToolsControl^ CreateDevToolsControl(IntPtr handle);
		
			
		protected:
			virtual void Initialize(String^ address, BrowserSettings^ settings) override;
			virtual CefRefPtr<RenderClientAdapter> CreateClientAdapter() override;
		public:
			virtual event RequestResourceHandler^ RequestResource;
			virtual event ShowDevToolsHandler^ DevToolsShowing;
			virtual event EventHandler^ LoadCompleted;
			
			WebViewEx():WebView()
			{
			}

			WebViewEx(String^ address, BrowserSettings^ settings) : WebView(address,settings)
			{
			}

			virtual bool PopupShowing(CefRefPtr<CefBrowser> parentBrowser, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, const CefString& url, CefRefPtr<CefClient>& client, CefBrowserSettings& settings);
			virtual void PopupShown(CefRefPtr<CefBrowser> browser) ;
		
			virtual void HandleBeforeResourceLoad(IWebBrowser^ browserControl, IRequestResponse^ requestResponse);
			
			virtual void OnRequestResource(IWebBrowser^ browserControl, IRequestResponse^ requestResponse);
			virtual void OnLoadCompleted();
			virtual void OnShowDevTools(DevToolsControl^ devTools);
			virtual void OnFrameLoadEnd() override;
		};

	}
}
