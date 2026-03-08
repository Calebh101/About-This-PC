using SharpDX.DXGI;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reactive.Subjects;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using Windows.Devices.Power;

namespace AboutThisPC
{
    internal class Logger
    {
        public static int MaxLogSize = 500;

        private static Queue<string> logs = new();
        private static Subject<string> stream = new();

        private static bool isAttached = false;
        private static bool useLogging = false;
        private static bool useVerbose = false;

        [DllImport("kernel32.dll")]
        private static extern bool AttachConsole(int dwProcessId);

        public static bool Attach(bool quiet = false)
        {
            if (isAttached)
            {
                Warn("Console is already attached!");
                return false;
            }

            if (AttachConsole(-1))
            {
                isAttached = true;
                if (!quiet) OutputRaw("---- CONSOLE ATTACHED ----", true);
                return true;
            } else
            {
                Warn("Unable to attach to console!");
                return false;
            }
        }

        private static string EffectToString(int effect)
        {
            return "\u001b[" + effect + "m";
        }

        private static void Output(String prefix, Object input, bool pass, int effect = 0)
        {
            string output = "> " + prefix + " " + DateTime.UtcNow.ToString("o") + " >> " + EffectToString(effect) + input.ToString() + EffectToString(0);
            OutputRaw(output, pass);
        }

        private static void OutputRaw(string input, bool pass)
        {
            string raw = Regex.Replace(input, @"\u001b\[\d+m", "");
            logs.Enqueue(raw);
            if (logs.Count > MaxLogSize) logs.Dequeue();
            if (!pass) return;

            if (Debugger.IsAttached)
            {
                System.Diagnostics.Debug.WriteLine(raw);
            }
            else
            {
                Console.WriteLine(input);
            }
        }

        private static void OutputVerbose(Object input, bool pass)
        {
            Output("VBS", input, pass, 2);
        }

        static public void Raw(Object input)
        {
            OutputRaw(input?.ToString() ?? "", true);
        }

        public static void Print(Object input, bool always = false)
        {
            Output("LOG", input, useLogging || always);
        }

        public static void Warn(Object input)
        {
            Output("WRN", input, true, 33);
        }

        public static void Verbose(Object input, bool always = false)
        {
            Output("VBS", input, (useLogging && useVerbose) || always, 2);
        }

        public static void EnableLogging()
        {
            if (useLogging) return;
            useLogging = true;
            OutputVerbose("Logging is enabled", true);
        }

        public static void EnableVerbose()
        {
            if (useVerbose) return;
            useVerbose = true;
            OutputVerbose("Verbose is enabled", true);
        }

        public static void SetVerbose(bool status)
        {
            bool output = useVerbose == false && status == true;
            useVerbose = status;
            if (output) OutputVerbose("Verbose is enabled", true);
        }

        public static List<string> GetCurrent()
        {
            return logs.ToList();
        }

        public static Subject<string> GetStream()
        {
            return stream;
        }
    }
}
