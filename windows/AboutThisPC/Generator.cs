using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Media.Imaging;
using Microsoft.Win32;
using SharpDX.DXGI;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Management;
using System.Net;
using System.Net.Sockets;
using System.Resources;
using System.Runtime.InteropServices;
using Windows.ApplicationModel;

namespace AboutThisPC
{
    internal class Generator
    {
        public static List<App.Result> GetValues(bool overview = false)
        {
            var results = new List<App.Result>();
            string? serial = SearchFor("SerialNumber", "SELECT SerialNumber FROM Win32_BIOS");
            string? processor = GetCPU();
            var startupDisk = GetStartupDisk();
            var localIp = GetLocalIP();
            var graphics = GetGPU();
            var memory = GetMemory();

            if (processor != null) results.Add(new App.Result("processor", "Processor", processor));
            results.Add(new App.Result("graphics", "Graphics", graphics.Name + " " + Math.Round(graphics.Bytes / (1024.0 * 1024.0), 0).ToString() + "MiB"));
            results.Add(new App.Result("memory", "Memory", Math.Round(memory.Bytes / (1024.0 * 1024.0 * 1024.0), 0).ToString() + "GiB " + memory.Generation + " " + memory.FormFactor + " " + memory.Speed + " MT/s"));
            if (serial != null) results.Add(new App.Result("serial", "Serial", serial));
            results.Add(new App.Result("startup_disk", "Startup Disk", startupDisk.Name + " (" + startupDisk.Letter + ":) " + Math.Round(startupDisk.Bytes / (1000.0 * 1000.0 * 1000.0), 0).ToString() + "GB"));
            results.Add(new App.Result("local_ip", "Local IP" + (localIp.Count > 1 ? "s" : ""), string.Join(", ", localIp)));
            if (!overview) results.Add(new App.Result("windows", "Windows", GetWindowsString()));
            return results;
        }

        public static string GetWindowsString()
        {
            return "Windows " + App.GetWindows(App.GetWindows()) + " " + App.GetWindowsFeatureRelease();
        }

        public static (string Family, string Model) GetComputerDetails()
        {
            var chassis = GetChassis();
            var obj = SearchFor("SELECT Manufacturer,Model FROM Win32_ComputerSystem");
            if (obj == null) return ("Unknown " + chassis.Name, "Unknown");
            var manufacturer = (string)obj["Manufacturer"];
            var model = (string)obj["Model"];
            return ((manufacturer ?? "Unknown") + " " + chassis.Name,  model);
        }

        public static List<SupportUrl> GetSupportURLs()
        {
            var result = new List<SupportUrl>();

            result.Add(new SupportUrl("support", "Support", "https://support.microsoft.com/"));
            result.Add(new SupportUrl("community", "Community", "https://answers.microsoft.com"));

            result.Add(new SupportUrl("windows.help", "Windows Help", "https://support.microsoft.com/windows"));
            result.Add(new SupportUrl("windows.update", "Windows Update Troubleshooter", "https://support.microsoft.com/help/4027322/windows-update-troubleshooter"));
            result.Add(new SupportUrl("windows.download", "Windows Download", $"https://www.microsoft.com/software-download/windows{App.GetWindows(App.GetWindows())}"));

            result.Add(new SupportUrl("atpc.github", "GitHub", "https://github.com/Calebh101/About-This-PC", true));
            result.Add(new SupportUrl("atpc.github.issues", "GitHub Issues", "https://github.com/Calebh101/About-This-PC/issues", true));

            if (!Generator.GetActivated()) result.Add(new SupportUrl("windows.activate", "Windows Activation", "https://support.microsoft.com/help/12440/windows-10-activation"));
            Logger.Print("Generated " + result.Count + " support URLs");
            return result;
        }

        public class SupportUrl(string id, string title, string value, bool self = false)
        {
            public string Id { get; set; } = id.Replace(" ", "_").Replace("\n", "_").ToLowerInvariant();
            public string Title { get; set; } = title;
            public Uri Value { get; set; } = new Uri(value);
            public bool IsAboutThisPCLink = self;
        }

        public static bool GetActivated()
        {
            ManagementObject? result = SearchFor("SELECT LicenseStatus FROM SoftwareLicensingProduct WHERE PartialProductKey IS NOT NULL AND ApplicationID='55c92734-d682-4d71-983e-d6ec3f16059f'");
            if (result == null) return default;

            uint status = (uint)result["LicenseStatus"];
            Logger.Print("Found license status: " + status.ToString());
            return status == 1;
        }

