using Microsoft.UI;
using Microsoft.UI.Windowing;
using Microsoft.UI.Xaml;
using System;
using Windows.Graphics;
using WinRT.Interop;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace AboutThisPC
{
    /// <summary>
    /// An empty window that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainWindow : Window
    {
        public MainWindow(bool classic)
        {
            bool redoTitlebar = true; // Make content expand into it
            InitializeComponent();

            int width;
            int height;

            var hWnd = WindowNative.GetWindowHandle(this);
            var windowId = Win32Interop.GetWindowIdFromWindow(hWnd);
            var appWindow = AppWindow.GetFromWindowId(windowId);
            var presenter = appWindow.Presenter as OverlappedPresenter;

            if (classic)
            {
                width = 350;
                height = 500;
            }
            else
            {
                width = 600;
                height = 300;
            }

            if (presenter != null)
            {
                presenter.IsResizable = false;
                presenter.IsMaximizable = false;
            }

            Logger.Verbose("Window size detected: " + width + "x" + height);
            appWindow?.Resize(new SizeInt32(width, height));
            if (redoTitlebar) this.ExtendsContentIntoTitleBar = true;

            if (redoTitlebar && appWindow != null)
            {
                appWindow.TitleBar.BackgroundColor = Windows.UI.Color.FromArgb(0, 0, 0, 0);
                appWindow.TitleBar.ButtonBackgroundColor = Windows.UI.Color.FromArgb(0, 0, 0, 0);
            }

            if (classic)
            {
                MainWindowGrid.Margin = new Thickness(0, 32, 0, 0);
                MainFrame.Navigate(typeof(ClassicPage));
            } else
            {
                MainWindowGrid.Margin = new Thickness(0, 0, 0, 0);
                MainFrame.Navigate(typeof(MainPage));
            }
        }
    }
}
