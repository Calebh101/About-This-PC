using Microsoft.UI;
using Microsoft.UI.Windowing;
using Microsoft.UI.Xaml;
using System;
using System.Threading.Tasks;
using Windows.Graphics;
using Windows.Storage;
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
            InitializeComponent();
            init(classic);
        }

        async void init(bool classic)
        {
            bool redoTitlebar = true; // Make content expand into the title bar
            App.Dimensions dimensions;

            var hWnd = WindowNative.GetWindowHandle(this);
            var windowId = Win32Interop.GetWindowIdFromWindow(hWnd);
            var appWindow = AppWindow.GetFromWindowId(windowId);
            var presenter = appWindow.Presenter as OverlappedPresenter;

            if (classic) dimensions = new App.Dimensions(350, 500);
            else /* - */ dimensions = new App.Dimensions(600, 375);

            if (presenter != null)
            {
                presenter.IsResizable = false;
                presenter.IsMaximizable = false;
            }

            App.dimensions = dimensions;
            var (width, height) = dimensions.Build();
            Logger.Verbose("Window size detected: " + width + "x" + height);
            appWindow?.Resize(new SizeInt32((int)width, (int)height));
            if (redoTitlebar) this.ExtendsContentIntoTitleBar = true;
            await SetIcon(appWindow);

            if (appWindow == null)
            {
                Logger.Warn("appWindow is null! We won't be able to apply properties!");
            }

            if (redoTitlebar && appWindow != null)
            {
                appWindow.TitleBar.BackgroundColor = Windows.UI.Color.FromArgb(0, 0, 0, 0);
                appWindow.TitleBar.ButtonBackgroundColor = Windows.UI.Color.FromArgb(0, 0, 0, 0);
            }

            if (classic)
            {
                MainWindowGrid.Margin = new Thickness(0, 32, 0, 0);
                MainFrame.Navigate(typeof(ClassicPage));
            }
            else
            {
                MainWindowGrid.Margin = new Thickness(0, 0, 0, 0);
                MainFrame.Navigate(typeof(MainPage));
            }
        }

        private static async Task SetIcon(AppWindow? window)
        {
            if (window == null)
            {
                Logger.Warn("Unable to set icon! App window was null.");
                return;
            }

            string icon = Generator.GetIconPath("AppIcon1.ico");
            StorageFile? storageFile = null;

            try
            {
                storageFile = await StorageFile.GetFileFromApplicationUriAsync(new Uri(icon));
            }
            catch (Exception e)
            {
                Logger.Warn("Unable to set icon! Unable to set storage file: " + e.Message);
            }

            if (storageFile != null)
            {
                string path = storageFile.Path;
                window.SetIcon(path);
                window.SetTaskbarIcon(path);
                window.SetTitleBarIcon(path);
            }
        }
    }
}
