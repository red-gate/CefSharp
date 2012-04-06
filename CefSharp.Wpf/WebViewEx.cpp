
#include "WebViewEx.h"
#include "RenderClientAdapterEx.h"

namespace CefSharp
{
	namespace Wpf
	{
		void WebViewEx::Initialize(String^ address, BrowserSettings^ settings)
		{
			WebView::Initialize(address, settings);
			
			WebView::BeforeResourceLoadHandler = this;
		}
		CefRefPtr<RenderClientAdapter> WebViewEx::CreateClientAdapter()
		{
			return new RenderClientAdapterEx(this);
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

		bool WebViewEx::PopupShowing(CefRefPtr<CefBrowser> parentBrowser,
			const CefPopupFeatures& popupFeatures, 
			CefWindowInfo& windowInfo,
			const CefString& url, 
			CefRefPtr<CefClient>& client,
			CefBrowserSettings& settings)
		{
			
			String^ urlString = toClr(url);
			if(urlString->StartsWith("chrome-devtools://"))
			{
				showingDevTools = true;
				IntPtr mwHandle = (IntPtr)Dispatcher->Invoke( gcnew GetMainWindowHandleHandler(this, &WebViewEx::GetMainWindowHandle));
				RECT rect;
				windowInfo.SetAsChild(static_cast<HWND>(mwHandle.ToPointer()), rect);
			}			
			return false;
		}

		IntPtr WebViewEx::GetMainWindowHandle()
		{
			Visual^ parent = (Visual^)VisualTreeHelper::GetParent(this);
			HwndSource^ source = (HwndSource^)PresentationSource::FromVisual(parent);
			return source->Handle;
		}

		DevToolsControl^ WebViewEx::CreateDevToolsControl(IntPtr handle)
		{			
			return gcnew DevToolsControl( HandleRef(this, handle));			
		}

		void WebViewEx::PopupShown(CefRefPtr<CefBrowser> browser) 
		{
			if(showingDevTools)
			{
				showingDevTools = false;
				HWND handle = browser->GetWindowHandle();
				DevToolsControl^ devTools = (DevToolsControl^)Dispatcher->Invoke( gcnew CreateDevToolsControlHandler(this, &WebViewEx::CreateDevToolsControl), IntPtr(handle));
				Dispatcher->Invoke( gcnew ShowDevToolsHandler(this, &WebViewEx::OnShowDevTools), devTools);
			}
		}

		void WebViewEx::OnShowDevTools(DevToolsControl^ devTools)
		{
			DevToolsShowing(devTools);
		}

		void WebViewEx::OnLoadCompleted()
		{
			LoadCompleted(this, EventArgs::Empty);
		}
	}
}