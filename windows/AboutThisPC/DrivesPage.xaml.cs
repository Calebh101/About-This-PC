using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
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
    public sealed partial class DrivesPage : Page
    {
        public ObservableCollection<DriveObject> Drives { get; } = new();

        public DrivesPage()
        {
            InitializeComponent();
            int columns = 2;
            var drives = Generator.GetAllDisks();
            var dimensions = App.dimensions!.Build(); // Window dimensions
            double totalWidth = dimensions.Width - (10 * 2 /* window padding */) - 10 /* item margin */ - 10 /* extra spacing I needed to add */;

            DrivesPageGridLayout.MinItemWidth = totalWidth / (double)columns;
            var startupDisk = Generator.GetStartupDisk();

            foreach (var drive in drives)
            {
                List<string> attributes = new();
                attributes.Add(drive.Ready ? "Ready" : "Not Ready");

                if (startupDisk.Letter == drive.Letter)
                {
                    attributes.Add("Startup Disk");
                }

                switch (drive.Type)
                {
                    case DriveType.CDRom: attributes.Add("CDROM"); break;
                    case DriveType.Removable: attributes.Add("External"); break;
                    case DriveType.Network: attributes.Add("Network"); break;
                }

                Drives.Add(new DriveObject
                {
                    Title = drive.Letter + ": " + drive.Name,
                    Subtitle = attributes.Count > 0 ? String.Join(" - ", attributes) : null,
                    Pretty = Math.Round(App.getGigabytes(drive.Bytes)) + "GB (" + Math.Round(App.getGigabytes(drive.Used)) + "GB used)",
                    Bytes = Convert.ToDouble(drive.Bytes),
                    Used = Convert.ToDouble(drive.Used),
                    Drive = drive,
                });
            }
        }

        private void Border_Tapped(object sender, TappedRoutedEventArgs e)
        {
            Button? element = (Button?)sender;
            DriveObject? data = (DriveObject?)element?.Tag;
            Drive? drive = data?.Drive;

            if (drive == null)
            {
                Logger.Warn("Button data was null! Unable to load File Explorer! (Trace: " + String.Join("", (new dynamic?[] {element, data, drive}).Select(x => (x != null ? 1 : 0).ToString())));
                return;
            }

            string path = $"{drive!.Letter}:\\";
            Logger.Print("Navigating to Explorer at " + path);
            Process.Start("explorer.exe", path); // Navigate to File Explorer at [path]
        }
    }

    public class DriveObject
    {
        public Drive? Drive { get; set; }
        public string Title { get; set; } = "Unknown";
        public string? Subtitle { get; set; }
        public string Pretty { get; set; } = "Unknown";
        public double Bytes { get; set; } = (double)0;
        public double Used { get; set; } = (double)0;
    }
}
