
#include "WebViewEx.h"
#include "RenderClientAdapterEx.h"

namespace CefSharp
{
	namespace Wpf
	{
		void WebViewEx::Initialize(String^ address, BrowserSettings^ settings)
		{
			WebView::Initialize(address, settings);
			
			WebView::RequestHandler = this;
		}
		
		void WebViewEx::CreateBrowser()
		{
			if(!_isDevTools)
			{
				WebView::CreateBrowser();
			}
		}

		CefRefPtr<RenderClientAdapter> WebViewEx::CreateClientAdapter()
		{
			if(!adapterInitialized)
			{
				adapterInitialized = true;
				CefRefPtr<RenderClientAdapter> adapter = new RenderClientAdapterEx(this);
				_adapter = adapter.get();
			}
			return _adapter.get();
		}
		bool WebViewEx::OnBeforeResourceLoad(IWebBrowser^ browserControl, IRequestResponse^ requestResponse)
		{
			OnRequestResource(browserControl, requestResponse);
			return false;
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

		IntPtr WebViewEx::GetMainWindowHandle()
		{	
			HwndSource^ source = (HwndSource^)PresentationSource::FromVisual(this);
			return source->Handle;
		}

		bool WebViewEx::PopupShowing(CefRefPtr<CefBrowser> parentBrowser,
			const CefPopupFeatures& popupFeatures, 
			CefWindowInfo& windowInfo,
			const CefString& url, 
			CefRefPtr<CefClient>& client,
			CefBrowserSettings& settings)
		{
			


			String^ urlString = toClr(url);
			if(showingDevTools && urlString->StartsWith("chrome-devtools://") && !_isDevTools)
			{
				settings.user_style_sheet_enabled = false;
				IntPtr mwHandle = (IntPtr)Dispatcher->Invoke( gcnew GetMainWindowHandleHandler(this, &WebViewEx::GetMainWindowHandle));
				if(mwHandle != IntPtr::Zero)
				{
					RECT rect;
					CefWindowInfo window;
       
					window.SetAsOffScreen(static_cast<HWND>(mwHandle.ToPointer()));
					windowInfo = window;

					CefRefPtr<RenderClientAdapter> clientAdapter = devToolsView->CreateClientAdapter();
					client = clientAdapter;
				}
			}			
			return false;
		}

		void WebViewEx::PopupShown(CefRefPtr<CefBrowser> browser) 
		{
			if(_isDevTools)
			{
				showingDevTools = false;
				HWND handle = browser->GetWindowHandle();
				HWND parent = GetParent(handle);
				
				Console::WriteLine((long)handle);
				Console::WriteLine((long)parent);
				Dispatcher->Invoke( gcnew DevToolsShownHandler(this, &WebViewEx::OnShowDevTools));
			}
		}

		void WebViewEx::OnShowDevTools()
		{
			this->Visibility = ::Visibility::Visible;
			DevToolsShown();
		}

		void WebViewEx::OnLoadCompleted()
		{
			LoadCompleted(this, EventArgs::Empty);
		}

		void WebViewEx::ShowDevTools()
		{
			showingDevTools = true;
			WebView::ShowDevTools();
		}

		void WebViewEx::CloseDevTools()
		{
			devToolsView->Visibility = ::Visibility::Collapsed;
			//devToolsView->CloseBrowser();
			WebView::CloseDevTools();
		}
	}
}