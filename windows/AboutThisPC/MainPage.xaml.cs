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

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace AboutThisPC
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            InitializeComponent();
            Navigate("OverviewPage");
        }

        public void NavigationChanged(object sender, NavigationViewSelectionChangedEventArgs e)
        {
            if (e.SelectedItem is NavigationViewItem item && item.Tag is string page) Navigate(page);
        }

        private void Navigate(string page)
        {
            Type? type = page switch
            {
                "OverviewPage" => typeof(OverviewPage),
                "SupportPage" => typeof(SupportPage),
                "DrivesPage" => typeof(DrivesPage),
                _ => null,
            };

            if (type != null)
            {
                Logger.Verbose("Navigating to page: " + page + " (" + type + ")");
                MainContentFrame.Navigate(type);
            }
            else
            {
                Logger.Warn("Tried to navigate to invalid page: " + page + " (" + type + ")");
            }
        }
    }
}
