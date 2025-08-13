using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Documents;
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
using Windows.System;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace AboutThisPC
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class SupportPage : Page
    {
        public SupportPage()
        {
            InitializeComponent();
            var results = Generator.GetSupportURLs();
            var columns = new[] { SupportLinks1, SupportLinks2 };
            bool spreadInColumns = false;

            for (int i = 0; i < results.Count; i++)
            {
                var item = results[i];
                var id = item.Id;
                var key = item.Title;
                var value = item.Value;
                var self = item.IsAboutThisPCLink;
                var column = spreadInColumns ? i % columns.Length : (int)Math.Round((decimal)((decimal)i / results.Count));

                var block = new TextBlock
                {
                    HorizontalAlignment = HorizontalAlignment.Center,
                    IsTextSelectionEnabled = true,
                };

                var button = new Hyperlink
                {
                    Inlines =
                    {
                        new Run
                        {
                            Text = self ? ("About This PC " + key) : key,
                        }
                    }
                };

                button.Click += async (sender, e) =>
                {
                    Uri uri = value;
                    Logger.Print("Launching support link: " + uri.ToString());
                    await Launcher.LaunchUriAsync(uri);
                };

                block.Inlines.Add(button);
                ToolTipService.SetToolTip(block, self ? (value + " (About This PC)") : value);
                (columns[column]).Children.Add(block);
            }

            foreach (string text in Generator.GetBottomText())
            {
                var block = new TextBlock();
                block.Text = text;
                block.FontSize = 13;
                block.Opacity = 0.75;
                block.HorizontalAlignment = HorizontalAlignment.Center;
                BottomText.Children.Add(block);
            }
        }
    }
}
