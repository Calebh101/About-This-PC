using Microsoft.UI.Text;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Media.Imaging;
using Microsoft.UI.Xaml.Navigation;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Text;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace AboutThisPC
{
    // A *lot* of this code was copied from [ClassicPage.xaml.cs]
    public sealed partial class OverviewPage : Page
    {
        private Dictionary<string, TextBlock> widgets = [];
        private List<App.Result> widgetValues = [];

        public OverviewPage()
        {
            InitializeComponent();
            List<App.Result> results = Generator.GetValues(true);
            var details = Generator.GetComputerDetails();
            widgetValues = results;
            OverviewStackPanelColumn.Children.Clear();
            double fontSize = 13;
            ComputerModelSub.FontSize = fontSize;

            var windowsInfo = Generator.GetWindows();
            OSVersion.Text = windowsInfo.Pretty;
            OSVersionSub.Text = "Version " + windowsInfo.FeatureRelease + " (" + windowsInfo.Build + ")";

            FontWeight? boldWeight = new FontWeight(625);
            if (boldWeight != null) ComputerModelSub.FontWeight = (FontWeight)boldWeight;

            foreach (var item in results)
            {
                string key = item.Title;
                string value = item.Value;
                string id = "Results(" + item.Id + ")"; // Yes, I make stuff complicated for no reason, sue me

                var column = new StackPanel
                {
                    Orientation = Orientation.Horizontal,
                    HorizontalAlignment = HorizontalAlignment.Left,
                    Spacing = 6,
                };

                var blockA = new TextBlock
                {
                    Text = key,
                    FontSize = fontSize,
                };

                var blockB = new TextBlock
                {
                    Text = value,
                    FontSize = fontSize,
                };

                if (boldWeight != null) blockB.FontWeight = (FontWeight)boldWeight;
                column.Children.Add(blockA);
                column.Children.Add(blockB);
                OverviewStackPanelColumn.Children.Add(column);
                widgets[id] = blockB;
            }

            foreach (string text in Generator.GetBottomText())
            {
                var block = new TextBlock();
                block.Text = text;
                block.FontSize = 13;
                block.Opacity = 0.75;
                block.HorizontalAlignment = HorizontalAlignment.Center;
                OverviewStackPanelBottomText.Children.Add(block);
            }

            ComputerModelSub.Text = details.Model;
            string windowsIcon = App.GetWindowsIconPath();
            WindowsIcon.Source = new BitmapImage(new Uri(windowsIcon));
        }

        private void EyeButtonClick(object sender, RoutedEventArgs e)
        {
            bool isEye = EyeIconLine.Visibility == Visibility.Collapsed;
            EyeIconLine.Visibility = isEye ? Visibility.Visible : Visibility.Collapsed;
            TextBlock? serialBlock = widgets["Results(serial)"];
            serialBlock.Text = isEye ? (widgetValues.Find((x) => x.Id == "serial")!.Value) : "";
            TextBlock? ipBlock = widgets["Results(local_iP)"];
            ipBlock.Text = isEye ? (widgetValues.Find((x) => x.Id == "local_ip")!.Value) : "";
        }
    }
}
