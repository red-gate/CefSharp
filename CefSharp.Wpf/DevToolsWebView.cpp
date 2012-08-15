#include "DevToolsWebView.h"
#include "DevToolsRenderClientAdapter.h"

namespace CefSharp
{
	namespace Wpf
	{
		CefRefPtr<RenderClientAdapter> DevToolsWebView::CreateClientAdapter()
		{
			if(!adapterInitialized)
			{
				adapterInitialized = true;
				CefRefPtr<RenderClientAdapter> newAdapter = new DevToolsRenderClientAdapter(this);
				adapter = newAdapter.get();
			}
			return adapter.get();
		}
	}
}