        public static List<ManagementObject> SearchForAll(string query)
        {
            try
            {
                List<ManagementObject> items = new List<ManagementObject>();
                ManagementObjectSearcher searcher = new ManagementObjectSearcher(query);
                foreach (ManagementObject obj in searcher.Get().Cast<ManagementObject>()) items.Add(obj);
                return items;
            } catch (ManagementException e)
            {
                Logger.Warn("System.Management.ManagementException: " + e.Message + " (finding " + query + ")");
                return new List<ManagementObject>();
            }
        }

        public static ManagementObject? SearchFor(string query)
        {
            try
            {
                ManagementObjectSearcher searcher = new ManagementObjectSearcher(query);
                foreach (ManagementObject obj in searcher.Get().Cast<ManagementObject>()) return obj;
                return null;
            }
            catch (ManagementException e)
            {
                Logger.Warn("System.Management.ManagementException: " + e.Message + " (finding " + query + ")");
                return null;
            }
        }

        public static string? SearchFor(string item, string query)
        {
            ManagementObject? output = SearchFor(query);
            return output?[item]?.ToString();
        }

        public static (string Name, long Bytes, long Used, DriveType type) GetDisk(string letter)
        {
            return GetDisk(new DriveInfo(letter));
        }

        private static string GetDiskName(DriveInfo drive)
        {
            return drive.VolumeLabel;
        }

        public static (string Name, long Bytes, long Used, DriveType type) GetDisk(DriveInfo drive)
        {
            return (GetDiskName(drive), drive.TotalSize, drive.TotalSize - drive.TotalFreeSpace, drive.DriveType);
        }

        public static List<Drive> GetAllDisks()
        {
            List<DriveInfo> drives = [.. DriveInfo.GetDrives()];
            List<Drive> result = new();
            
            foreach (DriveInfo drive in drives)
            {
                result.Add(new Drive { Name = GetDiskName(drive), Letter = drive.Name.Replace(":", "").Replace("\\", ""), Bytes = drive.TotalSize, Used = drive.TotalSize - drive.TotalFreeSpace, Type = drive.DriveType, Ready = drive.IsReady });
            }

            return result;
        }

        private static string? GetCPU()
        {
            bool editCpuName = true; // If I get into legal trouble (doubtful) then I know where to go haha
            ManagementObject? data = SearchFor("SELECT Name,MaxClockSpeed FROM Win32_Processor");
            if (data == null) return null;
            string model = data["Name"]?.ToString() ?? "Unknown";
            if (editCpuName) model = model.Replace("(R)", "").Replace("(TM)", "");
            uint speed = (uint)(data["MaxClockSpeed"] ?? 0);
            return GetArchitecture() + " " + Math.Round(speed / 1000.0, 1).ToString() + "GHz " + model.Trim();
        }

        public static (string Letter, string Name, double Bytes) GetStartupDisk()
        {
            string letter = App.GetDriveLetter();
            var disk = GetDisk(letter);
            return (letter, disk.Name, disk.Bytes);
        }

        private static List<string> GetLocalIP()
        {
            List<string> items = [];

            foreach (var ip in Dns.GetHostEntry(Dns.GetHostName()).AddressList)
            {
                if (ip.AddressFamily == AddressFamily.InterNetwork)
                {
                    items.Add(ip.ToString());
                }
            }

            return items;
        }

        private static (string Name, ulong Bytes) GetGPU()
        {
            ManagementObject? item = SearchFor("SELECT Name,AdapterRAM FROM Win32_VideoController");
            if (item == null) return ("Unknown", 0);

            string name = item?["Name"]?.ToString() ?? "Unknown";
            ulong bytes;
            ToUlong(item?["AdapterRAM"] ?? 0, out bytes);

            using (var factory = new Factory1())
            {
                var adapter = factory.Adapters1.FirstOrDefault();
                if (adapter != null)
                {
                    var description = adapter.Description;
                    var vram = description.DedicatedVideoMemory;

                    if (description.Description != null) name = description.Description;
                    else Logger.Warn("Found no GPU adapter description! Falling back to default GPU property...");

                    if (vram != null && vram > 0) ToUlong(vram, out bytes);
                    else Logger.Warn("Found no GPU adapter VRAM! (Found " + vram + " bytes) Falling back to default GPU property...");
                } else
                {
                    Logger.Warn("Found no GPU adapters! Falling back to default GPU properties...");
                }
            }

            return (name, bytes);
        }

