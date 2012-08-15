using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace CefSharp.Wpf.Example
{
	/// <summary>
	/// Interaction logic for NewWindow.xaml
	/// </summary>
	public partial class NewWindow : Window
	{
		private WebViewEx web_view;

		public NewWindow()
		{
			InitializeComponent();
			Loaded += new RoutedEventHandler(NewWindow_Loaded);
		}

		public void OnBtnMouseEnter(object sender, MouseEventArgs e)
		{
		}

		protected override void OnKeyUp(KeyEventArgs e)
		{
			if (e.Key == Key.F10)
			{
				e.Handled = true;
			}
			else
			{
				base.OnKeyUp(e);
			}
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			if(e.Key == Key.F10)
			{
				e.Handled = true;
			}
			else
			{
				base.OnKeyDown(e);
			}
		}
		void NewWindow_Loaded(object sender, RoutedEventArgs e)
		{
			this.web_view = new WebViewEx(@"C:\Users\varchev\AppData\Local\Telerik\BlackDragon\deyan.varchev_gmail.com\NewBlank\NewBlank\index.html", new BrowserSettings());
			this.viewCOntainer.Content = web_view;

			web_view.DevToolsShown += OnDevToolsShown;
		}

		void OnDevToolsShown(DevToolsWebView view)
		{
			DevToolsContainer.Content = view;
		}

		public void OnCloseDevTools(object sender, RoutedEventArgs e)
		{
			DevToolsContainer.Content = null;
			web_view.CloseDevTools();
		}

		public void OnShowDevTools(object sender, RoutedEventArgs e)
		{
			web_view.ShowDevTools();
		}

		private void OnShowDevToolsForDevTools(object sender, RoutedEventArgs e)
		{
			var devTools = DevToolsContainer.Content as DevToolsWebView;
			if (devTools != null)
			{
				devTools.ShowDevTools();
			}
		}


	}
}
