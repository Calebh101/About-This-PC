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
using Microsoft.UI.Xaml.Media.Imaging;
using Microsoft.UI.Text;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace AboutThisPC
{
    public sealed partial class ClassicPage : Page
    {
        private Dictionary<string, TextBlock> widgets = [];
        private List<App.Result> widgetValues = [];

        public ClassicPage()
        {
            InitializeComponent();
            List<App.Result> results = Generator.GetValues();
            var details = Generator.GetComputerDetails();
            widgetValues = results;

            double spacingV = 1;
            double fontSize = 13;

            ClassicStackPanelColumn1.Children.Clear();
            ClassicStackPanelColumn2.Children.Clear();
            ClassicStackPanelColumn1.Spacing = spacingV;
            ClassicStackPanelColumn2.Spacing = spacingV;

            foreach (App.Result item in results)
            {
                string key = item.Title;
                string value = item.Value;
                string id = "Results(" + item.Id + ")"; // Yes, I make stuff complicated for no reason, sue me

                var blockA = new TextBlock
                {
                    Text = key,
                    HorizontalAlignment = HorizontalAlignment.Right,
                    FontSize = fontSize,
                };

                var blockB = new TextBlock
                {
                    Text = value,
                    HorizontalAlignment = HorizontalAlignment.Left,
                    FontSize = fontSize,
                    IsTextSelectionEnabled = true
                };

                ClassicStackPanelColumn1.Children.Add(blockA);
                ClassicStackPanelColumn2.Children.Add(blockB);
                widgets[id] = blockB;
            }

            foreach (string text in Generator.GetBottomText())
            {
                var block = new TextBlock();
                block.Text = text;
                block.FontSize = 10;
                block.Opacity = 0.75;
                block.HorizontalAlignment = HorizontalAlignment.Center;
                ClassicStackPanelBottomText.Children.Add(block);
            }

            ComputerModelMain.Text = details.Family;
            ComputerModelSub.Text = details.Model;

            var chassis = Generator.GetChassis();
            if (chassis.Icon != null) ComputerImage.Source = new BitmapImage(new Uri(chassis.Icon));
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

        private async void MoreInfoButtonClick(object sender, RoutedEventArgs e)
        {
            Uri uri = new Uri("ms-settings:about");
            await Windows.System.Launcher.LaunchUriAsync(uri);
        }
    }
}
