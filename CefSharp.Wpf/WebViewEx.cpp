
#include "WebViewEx.h"
#include "DevToolsWebView.h"
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
		
		CefRefPtr<RenderClientAdapter> WebViewEx::CreateClientAdapter()
		{
			if(!adapterInitialized)
			{
				adapterInitialized = true;
				CefRefPtr<RenderClientAdapter> newAdapter = new RenderClientAdapterEx(this);
				adapter = newAdapter .get();
			}
			return adapter.get();
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
			if(showingDevTools && urlString->StartsWith("chrome-devtools://"))
			{
				settings.user_style_sheet_enabled = false;
				IntPtr mwHandle = (IntPtr)Dispatcher->Invoke( gcnew GetMainWindowHandleHandler(this, &WebViewEx::GetMainWindowHandle));
				if(mwHandle != IntPtr::Zero)
				{
					RECT rect;
					CefWindowInfo window;
       
					window.SetAsOffScreen(static_cast<HWND>(mwHandle.ToPointer()));
					windowInfo = window;
					DevToolsWebView^ devToolsView = (DevToolsWebView^)Dispatcher->Invoke( gcnew Func<DevToolsWebView^>(this, &WebViewEx::CreateDevToolsView)) ;
					CefRefPtr<RenderClientAdapter> clientAdapter = devToolsView->CreateClientAdapter();
					client = clientAdapter;
				}
			}			
			return false;
		}

		DevToolsWebView^ WebViewEx::CreateDevToolsView()
		{
			return gcnew DevToolsWebView(this);
		}
		void WebViewEx::OnShowDevTools(DevToolsWebView^ view)
		{
			showingDevTools = false;
			DevToolsShown(view);
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
		bool WebViewEx::GetAuthCredentials(IWebBrowser^ browser, Uri^ serverAddress, bool isProxy, String^ realm, String^% username, String^% password)
		{
			return OnRequestAuthCredentials(browser, serverAddress, isProxy, realm, username, password);
		}
		bool WebViewEx::OnRequestAuthCredentials(IWebBrowser^ browserControl, Uri^ serverAddress, bool isProxy, String^ realm, String^% username, String^% password)
		{
			auto args = gcnew RequestAuthCredentialsEventArgs(serverAddress, isProxy, realm);
			RequestAuthCredentials(browserControl, args);
			username = args->Username;
			password = args->Password;
			return args->IsSuccessful;
		}

	}
}