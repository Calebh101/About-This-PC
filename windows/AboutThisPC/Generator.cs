using Microsoft.UI.Xaml.Media.Imaging;
using System;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Management;
using System.Net;
using System.Net.Sockets;
using System.Resources;
using Windows.ApplicationModel;

namespace AboutThisPC
{
    internal class Generator
    {
        public static Dictionary<string, string> getValues()
        {
            var results = new Dictionary<string, string>();
            string? serial = SearchFor("SerialNumber", "SELECT SerialNumber FROM Win32_BIOS");
            string? processor = GetCPU();
            var startupDisk = GetStartupDisk();
            var localIp = GetLocalIP();
            var graphics = GetGPU();
            var memory = getMemory();

            if (processor != null) results["Processor"] = processor;
            results["Graphics"] = graphics.Name + " " + Math.Round(graphics.Bytes / (1000.0 * 1000.0), 0).ToString() + "MB";
            results["Memory"] = Math.Round(memory.Bytes / (1024.0 * 1024.0 * 1024.0), 0).ToString() + "GiB " + memory.Generation + " " + memory.FormFactor + " " + memory.Speed + " MT/s";
            if (serial != null) results["Serial"] = serial;
            results["Startup Disk"] = startupDisk.Name + " (" + startupDisk.Letter + ":) " + Math.Round(startupDisk.Bytes / (1000.0 * 1000.0 * 1000.0), 0).ToString() + "GB";
            if (localIp.Any()) results["Local IP"] = string.Join(", ", localIp);
            results["Windows"] = Environment.OSVersion.Version.ToString();
            return results;
        }

        public static Dictionary<string, string> getSupportURLs()
        {
            var results = new Dictionary<string, string>();
            results["Support"] = "https://support.microsoft.com/";
            results["Community"] = "https://answers.microsoft.com";

            results["Windows Help"] = "https://support.microsoft.com/windows";
            results["Windows Update Troubleshooter"] = "https://support.microsoft.com/help/4027322/windows-update-troubleshooter";
            results["Windows Activation"] = "https://support.microsoft.com/help/12440/windows-10-activation";
            results["Windows Security"] = "https://support.microsoft.com/windows/windows-security-help-2a0a2485-024a-44e6-90d6-1d26186e7759";
            results["Windows Recovery"] = "https://support.microsoft.com/windows/recovery-options-in-windows-10-f4b99a99-42c1-d704-86b7-52293d29f42b";
            results["Windows Installation Media"] = "https://www.microsoft.com/software-download/windows10";
            results["Windows 11 Download"] = "https://www.microsoft.com/software-download/windows11";

            // My own plugs hehe
            results["GitHub"] = "https://github.com/Calebh101/About-This-PC";
            results["GitHub Issues"] = "https://github.com/Calebh101/About-This-PC/issues";
            return results;
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

        private static (string Name, double Bytes) GetDisk(string letter)
        {
            ManagementObject? disk = SearchFor($"SELECT VolumeName,Size FROM Win32_LogicalDisk WHERE DeviceID = '{letter}:'");
            string name = disk?["VolumeName"]?.ToString() ?? "Unknown";
            object? size = disk?["size"];
            ulong bytes = 0;
            ToUlong(size, out bytes);
            return (name, bytes);
        }

        private static string? GetCPU()
        {
            ManagementObject? data = SearchFor("SELECT Name,MaxClockSpeed FROM Win32_Processor");
            if (data == null) return null;
            string model = data["Name"]?.ToString() ?? "Unknown";
            uint speed = (uint)(data["MaxClockSpeed"] ?? 0);
            return Math.Round(speed / 1000.0, 1).ToString() + "GHz " + model;
        }

        private static (string Letter, string Name, double Bytes) GetStartupDisk()
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
            ManagementObject? item = SearchFor("SELECT Name, AdapterRAM FROM Win32_VideoController");
            if (item == null) return ("Unknown", 0);

            string name = item?["Name"]?.ToString() ?? "Unknown";
            ulong bytes;
            ToUlong(item?["AdapterRAM"] ?? 0, out bytes);
            return (name, bytes);
        }

        private static (ulong Bytes, string Generation, string Speed, string FormFactor) getMemory()
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

        public static (string Name, string? Icon) getChassis()
        {
            ManagementObject? item = SearchFor("SELECT ChassisTypes FROM Win32_SystemEnclosure");
            if (item == null) return ("Unknown", null);

            ushort[] types = (ushort[])item["ChassisTypes"];
            if (types.Length == 0) return ("Unknown", null);
            var details = getChassisDetails(types.Select(x => (int)x).ToList());

            var rm = new ResourceManager("AboutThisPC.Resources", typeof(App).Assembly);
            string found = "ERR";

            try
            {
                found = rm.GetString(details.Icon)!;
            } catch (Exception e)
            {
                Logger.Warn("Error getting resource " + details.Icon + ": " + e.Message);
            }

            return (string.Join("/", details.Names), $"ms-appx:///{found.Split(';')[0].Replace('\\', '/')}");
        }

        public static (List<string> Names, List<int> Numbers, string Icon) getChassisDetails(List<int> input)
        {
            List<Dictionary<string, dynamic>> output = new List<Dictionary<string, dynamic>>();
            List<string> icons = new List<string>();

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
    }
}
