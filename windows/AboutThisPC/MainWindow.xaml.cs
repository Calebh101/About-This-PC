using Microsoft.Graphics.Display;
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
        private bool redoTitlebar = true; // Make content expand into the title bar
        public static AppWindow? Window;

        public MainWindow(bool classic)
        {
            InitializeComponent();
            init(classic);
        }

        async void init(bool classic)
        {
            var appWindow = this.AppWindow;
            var presenter = appWindow.Presenter as OverlappedPresenter;

            if (classic) App.dimensions = new App.Dimensions(350, 500);
            else /* - */ App.dimensions = new App.Dimensions(525, 350);

            if (presenter != null)
            {
                presenter.IsResizable = false;
                presenter.IsMaximizable = false;
            }

            Window = appWindow;
            MainWindowGrid.ActualThemeChanged += SetTitleBarForeground;
            SizeInt32 dimensions = App.dimensions!.Build();
            Logger.Verbose("Window size detected: " + dimensions.ToString());
            appWindow?.Resize(dimensions);
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
                SetTitleBarForeground();
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

        public static async Task SetIcon(AppWindow? window)
        {
            if (window == null)
            {
                Logger.Warn("Unable to set icon! App window was null.");
                return;
            }

            string icon = Generator.GetIconPath("appicon.ico");
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

        public void SetTitleBarForeground()
        {
            if (!redoTitlebar) return;
            var color = (Windows.UI.Color)Application.Current.Resources["SystemBaseHighColor"];
            Window!.TitleBar.ForegroundColor = color;
        }

        public void SetTitleBarForeground(FrameworkElement sender, object args)
        {
            SetTitleBarForeground();
        }
    }
}