        private static (ulong Bytes, string Generation, string Speed, string FormFactor) GetMemory()
        {
            ManagementObject? item = SearchFor("SELECT * FROM Win32_PhysicalMemory");
            if (item == null) return (0, "Unknown", "Unknown", "Unknown");
            string speed = item["Speed"]?.ToString() ?? "Unknown";

            ulong capacity;
            ToUlong(item["Capacity"], out capacity);

            string type = (uint)item["SMBIOSMemoryType"] switch
            {
                20 => "DDR",
                21 => "DDR2",
                24 => "DDR3",
                26 => "DDR4",
                27 => "DDR5",
                _ => "Unknown",
            };

            string form = (ushort)item["FormFactor"] switch
            {
                // 0 => "Unknown",
                1 => "Other",
                2 => "SIP",
                3 => "DIP",
                4 => "ZIP",
                5 => "SOJ",
                // 6 => "Proprietary",
                7 => "SIMM",
                8 => "DIMM",
                9 => "TSOP",
                10 => "PGA",
                11 => "RIMM",
                12 => "SODIMM",
                13 => "SRIMM",
                14 => "FB-DIMM",
                15 => "Die",
                _ => "Unknown"
            };

            return (capacity, type, speed, form);
        }

        public static bool ToUlong(object? input, out ulong output)
        {
            output = 0;
            if (input == null) return false;
            if (input is ulong ul) { output = ul; return true; }
            if (input is long l) { output = (ulong)l; return true; }
            if (input is int i) { output = (ulong)i; return true; }
            return ulong.TryParse(input.ToString(), out output);
        }

        public static (string Name, string? Icon) GetChassis()
        {
            ManagementObject? item = SearchFor("SELECT ChassisTypes FROM Win32_SystemEnclosure");
            if (item == null) return ("Unknown", null);

            ushort[] types = (ushort[])item["ChassisTypes"];
            if (types.Length == 0) return ("Unknown", null);
            var details = GetChassisDetails(types.Select(x => (int)x).ToList());

            string iconName = details.Icon + ".png";
            string icon = GetIconPath("computers/" + iconName);

            Logger.Print("Found icon resource: " + icon);
            return (string.Join("/", details.Names), icon);
        }

        public static string GetIconPath(string path)
        {
            return $"ms-appx:///Assets/{path}";
        }

        public static (List<string> Names, List<int> Numbers, string Icon) GetChassisDetails(List<int> input)
        {
            List<Dictionary<string, dynamic>> output = [];
            List<string> icons = [];

            foreach (var type in input)
            {
                string name;
                string icon;

                switch (type) // From the DMI specification at https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.8.0.pdf
                {
                    case 3:
                    case 4:
                    case 7:
                    case 17:
                    case 23:
                    case 24:
                    case 25:
                    case 29:
                        name = "Desktop";
                        icon = "monitor";
                        break;

                    case 5:
                    case 6:
                    case 15:
                    case 16:
                    case 26:
                    case 28:
                    case 34:
                    case 35:
                        name = "Mini PC";
                        icon = "monitor";
                        break;

                    case 8:
                    case 9:
                        name = "Laptop";
                        icon = "laptop";
                        break;


                    case 10:
                    case 14:
                    case 31:
                    case 32:
                        name = "Notebook";
                        icon = "laptop";
                        break;

                    case 11:
                    case 30:
                        name = "Handheld";
                        icon = "laptop";
                        break;

                    case 13:
                        name = "All-in-One";
                        icon = "monitor";
                        break;

                    default: // 0, 1, 12, 18 - 22, 33
                        name = "Computer";
                        icon = "monitor";
                        break;
                }

                Dictionary<string, dynamic> result = new Dictionary<string, dynamic>();
                result["name"] = name;
                result["icon"] = icon;
                result["number"] = type;

                icons.Add(icon);
                output.Add(result);
            }

            string iconChosen = icons.GroupBy(s => s).OrderByDescending(g => g.Count()).ThenBy(g => g.Key).First().Key;
            return (output.Select(x => (string)x["name"].ToString()).ToList<string>(), output.Select(x => (int)x["number"]).ToList<int>(), iconChosen);
        }

        static string GetArchitecture()
        {
            string arch = RuntimeInformation.OSArchitecture.ToString();

            if (arch == "X64") return "x64";
            if (arch == "X86") return "x86";
            if (arch == "Arm64") return "ARM64";
            if (arch == "Arm") return "ARM";

            return "Unknown";
        }

        public static List<string> GetBottomText()
        {
            return [
                "About This PC " + App.Version + " by Calebh101",
            ];
        }

        public static (string Pretty, string Build, string FeatureRelease) GetWindows()
        {
            string pretty = "Windows " + App.GetWindows(App.GetWindows());
            string build = Environment.OSVersion.Version.Build.ToString();
            pretty = SearchFor("Caption", "SELECT Caption FROM Win32_OperatingSystem")?.Replace("Microsoft", "").Trim() ?? pretty;
            return (pretty, build, App.GetWindowsFeatureRelease());
        }
    }
}
