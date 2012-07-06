#include "WebView.h"

namespace CefSharp
{
namespace Wpf
{
    void WebView::Initialize(String^ address, BrowserSettings^ settings)
    {
        if (!CEF::IsInitialized &&
            !CEF::Initialize(gcnew Settings))
        {
            throw gcnew InvalidOperationException("CEF::Initialize() failed");
        }

        Focusable = true;
        FocusVisualStyle = nullptr;
        IsTabStop = true;

        _settings = settings;

        _browserCore = gcnew BrowserCore(address);
        _browserCore->PropertyChanged +=
            gcnew PropertyChangedEventHandler(this, &WebView::BrowserCore_PropertyChanged);

        _scriptCore = new ScriptCore();
		_paintDelegate = gcnew ActionHandler(this, &WebView::SetBitmap);
		_paintPopupDelegate = gcnew ActionHandler(this, &WebView::SetPopupBitmap);
		_showPopupDelegate = gcnew Action<bool>(this, &WebView::ShowPopupImpl);
		_resizePopupDelegate = gcnew ActionHandler(this, &WebView::SetPopupSizeAndPositionImpl);

        ToolTip = _toolTip =
            gcnew System::Windows::Controls::ToolTip();
        _toolTip->StaysOpen = true;
        _toolTip->Visibility = ::Visibility::Collapsed;
        _toolTip->Closed +=
            gcnew RoutedEventHandler(this, &WebView::ToolTip_Closed);

        _timer = gcnew DispatcherTimer(DispatcherPriority::Render);
        _timer->Interval = TimeSpan::FromSeconds(0.5);
        _timer->Tick +=
            gcnew EventHandler(this, &WebView::Timer_Tick);
    }

    CefRefPtr<RenderClientAdapter> WebView::CreateClientAdapter()
    {
		return new RenderClientAdapter(this);
	}

    bool WebView::TryGetCefBrowser(CefRefPtr<CefBrowser>& browser)
    {
        if (_browserCore->IsBrowserInitialized)
        {
            browser = _clientAdapter->GetCefBrowser();
            return browser != nullptr;
        }
        else
        {
            return false;
        }
    }

    void WebView::SetCursor(SafeFileHandle^ handle)
    {
        Cursor = CursorInteropHelper::Create(handle);
    }

    void WebView::BrowserCore_PropertyChanged(Object^ sender, PropertyChangedEventArgs^ e)
    {
        if (e->PropertyName == "TooltipText")
        {
            _timer->Stop();

            if (String::IsNullOrEmpty(_browserCore->TooltipText))
            {
                Dispatcher->BeginInvoke(DispatcherPriority::Render,
                    gcnew Action<String^>(this, &WebView::SetTooltipText), nullptr);
            }
            else
            {
                _timer->Start();
            }
        }
    }

    void WebView::Timer_Tick(Object^ sender, EventArgs^ e)
    {
        _timer->Stop();
        SetTooltipText(_browserCore->TooltipText);
    }

    void WebView::ToolTip_Closed(Object^ sender, RoutedEventArgs^ e)
    {
        _toolTip->Visibility = ::Visibility::Collapsed;
        // set Placement to something other than PlacementMode::Mouse,
        // so that when we re-show the tooltip in SetTooltipText(),
        // the tooltip will be repositioned to the new mouse point.
        _toolTip->Placement = PlacementMode::Absolute;
    }

    IntPtr WebView::SourceHook(IntPtr hWnd, int message, IntPtr wParam, IntPtr lParam, bool% handled)
    {
        handled = false;

        switch(message)
        {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_CHAR:
        case WM_SYSCHAR:
        case WM_IME_CHAR:
            CefRefPtr<CefBrowser> browser;
            if (!IsFocused ||
                !TryGetCefBrowser(browser))
            {
                break;
            }

            CefBrowser::KeyType type;
            if (message == WM_CHAR)
                type = KT_CHAR;
            else if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
                type = KT_KEYDOWN;
            else if (message == WM_KEYUP || message == WM_SYSKEYUP)
                type = KT_KEYUP;

            bool sysChar =
                message == WM_SYSKEYDOWN ||
                message == WM_SYSKEYUP ||
                message == WM_SYSCHAR;

            bool imeChar =
                message == WM_IME_CHAR;

            browser->SendKeyEvent(type, wParam.ToInt32(), lParam.ToInt32(), sysChar, imeChar);
        }

        return IntPtr::Zero;
    }

