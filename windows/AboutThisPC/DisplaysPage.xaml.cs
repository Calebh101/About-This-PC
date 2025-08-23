using Microsoft.Graphics.Display;
using Microsoft.UI;
using Microsoft.UI.Windowing;
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
using System.Collections.ObjectModel;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Management;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Devices.Display;
using Windows.Foundation;
using Windows.Foundation.Collections;

[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
public struct MONITORINFOEX
{
    public int cbSize;
    public RECT rcMonitor;
    public RECT rcWork;
    public uint dwFlags;

    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
    public string szDevice;
}

[StructLayout(LayoutKind.Sequential)]
public struct RECT
{
    public int left;
    public int top;
    public int right;
    public int bottom;
}

[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
public struct DISPLAY_DEVICE
{
    public int cb;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
    public string DeviceName;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
    public string DeviceString;
    public int StateFlags;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
    public string DeviceID;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
    public string DeviceKey;
}

namespace AboutThisPC
{
    public static class MonitorHelper
    {
        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool GetMonitorInfo(IntPtr hMonitor, ref MONITORINFOEX lpmi);

        [DllImport("user32.dll", CharSet = CharSet.Ansi)]
        public static extern bool EnumDisplayDevices(string lpDevice, uint iDevNum, ref DISPLAY_DEVICE lpDisplayDevice, uint dwFlags);

        [DllImport("user32.dll")]
        private static extern int GetSystemMetrics(int nIndex);
    }

    public sealed partial class DisplaysPage : Page
    {
        public ObservableCollection<DisplayObject> Displays { get; } = new();

        public DisplaysPage()
        {
            InitializeComponent();
            var displays = GetDisplays();

            foreach (var display in displays)
            {
                List<string> attributes = new();
                var dimensions = display.Data.OuterBounds;
                attributes.Add(string.Join("x", new List<string> { dimensions.Width.ToString(), dimensions.Height.ToString() }));
                if (display.Data.IsPrimary) attributes.Add("Primary");

                string icon = "monitor";
                Uri iconPath = new Uri(Generator.GetIconPath("computers/" + icon + ".png"));

                Displays.Add(new DisplayObject
                {
                    Data = display,
                    Subtitle = string.Join(" - ", attributes),
                    Icon = new BitmapImage(iconPath),
                });
            }
        }

        public static List<Display> GetDisplays()
        {
            var displays = DisplayArea.FindAll();
            List<Display> result = [];

            for (int i = 0; i < displays.Count; i++)
            {
                DisplayArea display = displays[i];
                result.Add(new Display(display));
            }

            return result;
        }
    }

    public class Display
    {
        public DisplayArea Data { get; }
        public DisplayInformation Information { get; }
        public string Path { get; }
        //public double Diagonal { get; }

        public Display(DisplayArea data)
        {
            Data = data;
            Information = DisplayInformation.CreateForDisplayId(Data.DisplayId);

            IntPtr hMonitor = new IntPtr((int)Data.DisplayId.Value);
            MONITORINFOEX info = new MONITORINFOEX();
            info.cbSize = Marshal.SizeOf<MONITORINFOEX>();
            MonitorHelper.GetMonitorInfo(hMonitor, ref info);
            Path = info.szDevice;

            int width = Data.WorkArea.Width;
            int height = Data.WorkArea.Height;
            int pixels = (int)Math.Round(Math.Sqrt((width * width) + (height * height)));
        }
    }

    public class DisplayObject
    {
        public Display? Data { get; set; }
        public string? Name { get; set; }
        public string? Subtitle { get; set; }
        public BitmapImage? Icon { get; set; }
    }
}
