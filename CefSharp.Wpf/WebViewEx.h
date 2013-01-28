#pragma once

#include "WebView.h"
#include "RequestAuthCredentialsEventArgs.h"
using namespace System;
using namespace System::Net;
namespace CefSharp
{
	namespace Wpf
	{
		ref class DevToolsWebView;
		public delegate void RequestAuthCredentialsHandler(IWebBrowser^ browserControl,  RequestAuthCredentialsEventArgs^ args);
		public delegate void RequestResourceHandler(IWebBrowser^ browserControl, IRequestResponse^ requestResponse);

		public delegate void DevToolsShownHandler(DevToolsWebView^ view);

		public ref class WebViewEx : public WebView, IRequestHandler
		{
		private:
			bool showingDevTools;
			delegate void ActionHandler();
			
			delegate IntPtr GetMainWindowHandleHandler();
			IntPtr GetMainWindowHandle();
			DevToolsWebView^ CreateDevToolsView();
		protected:
			bool adapterInitialized;
			MCefRefPtr<RenderClientAdapter> adapter;
			virtual void Initialize(String^ address, BrowserSettings^ settings) override;
			virtual CefRefPtr<RenderClientAdapter> CreateClientAdapter() override;

		public:
			virtual event RequestAuthCredentialsHandler^ RequestAuthCredentials;
			virtual event RequestResourceHandler^ RequestResource;
			virtual event DevToolsShownHandler^ DevToolsShown;
			virtual event EventHandler^ LoadCompleted;
			
			WebViewEx() 
			{
								
			}
			
			WebViewEx(String^ address, BrowserSettings^ settings) : WebView(address,settings)
			{
			}

			virtual bool PopupShowing(CefRefPtr<CefBrowser> parentBrowser, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, const CefString& url, CefRefPtr<CefClient>& client, CefBrowserSettings& settings);
			virtual void PopupShown(CefRefPtr<CefBrowser> browser) {};
		
			
			
			virtual  bool OnBeforeBrowse(IWebBrowser^ browser, IRequest^ request, NavigationType naigationvType, bool isRedirect){return false;};
			virtual bool OnBeforeResourceLoad(IWebBrowser^ browser, IRequestResponse^ requestResponse);
			virtual void OnResourceResponse(IWebBrowser^ browser, String^ url, int status, String^ statusText, String^ mimeType, WebHeaderCollection^ headers){};
			virtual bool GetAuthCredentials(IWebBrowser^ browser, Uri^ serverAddress, bool isProxy, String^ realm, String^% username, String^% password);
		    
			virtual void OnRequestResource(IWebBrowser^ browserControl, IRequestResponse^ requestResponse);
			virtual bool OnRequestAuthCredentials(IWebBrowser^ browserControl, Uri^ serverAddress, bool isProxy, String^ realm, String^% username, String^% password);
			virtual void OnLoadCompleted();
			virtual void OnShowDevTools(DevToolsWebView^ view);
			virtual void OnFrameLoadEnd() override;
			virtual void ShowDevTools() override;

		};

	}
}