    void WebView::SetBitmap()
    {
		InteropBitmap^ bitmap = _ibitmap;

		if(bitmap == nullptr) 
		{
			_image->Source = nullptr;
			GC::Collect(1);

			int stride = _width * PixelFormats::Bgr32.BitsPerPixel / 8;
            bitmap = (InteropBitmap^)Interop::Imaging::CreateBitmapSourceFromMemorySection(
                (IntPtr)_fileMappingHandle, _width, _height, PixelFormats::Bgr32, stride, 0);
			_image->Source = bitmap;
			_ibitmap = bitmap;
		}

		bitmap->Invalidate();
    }

    void WebView::SetPopupBitmap()
    {
		InteropBitmap^ bitmap = _popupIbitmap;

		if(bitmap == nullptr) 
		{
			_popupImage->Source = nullptr;
			GC::Collect(1);

			int stride = _popupWidth * PixelFormats::Bgr32.BitsPerPixel / 8;
            bitmap = (InteropBitmap^)Interop::Imaging::CreateBitmapSourceFromMemorySection(
                (IntPtr)_popupFileMappingHandle, _popupWidth, _popupHeight, PixelFormats::Bgr32, stride, 0);
			_popupImage->Source = bitmap;
			_popupIbitmap = bitmap;
		}

		bitmap->Invalidate();
    }
    void WebView::OnPreviewKey(KeyEventArgs^ e)
    {
        CefRefPtr<CefBrowser> browser;
        if (!TryGetCefBrowser(browser))
        {
            return;
        }

        if (e->Key == Key::Tab ||
            e->Key >= Key::Left && e->Key <= Key::Down)
        {
            CefBrowser::KeyType type = e->IsDown ? KT_KEYDOWN : KT_KEYUP;
            int key = KeyInterop::VirtualKeyFromKey(e->Key);
            browser->SendKeyEvent(type, key, 0, false, false);

            e->Handled = true;
        }
    }

    void WebView::OnMouseButton(MouseButtonEventArgs^ e)
    {
        CefRefPtr<CefBrowser> browser;
        if (!TryGetCefBrowser(browser))
        {
            return;
        }

        Point point = e->GetPosition(this);

        CefBrowser::MouseButtonType type;
        if (e->ChangedButton == MouseButton::Left)
            type = CefBrowser::MouseButtonType::MBT_LEFT;
        else if (e->ChangedButton == MouseButton::Middle)
            type = CefBrowser::MouseButtonType::MBT_MIDDLE;
        else
            type = CefBrowser::MouseButtonType::MBT_RIGHT;

        bool mouseUp = e->ButtonState == MouseButtonState::Released;

        browser->SendMouseClickEvent((int)point.X, (int)point.Y,
            type, mouseUp, e->ClickCount);
    }

    Size WebView::ArrangeOverride(Size size)
    {
        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            Point point = _matrix->Transform(Point(size.Width, size.Height));
            browser->SetSize(PET_VIEW, (int)point.X, (int)point.Y);
			HidePopup();
        }
        else
        {
            Dispatcher->BeginInvoke(DispatcherPriority::Loaded,
                gcnew ActionHandler(this, &WebView::InvalidateArrange));
        }

        return ContentControl::ArrangeOverride(size);
    }

    void WebView::OnGotFocus(RoutedEventArgs^ e)
    {
        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->SendFocusEvent(true);
        }

