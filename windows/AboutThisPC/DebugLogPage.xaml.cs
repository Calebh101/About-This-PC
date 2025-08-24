using Microsoft.UI.Windowing;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;

namespace AboutThisPC
{
    public sealed partial class DebugLogPage : Page
    {
        public DebugLogPage()
        {
            InitializeComponent();
            MainText.Text = string.Join("\n", Logger.GetCurrent());

            Logger.GetStream().Subscribe(value =>
            {
                MainText.Text = MainText.Text + "\n" + value;
            });
        }

        public static async void Window()
        {
            var dimensions = new App.Dimensions(800, 500).Build();
            var window = new Window();
            var appwindow = window.AppWindow;
            var presenter = appwindow?.Presenter as OverlappedPresenter;

            appwindow?.Resize(dimensions);
            window.Content = new DebugLogPage();
            await MainWindow.SetIcon(appwindow);

            if (presenter != null)
            {
                presenter.PreferredMinimumWidth = dimensions.Width;
                presenter.PreferredMinimumHeight = dimensions.Height;
            }

            window.Title = "About This PC Debug Logs";
            window.Activate();
        }
    }
}
