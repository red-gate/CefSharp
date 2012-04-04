
#include "WebViewEx.h"
namespace CefSharp
{
	namespace Wpf
	{
		void WebViewEx::Initialize(String^ address, BrowserSettings^ settings)
		{
			WebView::Initialize(address, settings);
			
			WebView::BeforeResourceLoadHandler = this;
		}
		void WebViewEx::HandleBeforeResourceLoad(IWebBrowser^ browserControl, IRequestResponse^ requestResponse)
		{
			OnRequestResource(browserControl, requestResponse);
		}
		void WebViewEx::OnRequestResource(IWebBrowser^ browserControl, IRequestResponse^ requestResponse)
		{
			RequestResource(browserControl, requestResponse);
		}

		void WebViewEx::OnFrameLoadEnd()
		{
			WebView::OnFrameLoadEnd();
			Dispatcher->Invoke(gcnew ActionHandler(this, &WebViewEx::OnLoadCompleted)); 
		}
		
		void WebViewEx::OnLoadCompleted()
		{
			LoadCompleted(this, EventArgs::Empty);
		}
	}
}