        ContentControl::OnGotFocus(e);
    }

    void WebView::OnLostFocus(RoutedEventArgs^ e)
    {
        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->SendFocusEvent(false);
        }
		
		HidePopup();

        ContentControl::OnLostFocus(e);
    }

    void WebView::OnPreviewKeyDown(KeyEventArgs^ e)
    {
        OnPreviewKey(e);
    }

    void WebView::OnPreviewKeyUp(KeyEventArgs^ e)
    {
        OnPreviewKey(e);
    }

    void WebView::OnMouseMove(MouseEventArgs^ e)
    {
        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            Point point = e->GetPosition(this);
            browser->SendMouseMoveEvent((int)point.X, (int)point.Y, false);
        }
    }

    void WebView::OnMouseWheel(MouseWheelEventArgs^ e)
    {
        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            Point point = e->GetPosition(this);
            browser->SendMouseWheelEvent((int)point.X, (int)point.Y, e->Delta);
        }
    }

    void WebView::OnMouseDown(MouseButtonEventArgs^ e)
    {
        Focus();
        OnMouseButton(e);
        Mouse::Capture(this);
    }

    void WebView::OnMouseUp(MouseButtonEventArgs^ e)
    {
        OnMouseButton(e);
		Mouse::Capture(nullptr);
    }

    void WebView::OnMouseLeave(MouseEventArgs^ e)
    {
        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->SendMouseMoveEvent(0, 0, true);
        }

        _toolTip->IsOpen = false;
    }

    void WebView::OnInitialized()
    {
        _browserCore->OnInitialized();
    }

    void WebView::Load(String^ url)
    {
        _browserCore->CheckBrowserInitialization();
        _browserCore->OnLoad();

        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->GetMainFrame()->LoadURL(toNative(url));
        }

    }

    void WebView::LoadHtml(String^ html)
    {
        _browserCore->CheckBrowserInitialization();
        _browserCore->OnLoad();
        _clientAdapter->GetCefBrowser()->GetMainFrame()->LoadString(toNative(html), toNative("about:blank"));
    }

    void WebView::Stop()
    {
        _browserCore->CheckBrowserInitialization();

        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->StopLoad();
        }
    }

    void WebView::Back()
    {
        _browserCore->CheckBrowserInitialization();


        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->GoBack();
        }
    }

    void WebView::Forward()
    {
        _browserCore->CheckBrowserInitialization();

        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->GoForward();
        }
    }

    void WebView::Reload()
    {
        Reload(false);
    }

    void WebView::Reload(bool ignoreCache)
    {
        _browserCore->CheckBrowserInitialization();

        CefRefPtr<CefBrowser> browser;
        if (!TryGetCefBrowser(browser))
        {
            return;
        }

        if (ignoreCache)
        {
            browser->ReloadIgnoreCache();
        }
        else
        {
            browser->Reload();
        }
    }

    void WebView::ClearHistory()
    {
        _browserCore->CheckBrowserInitialization();

        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->ClearHistory();
        }
    }

    void WebView::ShowDevTools()
    {
        _browserCore->CheckBrowserInitialization();

        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->ShowDevTools();
        }
    }

    void WebView::CloseDevTools()
    {
        _browserCore->CheckBrowserInitialization();

        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->CloseDevTools();
        }
    }

    void WebView::Undo()
    {
        _browserCore->CheckBrowserInitialization();

        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->GetMainFrame()->Undo();
        }
    }

    void WebView::Redo()
    {
        _browserCore->CheckBrowserInitialization();

        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->GetMainFrame()->Redo();
        }
    }

    void WebView::Cut()
    {
        _browserCore->CheckBrowserInitialization();

        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->GetMainFrame()->Cut();
        }
    }

    void WebView::Copy()
    {
        _browserCore->CheckBrowserInitialization();

        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->GetMainFrame()->Copy();
        }
    }

    void WebView::Paste()
    {
        _browserCore->CheckBrowserInitialization();

        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->GetMainFrame()->Paste();
        }
    }

    void WebView::Delete()
    {
        _browserCore->CheckBrowserInitialization();

        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->GetMainFrame()->Delete();
        }
    }

    void WebView::SelectAll()
    {
        _browserCore->CheckBrowserInitialization();

        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->GetMainFrame()->SelectAll();
        }
    }

    void WebView::Print()
    {
        _browserCore->CheckBrowserInitialization();

        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->GetMainFrame()->Print();
        }
    }

    void WebView::ExecuteScript(String^ script)
    {
        _browserCore->CheckBrowserInitialization();

        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            _scriptCore->Execute(browser, toNative(script));
        }
    }

    Object^ WebView::EvaluateScript(String^ script)
    {
        return EvaluateScript(script, TimeSpan::MaxValue);
    }

    Object^ WebView::EvaluateScript(String^ script, TimeSpan timeout)
    {
	    _browserCore->CheckBrowserInitialization();

        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            return _scriptCore->Evaluate(browser, toNative(script),
                timeout.TotalMilliseconds);
        }
        else
        {
            return nullptr;
        }
    }

    void WebView::SetNavState(bool isLoading, bool canGoBack, bool canGoForward)
    {
        _browserCore->SetNavState(isLoading, canGoBack, canGoForward);
    }

    void WebView::OnConsoleMessage(String^ message, String^ source, int line)
    {
        ConsoleMessage(this, gcnew ConsoleMessageEventArgs(message, source, line));
    }

    void WebView::RegisterJsObject(String^ name, Object^ objectToBind)
    {
        _browserCore->RegisterJsObject(name, objectToBind);
    }

    IDictionary<String^, Object^>^ WebView::GetBoundObjects()
    {
        return _browserCore->GetBoundObjects();
    }

    void WebView::OnFrameLoadStart()
    {
        _browserCore->OnFrameLoadStart();
    }

    void WebView::OnFrameLoadEnd()
    {
        _browserCore->OnFrameLoadEnd();
    }

    void WebView::OnTakeFocus(bool next)
    {
        FocusNavigationDirection direction = next ?
            FocusNavigationDirection::Next :
            FocusNavigationDirection::Previous;
        TraversalRequest^ request = gcnew TraversalRequest(direction);

        Dispatcher->BeginInvoke(DispatcherPriority::Input,
            gcnew MoveFocusHandler(this, &WebView::MoveFocus), request);
    }

    void WebView::OnApplyTemplate()
    {
        ContentControl::OnApplyTemplate();

		_clientAdapter = CreateClientAdapter().get();

        HwndSource^ source = (HwndSource^)PresentationSource::FromVisual(this);
        HWND hwnd = static_cast<HWND>(source->Handle.ToPointer());

        CefWindowInfo window;
        window.SetAsOffScreen(hwnd);
        CefString url = toNative(_browserCore->Address);

        CefBrowser::CreateBrowser(window, _clientAdapter.get(),
            url, *_settings->_browserSettings);

        source->AddHook(gcnew Interop::HwndSourceHook(this, &WebView::SourceHook));

        Content = _image = gcnew Image();
		_popup = gcnew Popup();
		
        _popup->Child = _popupImage = gcnew Image();

		_popup->MouseDown += gcnew MouseButtonEventHandler(this, &WebView::OnPopupMouseDown);
		_popup->MouseUp += gcnew MouseButtonEventHandler(this, &WebView::OnPopupMouseUp);
		_popup->MouseMove += gcnew MouseEventHandler(this, &WebView::OnPopupMouseMove);
		//_popup->MouseEnter += gcnew MouseEventHandler(this, &WebView::OnPopupMouseEnter);
		_popup->MouseLeave += gcnew MouseEventHandler(this, &WebView::OnPopupMouseLeave);
		_popup->MouseWheel += gcnew MouseWheelEventHandler(this, &WebView::OnPopupMouseWheel);
		Window^ currentWindow = Window::GetWindow(this);
		currentWindow->LocationChanged += gcnew EventHandler(this, &WebView::OnWindowLocationChanged);
		currentWindow->Deactivated += gcnew EventHandler(this, &WebView::OnWindowLocationChanged);

		_popup->PlacementTarget = this;
		_popup->Placement = PlacementMode::Relative;
//		_popup->StaysOpen = false;

        _image->Stretch = Stretch::None;
        _image->HorizontalAlignment = ::HorizontalAlignment::Left;
        _image->VerticalAlignment = ::VerticalAlignment::Top;

        _popupImage->Stretch = Stretch::None;
        _popupImage->HorizontalAlignment = ::HorizontalAlignment::Left;
        _popupImage->VerticalAlignment = ::VerticalAlignment::Top;

        _matrix = source->CompositionTarget->TransformToDevice;
    }

    void WebView::SetCursor(CefCursorHandle cursor)
    {
        SafeFileHandle^ handle = gcnew SafeFileHandle((IntPtr)cursor, false);
        Dispatcher->BeginInvoke(DispatcherPriority::Render,
            gcnew Action<SafeFileHandle^>(this, &WebView::SetCursor), handle);
    }

    void WebView::SetTooltipText(String^ text)
    {
        if (String::IsNullOrEmpty(text))
        {
            _toolTip->IsOpen = false;
        }
        else
        {
            _toolTip->Content = text;
            _toolTip->Placement = PlacementMode::Mouse;
            _toolTip->Visibility = ::Visibility::Visible;
            _toolTip->IsOpen = true;
        }
    }

    void WebView::SetBuffer(int width, int height, const void* buffer)
    {
        if (!_backBufferHandle || _width != width || _height != height)
        {
			_ibitmap = nullptr;

			if (_backBufferHandle)
            {
                UnmapViewOfFile(_backBufferHandle);
                _backBufferHandle = NULL;
            }

            if (_fileMappingHandle)
            {
                CloseHandle(_fileMappingHandle);
                _fileMappingHandle = NULL;
            }

			int pixels = width * height;
            int bytes = pixels * PixelFormats::Bgr32.BitsPerPixel / 8;

			_fileMappingHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, bytes, NULL);
			if(!_fileMappingHandle) 
			{
				return;
			}

			_backBufferHandle = MapViewOfFile(_fileMappingHandle, FILE_MAP_ALL_ACCESS, 0, 0, bytes);
			if(!_backBufferHandle) 
			{
				return;
			}

			_width = width;
			_height = height;
        }
	
		int stride = width * PixelFormats::Bgr32.BitsPerPixel / 8;
		CopyMemory(_backBufferHandle, (void*) buffer, height * stride);
		
		if(!Dispatcher->HasShutdownStarted) {
			Dispatcher->BeginInvoke(DispatcherPriority::Render, _paintDelegate);
		}
    }

	void WebView::SetPopupBuffer(int width, int height, const void* buffer)
    {
        if (!_popupBackBufferHandle || _popupWidth != width || _popupHeight != height)
        {
			_popupIbitmap = nullptr;

			if (_popupBackBufferHandle)
            {
                UnmapViewOfFile(_popupBackBufferHandle);
                _popupBackBufferHandle = NULL;
            }

            if (_popupFileMappingHandle)
            {
                CloseHandle(_popupFileMappingHandle);
                _popupFileMappingHandle = NULL;
            }

			int pixels = width * height;
            int bytes = pixels * PixelFormats::Bgr32.BitsPerPixel / 8;

			_popupFileMappingHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, bytes, NULL);
			if(!_popupFileMappingHandle) 
			{
				return;
			}

			_popupBackBufferHandle = MapViewOfFile(_popupFileMappingHandle, FILE_MAP_ALL_ACCESS, 0, 0, bytes);
			if(!_popupBackBufferHandle) 
			{
				return;
			}

			_popupWidth = width;
			_popupHeight = height;
        }
	
		int stride = width * PixelFormats::Bgr32.BitsPerPixel / 8;
		CopyMemory(_popupBackBufferHandle, (void*) buffer, height * stride);
		
		if(!Dispatcher->HasShutdownStarted) {
			Dispatcher->BeginInvoke(DispatcherPriority::Render, _paintPopupDelegate);
		}
    }

	void WebView::SetPopupSizeAndPosition(const CefRect& rect)
	{
		_popupX = rect.x;
		_popupY = rect.y;
		_popupWidth = rect.width;
		_popupHeight = rect.height;
		
		if(!Dispatcher->HasShutdownStarted) {
			Dispatcher->BeginInvoke(DispatcherPriority::Render, _resizePopupDelegate);
		}
	}
	void WebView::SetPopupIsOpened(bool isOpened){
		
		if(!Dispatcher->HasShutdownStarted) {
			Dispatcher->BeginInvoke(_showPopupDelegate, DispatcherPriority::Render, isOpened);
		}

	}

	void WebView::GetScreenPoint(int relativeX, int relativeY, int& screenX, int& screenY){
		Point point;
		if(!Dispatcher->HasShutdownStarted) {
			point =(Point) Dispatcher->Invoke(gcnew Func<int, int, Point>(this, &WebView::GetScreenPointImpl), DispatcherPriority::Render, relativeX, relativeY);
		}
		screenX = point.X;
		screenY = point.Y;
	}

	Point WebView::GetScreenPointImpl(int x, int y)
	{

		Point locationFromScreen = this->PointToScreen(Point(0, 0));
		Point targetPoints = _matrix->Transform(locationFromScreen);

		return Point(targetPoints.X + x,targetPoints.Y + y);
	}

	void WebView::SetPopupSizeAndPositionImpl()
	{
		_popup->Width = _popupWidth;
		_popup->Height = _popupHeight;

		_popup->HorizontalOffset = _popupX;
		_popup->VerticalOffset = _popupY;



	}
	void WebView::ShowPopupImpl(bool isOpened)
	{
		_popup->IsOpen = isOpened;
	}


	void WebView::OnPopupMouseMove(Object^ sender, MouseEventArgs^ e)
    {
        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            Point point = e->GetPosition(this);

            browser->SendMouseMoveEvent((int)point.X, (int)point.Y, false);
        }
    }

    void WebView::OnPopupMouseWheel(Object^ sender, MouseWheelEventArgs^ e)
    {
        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            Point point = e->GetPosition(this);
	
            browser->SendMouseWheelEvent((int)point.X, (int)point.Y, e->Delta);
        }
    }

    void WebView::OnPopupMouseDown(Object^ sender,MouseButtonEventArgs^ e)
    {
        OnPopupMouseButton(e);
        Mouse::Capture(this);
    }

    void WebView::OnPopupMouseUp(Object^ sender,MouseButtonEventArgs^ e)
    {
        OnPopupMouseButton(e);
        Mouse::Capture(nullptr);
    }

    void WebView::OnPopupMouseLeave(Object^ sender,MouseEventArgs^ e)
    {
        CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {
            browser->SendMouseMoveEvent(0, 0, true);
        }
    }
	
	void WebView::OnWindowLocationChanged(Object^ sender, EventArgs^ e)
    { 
		HidePopup();
	}

	void WebView::HidePopup()
	{
		CefRefPtr<CefBrowser> browser;
        if (TryGetCefBrowser(browser))
        {           
			if(_popup != nullptr && _popup->IsOpen)
			{
				browser->SendMouseClickEvent(-1,-1, CefBrowser::MouseButtonType::MBT_LEFT, false, 1 );
			}
		}
	}
	
    void WebView::OnPopupMouseButton(MouseButtonEventArgs^ e)
    {
        CefRefPtr<CefBrowser> browser;
        if (!TryGetCefBrowser(browser))
        {
            return;
        }

        Point point = e->GetPosition(this);
	
        CefBrowser::MouseButtonType type;
        if (e->ChangedButton == MouseButton::Left)
            type = CefBrowser::MouseButtonType::MBT_LEFT;
        else if (e->ChangedButton == MouseButton::Middle)
            type = CefBrowser::MouseButtonType::MBT_MIDDLE;
        else
            type = CefBrowser::MouseButtonType::MBT_RIGHT;

        bool mouseUp = e->ButtonState == MouseButtonState::Released;

        browser->SendMouseClickEvent((int)point.X, (int)point.Y,
            type, mouseUp, e->ClickCount);
    }

	

 

}}