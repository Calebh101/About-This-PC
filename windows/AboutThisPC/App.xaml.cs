using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
using Microsoft.UI.Xaml.Shapes;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.Foundation;
using Windows.Foundation.Collections;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace AboutThisPC
{
    /// <summary>
    /// Provides application-specific behavior to supplement the default Application class.
    /// </summary>
    public partial class App : Application
    {
        static public string Version = "0.0.0A";
        private Window? _window;

        public enum Windows
        {
            ten = 10,
            eleven = 11,
        }

        public static string GetDriveLetter()
        {
            return Environment.GetFolderPath(Environment.SpecialFolder.System)[..1];
        }

        public static Windows GetWindows()
        {
            var build = Environment.OSVersion.Version.Build;
            if (build > 22000) return Windows.eleven;
            else if (build > 10240) return Windows.ten;
            else throw new Exception("Invalid Windows build number: " + build);
        }

        /// <summary>
        /// Usage: App.GetWindows(App.GetWindows())
        /// Yes, I know it's cursed
        /// </summary>
        /// <param name="windows">[Windows] object to switch</param>
        /// <returns>String identifier (like "10" or "XP")</returns>
        public static string GetWindows(Windows windows)
        {
            switch (windows)
            {
                case Windows.ten: return "10";
                case Windows.eleven: return "11";
                default: return "12";
            }
        }

        public static string GetWindowsFeatureRelease()
        {
            var key = Registry.LocalMachine.OpenSubKey(@"SOFTWARE\Microsoft\Windows NT\CurrentVersion");
            if (key != null)
            {
                var releaseId = key.GetValue("ReleaseId") as string;
                var displayVersion = key.GetValue("DisplayVersion") as string;

                return displayVersion ?? releaseId ?? "Unknown";
            }
            return "Unknown";
        }

        public static string GetWindowsIconPath()
        {
            Windows windows = GetWindows();

            string generate(string input)
            {
                string path = Generator.GetIconPath("Windows/windows" + input + ".png");
                Logger.Verbose("Found Windows icon path: " + path);
                return path;
            }

            switch (windows)
            {
                case Windows.ten: return generate("10");
                case Windows.eleven: return generate("11");
                default: return generate("11");
            }
        }

        /// <summary>
        /// Initializes the singleton application object.  This is the first line of authored code
        /// executed, and as such is the logical equivalent of main() or WinMain().
        /// </summary>
        public App()
        {
            Logger.Print("Starting AboutThisPC version " + Version + "...", true);
            InitializeComponent();
        }

        /// <summary>
        /// Invoked when the application is launched.
        /// </summary>
        /// <param name="args">Details about the launch request and process.</param>
        protected override void OnLaunched(Microsoft.UI.Xaml.LaunchActivatedEventArgs args)
        {
            bool classic = false;
            string arguments = args.Arguments;

            if (!string.IsNullOrEmpty(arguments))
            {
                if (arguments.Contains("--classic"))
                {
                    classic = true;
                }
            }

#if DEBUG
            Logger.EnableLogging();
            Logger.EnableVerbose();
#endif

            _window = new MainWindow(classic);
            _window.Activate();
        }

        public class Dimensions(double Width, double Height)
        {
            public (double Width, double Height) Build()
            {
                return (Width, Height);
            }
        }

        public class Result(string id, string title, string value = "Unknown")
        {
            public string Id { get; set; } = id.Replace(" ", "_").Replace("\n", "_").ToLowerInvariant();
            public string Title { get; set; } = title;
            public string Value { get; set; } = value;
        }
    }
}
