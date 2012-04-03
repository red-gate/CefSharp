#pragma once

#include "WebView.h"

namespace CefSharp
{
	namespace Wpf
	{
		
		public delegate void RequestResourceHandler(IWebBrowser^ browserControl, IRequestResponse^ requestResponse);

		public ref class WebViewEx : public WebView, IBeforeResourceLoad
		{
		public:
			virtual event RequestResourceHandler^ RequestResource;
			virtual event EventHandler^ LoadCompleted;
			
			WebViewEx():WebView()
			{}

			WebViewEx(String^ address, BrowserSettings^ settings) : WebView(address,settings)
			{}

			
			
			virtual void HandleBeforeResourceLoad(IWebBrowser^ browserControl, IRequestResponse^ requestResponse);
			
			virtual void OnRequestResource(IWebBrowser^ browserControl, IRequestResponse^ requestResponse);
			virtual void OnLoadCompleted();
			
			virtual void OnFrameLoadEnd() override;
		protected:
			virtual void Initialize(String^ address, BrowserSettings^ settings) override;
		};

	}
}
