using System;
using System.Diagnostics;
using System.Text.RegularExpressions;

namespace AboutThisPC
{
    internal class Logger
    {
        private static bool useLogging = false;
        private static bool useVerbose = false;

        private static void Output(String prefix, Object input, int effect = 0)
        {
            string output = "> " + prefix + " " + DateTime.UtcNow.ToString("o") + " >> \\e[" + effect.ToString() + "m" + input.ToString() + "\\e[0m";

            if (Debugger.IsAttached)
            {
                string pattern = "\\\\e\\[\\d+m";
                System.Diagnostics.Debug.WriteLine(Regex.Replace(output, pattern, ""));
            } else
            {
                Console.WriteLine(output);
            }
        }

        private static void OutputVerbose(Object input)
        {
            Output("VBS", input, 2);
        }

        public static void Print(Object input, bool always = false)
        {
            if (!useLogging && !always) return;
            Output("LOG", input);
        }

        public static void Warn(Object input)
        {
            Output("WRN", input, 33);
        }

        public static void Verbose(Object input, bool always = false)
        {
            if ((!useLogging || !useVerbose) && !always) return;
            Output("VBS", input, 2);
        }

        public static void EnableLogging()
        {
            if (useLogging) return;
            useLogging = true;
            OutputVerbose("Logging is enabled");
        }

        public static void EnableVerbose()
        {
            if (useVerbose) return;
            useVerbose = true;
            OutputVerbose("Verbose is enabled");
        }
    }
}
