#pragma once

#include "WebView.h"

using namespace System;
using namespace System::Net;
namespace CefSharp
{
	namespace Wpf
	{
		public delegate void RequestResourceHandler(IWebBrowser^ browserControl, IRequestResponse^ requestResponse);

		public delegate void DevToolsShownHandler();

		public ref class WebViewEx : public WebView, IRequestHandler
		{
		private:
			bool _isDevTools;
			bool adapterInitialized;
			bool showingDevTools;
			delegate void ActionHandler();
			
			WebViewEx^ devToolsView;
			MCefRefPtr<RenderClientAdapter> _adapter;
		
			delegate IntPtr GetMainWindowHandleHandler();
			IntPtr GetMainWindowHandle();
			
		protected:
			virtual void Initialize(String^ address, BrowserSettings^ settings) override;
			virtual CefRefPtr<RenderClientAdapter> CreateClientAdapter() override;
			virtual void CreateBrowser() override;

		internal:
			virtual property bool IsDevTools
			{
				bool get() { return _isDevTools; }
			}

		public:
			virtual event RequestResourceHandler^ RequestResource;
			virtual event DevToolsShownHandler^ DevToolsShown;
			virtual event EventHandler^ LoadCompleted;
			
			WebViewEx():WebView()
			{
				devToolsView = gcnew WebViewEx(true);
				devToolsView->Visibility = ::Visibility::Collapsed;
			}
			
			WebViewEx(bool isDevTools):WebView()
			{
				_isDevTools = isDevTools;
			}
			WebViewEx(String^ address, BrowserSettings^ settings) : WebView(address,settings)
			{
				devToolsView = gcnew WebViewEx(true);
				devToolsView->Visibility = ::Visibility::Collapsed;
			}

			virtual bool PopupShowing(CefRefPtr<CefBrowser> parentBrowser, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, const CefString& url, CefRefPtr<CefClient>& client, CefBrowserSettings& settings);
			virtual void PopupShown(CefRefPtr<CefBrowser> browser) ;
		
			
			
			virtual  bool OnBeforeBrowse(IWebBrowser^ browser, IRequest^ request, NavigationType naigationvType, bool isRedirect){return false;};
			virtual bool OnBeforeResourceLoad(IWebBrowser^ browser, IRequestResponse^ requestResponse);
			virtual void OnResourceResponse(IWebBrowser^ browser, String^ url, int status, String^ statusText, String^ mimeType, WebHeaderCollection^ headers){};
		    
			virtual property WebViewEx^ DevToolsView
			{
				WebViewEx^ get() { return devToolsView; }
			}

			virtual void OnRequestResource(IWebBrowser^ browserControl, IRequestResponse^ requestResponse);
			virtual void OnLoadCompleted();
			virtual void OnShowDevTools();
			virtual void OnFrameLoadEnd() override;
			virtual void ShowDevTools() override;
			virtual void CloseDevTools() override;

		};

	}
}
