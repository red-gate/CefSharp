using System;
using System.ComponentModel;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Threading;
using CefSharp.Example;
using CefSharp.Wpf;
using System.IO;
using System.Text;

namespace CefSharp.Wpf.Example
{
    public partial class MainWindow : Window, IExampleView
    {

        private const string resource_url = "http://test/resource/load";
        // file
        public event EventHandler ExitActivated;

        // edit
        public event EventHandler UndoActivated;
        public event EventHandler RedoActivated;
        public event EventHandler CutActivated;
        public event EventHandler CopyActivated;
        public event EventHandler PasteActivated;
        public event EventHandler DeleteActivated;
        public event EventHandler SelectAllActivated;

        // test
        public event EventHandler TestResourceLoadActivated;
        public event EventHandler TestSchemeLoadActivated;
        public event EventHandler TestExecuteScriptActivated;
        public event EventHandler TestEvaluateScriptActivated;
        public event EventHandler TestBindActivated;
        public event EventHandler TestConsoleMessageActivated;
        public event EventHandler TestTooltipActivated;
        public event EventHandler TestPopupActivated;
        public event EventHandler TestLoadStringActivated;

        // navigation
        public event Action<object, string> UrlActivated;
        public event EventHandler BackActivated;
        public event EventHandler ForwardActivated;

        private IDictionary<object, EventHandler> handlers;

        public MainWindow()
        {
            InitializeComponent();
            web_view.RequestResource += new RequestResourceHandler(OnRequestResource);
            web_view.DevToolsShowing += new DevToolsShowingHandler(web_view_DevToolsShowing);
            web_view.DevToolsShowed += new DevToolsShowedHandler(web_view_DevToolsShowing);
            var presenter = new ExamplePresenter(web_view, this,
                invoke => Dispatcher.BeginInvoke(invoke));

            handlers = new Dictionary<object, EventHandler>
            {
                // file
                { exitMenuItem, ExitActivated },

                // edit
                { undoMenuItem, UndoActivated },
                { redoMenuItem, RedoActivated },
                { cutMenuItem, CutActivated },
                { copyMenuItem, CopyActivated },
                { pasteMenuItem, PasteActivated },
                { deleteMenuItem, DeleteActivated },
                { selectAllMenuItem, SelectAllActivated },

                // test
                { testResourceLoadMenuItem, TestResourceLoadActivated },
                { testSchemeLoadMenuItem, TestSchemeLoadActivated },
                { testExecuteScriptMenuItem, TestExecuteScriptActivated },
                { testEvaluateScriptMenuItem, TestEvaluateScriptActivated },
                { testBindMenuItem, TestBindActivated },
                { testConsoleMessageMenuItem, TestConsoleMessageActivated },
                { testTooltipMenuItem, TestTooltipActivated },
                { testPopupMenuItem, TestPopupActivated },
                { testLoadStringItem, TestLoadStringActivated },

                // navigation
                { backButton, BackActivated },
                { forwardButton, ForwardActivated },
            };
        }

        void web_view_DevToolsShowing(object sender, DevToolsShowingEventArgs args)
        {
            debugWindow = new Window();
            debugWindow.Closed += debugWindow_Closed;
            args.ChangeParentWindow(debugWindow);
            debugWindow.Show();
        }
        private void OnShowDevToolsClick(object sender, RoutedEventArgs e)
        {
            if (debugWindow == null)
            {
                web_view.ShowDevTools();
            }
            else
            {
                debugWindow.Focus();
            }
        }


        void debugWindow_Closed(object sender, EventArgs e)
        {
            debugWindow.Closed -= debugWindow_Closed;
            web_view.CloseDevTools();
            debugWindow = null;
        }

        Window debugWindow = null;
        void web_view_DevToolsShowing(DevToolsControl devToolsControl)
        {

            debugWindow.Content = devToolsControl;
            //Grid.SetRow(devToolsControl, 1);
            //viewContainer.Children.Add(devToolsControl);
        }

        public void SetTitle(string title)
        {
            Title = title;
        }

        public void SetAddress(string address)
        {
            urlTextBox.Text = address;
        }

        public void SetCanGoBack(bool can_go_back)
        {
            backButton.IsEnabled = can_go_back;
        }

        public void SetCanGoForward(bool can_go_forward)
        {
            forwardButton.IsEnabled = can_go_forward;
        }

        public void SetIsLoading(bool is_loading)
        {

        }

        public void ExecuteScript(string script)
        {
            web_view.ExecuteScript(script);
        }

        public object EvaluateScript(string script)
        {
            return web_view.EvaluateScript(script);
        }

        public void DisplayOutput(string output)
        {
            outputLabel.Content = output;
        }


        private void OnRequestResource(IWebBrowser browser, IRequestResponse requestResponse)
        {
            IRequest request = requestResponse.Request;
            if (request.Url.StartsWith(resource_url))
            {
                Stream resourceStream = new MemoryStream(Encoding.UTF8.GetBytes(
                    "<html><body><h1>Success</h1><p>This document is loaded from a System.IO.Stream</p></body></html>"));
                requestResponse.RespondWith(resourceStream, "text/html");
            }
        }

        private void control_Activated(object sender, RoutedEventArgs e)
        {
            EventHandler handler;
            if (handlers.TryGetValue(sender, out handler) &&
                handler != null)
            {
                handler(this, EventArgs.Empty);
            }
        }

        private void urlTextBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key != Key.Enter)
            {
                return;
            }

            var handler = UrlActivated;
            if (handler != null)
            {
                handler(this, urlTextBox.Text);
            }
        }
    }
}
