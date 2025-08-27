using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
using Microsoft.UI.Xaml.Shapes;
using Microsoft.Win32;
using Microsoft.Windows.ApplicationModel.DynamicDependency;
using SharpDX;
using Windows.Win32;
using Windows.Win32.Foundation;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text.Json;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Graphics;
using Windows.System;
using Microsoft.UI.Windowing;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace AboutThisPC
{
    /// <summary>
    /// Provides application-specific behavior to supplement the default Application class.
    /// </summary>
    public partial class App : Application
    {
        public static bool reverseEyeButtons = true; // TODO
        static public Dimensions? dimensions;
        static public string Version = "0.0.0A-R5";
        static public Settings? settings;
        private Window? _window;

        public enum Windows
        {
            ten = 10,
            eleven = 11,
        }

        public static string GetDriveLetter()
        {
            return Environment.GetFolderPath(Environment.SpecialFolder.System)[..1].Replace(":", "").Replace("\\", "");
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
            return windows switch
            {
                Windows.ten => "10",
                Windows.eleven => "11",
                _ => "12",
            };
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
#if DEBUG
            Logger.EnableLogging();
            Logger.SetVerbose(true);
#endif

            InitializeComponent();
        }

        /// <summary>
        /// Invoked when the application is launched.
        /// </summary>
        /// <param name="args">Details about the launch request and process.</param>
        protected override void OnLaunched(Microsoft.UI.Xaml.LaunchActivatedEventArgs args)
        {
            bool classic = false;
            string[] arguments = Environment.GetCommandLineArgs();
            Logger.Print("Found arguments: " + string.Join(" ", arguments));

            if (arguments.Contains("--version"))
            {
                Logger.Attach(true);
                Logger.Raw(Version);
                Current.Exit();
                return;
            }

            if (arguments.Contains("--debug"))
            {
                Logger.Attach();
                Logger.EnableLogging();
                Logger.EnableVerbose();
            }

            if (arguments.Contains("--classic"))
            {
                Logger.Print("Classic mode activated");
                classic = true;
            }

            settings = new Settings();

            if (arguments.Contains("--settings"))
            {
                Logger.Print("Showing settings...");
                SettingsPage.Window();
                return;
            }

            Logger.Print("Starting AboutThisPC version " + Version + "...", true);
            _window = new MainWindow(classic);
            _window.Activate();
        }

        public static double getGigabytes(long bytes)
        {
            return formatBytes(bytes, 1000, 3);
        }

        public static double getGigabits(long bytes)
        {
            return formatBytes(bytes, 1024, 3);
        }

        public static double getMegabytes(long bytes)
        {
            return formatBytes(bytes, 1000, 2);
        }

        public static double getMegabits(long bytes)
        {
            return formatBytes(bytes, 1024, 2);
        }

        public static double formatBytes(long bytes, double factor, int rounds)
        {
            if (bytes < 0 || rounds < 0) return default;
            double divisor = Math.Pow(factor, rounds);
            return bytes / divisor;
        }

        public class Dimensions(double Width, double Height)
        {
            public SizeInt32 Build(AppWindow? window = null)
            {
                double dpi = PInvoke.GetDpiForWindow(new HWND((nint)(window ?? MainWindow.Window)!.Id.Value));
                double width = Width * (dpi / 92);
                double height = Height * (dpi / 92);
                return new SizeInt32((int)width, (int)height);
            }
        }

        public class Result(string id, string title, string value = "Unknown")
        {
            public string Id { get; set; } = id.Replace(" ", "_").Replace("\n", "_").ToLowerInvariant();
            public string Title { get; set; } = title;
            public string Value { get; set; } = value;
        }

        public static async Task<ContentDialogResult> Error(XamlRoot root, string message, object? code, bool warning = false, bool ok = false)
        {
            var dialogue = new ContentDialog
            {
                Title = warning ? "Warning" : "Error",
                Content = $"{message}\n\nCode: {code}",
                PrimaryButtonText = ok ? "OK" : null,
                CloseButtonText = "Close",
                XamlRoot = root,
            };

            return await dialogue.ShowAsync();
        }
    }

    public class Drive
    {
        public string Name { get; set; } = "Unknown";
        public string Letter { get; set; } = "Unknown";
        public long Bytes { get; set; } = (long)0;
        public long Used { get; set; } = (long)0;
        public DriveType Type { get; set; } = DriveType.Unknown;
        public bool Ready { get; set; } = false;
    }

    public class Version
    {
        private int aa;
        private int ab;
        private int ac;
        private int ba;
        private int ca;

        public Version(int major, int intermediate = 0, int minor = 0, int patch = 0, int release = 0)
        {
            aa = major;
            ab = intermediate;
            ac = minor;
            ba = patch;
            ca = release;
        }

        public override bool Equals(object? o)
        {
            if (o is Version)
            {
                return this == (Version)o;
            } else
            {
                return false;
            }
        }

        public override int GetHashCode()
        {
            return HashCode.Combine(aa, ab, ac, ba, ca);
        }

        public override string ToString()
        {
            return (string.Join(".", new List<string> { aa.ToString(), ab.ToString(), ac.ToString() }) + (char)(ba + 'A') + "-R" + ca.ToString());
        }

        public static bool operator ==(Version a, Version b)
        {
            return a.aa == b.aa && a.ab == b.ab && a.ac == b.ac && a.ba == b.ba && a.ca == b.ca;
        }

        public static bool operator !=(Version a, Version b)
        {
            return !(a == b);
        }

        public static bool operator >(Version a, Version b)
        {
            if (a.aa != b.aa) return a.aa > b.aa;
            if (a.ab != b.ab) return a.ab > b.ab;
            if (a.ac != b.ac) return a.ac > b.ac;
            if (a.ba != b.ba) return a.ba > b.ba;
            return a.ca > b.ca;
        }

        public static bool operator <(Version a, Version b)
        {
            if (a.aa != b.aa) return a.aa < b.aa;
            if (a.ab != b.ab) return a.ab < b.ab;
            if (a.ac != b.ac) return a.ac < b.ac;
            if (a.ba != b.ba) return a.ba < b.ba;
            return a.ca < b.ca;
        }

        public static bool operator >=(Version a, Version b)
        {
            return a > b || a == b;
        }

        public static bool operator <=(Version a, Version b)
        {
            return a < b || a == b;
        }

        public static Version Parse(string input)
        {
            int major = 0;
            int intermediate = 0;
            int minor = 0;
            int patch = 0;
            int release = 0;

            string[] areas = input.Split("-");
            string[] sections = areas[0].Split(".");

            if (sections.Length >= 3)
            {
                char[] text = sections[2].ToCharArray();
                string letters = "";
                string numbers = "";

                foreach (char c in text)
                {
                    if (char.IsDigit(c))
                    {
                        numbers.Append(c);
                    } else if (char.IsLetter(c))
                    {
                        letters.Append(c);
                    }
                }

                if (numbers.Length > 0)
                {
                    minor = int.Parse(numbers);
                }

                if (letters.Length == 1)
                {
                    char c = char.ToUpper(letters.ToCharArray().First());
                    patch = c - 'A';
                }
            }

            if (sections.Length >= 2) intermediate = int.Parse(sections[1]);
            if (sections.Length >= 1) major = int.Parse(sections[0]);
            if (areas.Length >= 2) release = int.Parse(areas[1].Replace("R", ""));
            return new Version(major, intermediate, minor, patch, release);
        }

        public static async Task<Version?> CheckForUpdates(XamlRoot root, bool inferred = true)
        {
            Logger.Print("Checking for updates...");
            Version current = Version.Parse(App.Version);
            Release? result = null;

            bool useBeta = App.settings!.Get<bool>("betaVersions");
            string url = "https://api.github.com/repos/Calebh101/About-This-PC/releases";
            using HttpClient client = new HttpClient();
            string architecture;

            if (RuntimeInformation.OSArchitecture == Architecture.X64)
            {
                architecture = "x64";
            }
            else if (RuntimeInformation.OSArchitecture == Architecture.Arm64)
            {
                architecture = "arm64";
            }
            else
            {
                architecture = "x64";
            }

            client.DefaultRequestHeaders.UserAgent.ParseAdd("AboutThisPC/" + App.Version);
            HttpResponseMessage response = await client.GetAsync(url);
            string content = (await response.Content.ReadAsStringAsync()).Trim();
            Logger.Print("Fetched response from URL " + url + " and status " + response.StatusCode);

            if (response.IsSuccessStatusCode)
            {
                Logger.Print("Response was successful");
            } else
            {
                Logger.Warn("Unable to fetch for updates: Bad status code: " + response.StatusCode + " with content: " + content);
                _ = App.Error(root, "We were unable to fetch for updates.\nStatus code: " + response.StatusCode, "BAD_STATUS");
                return null;
            }

            string json = content;
            List<Release> releases = JsonSerializer.Deserialize<List<Release>>(json) ?? [];

            if (releases.Count <= 0)
            {
                Logger.Warn("Unable to fetch for updates: Releases was null or empty");
                _ = App.Error(root, "We were unable to fetch for updates.", "NO_RESULTS");
                return null;
            }
            
            foreach (Release release in releases)
            {
                Version version = Version.Parse(release.TagName);
                bool status = false;
                Logger.Verbose("Scanning release " + version.ToString() + " " + (release.Prerelease ? "prerelease" : "release") + "... (current: " + current + ":" + (version > current) + ")");

                if (version <= current) break;
                if (useBeta == false && release.Prerelease) continue;

                foreach (Asset asset in release.Assets)
                {
                    Logger.Verbose("Scanning asset " + asset.Name + "... (arch: " + architecture + ")");
                    if (asset.Name.Contains("win-" + architecture) || asset.Name.Contains("win-" + "universal"))
                    {
                        status = true;
                        break;
                    }
                }

                if (status)
                {
                    Logger.Verbose("Release " + version + " passed");
                    result = release;
                    break;
                }
            }

            if (result == null)
            {
                var dialogue = new ContentDialog
                {
                    Title = "No Updates Found",
                    Content = "No new updates found.",
                    PrimaryButtonText = "OK",
                    XamlRoot = root,
                };

                if (inferred) _ = dialogue.ShowAsync();
                return null;
            } else
            {
                async void show(Release release)
                {
                    var dialogue = new ContentDialog
                    {
                        Title = "Updates Found",
                        Content = "A new update was found!\n\n" + result.Name + "\nVersion: " + result.TagName + " (" + (result.Prerelease ? "Beta" : "Release") + ")" + "\nReleased: " + result.PublishedAt.ToString("dddd, MMMM dd, yyyy") + "\n\n" + result.Body,
                        PrimaryButtonText = "Open",
                        CloseButtonText = "OK",
                        DefaultButton = ContentDialogButton.Close,
                        XamlRoot = root,
                    };

                    var output = await dialogue.ShowAsync();
                    Logger.Print("Showed dialogue: " + result);

                    if (output == ContentDialogResult.Primary)
                    {
                        Uri url = new Uri(release.HtmlUrl);
                        await Launcher.LaunchUriAsync(url);
                    }
                }

                show(result);
                return Version.Parse(result.TagName);
            }
        }
    }
}
