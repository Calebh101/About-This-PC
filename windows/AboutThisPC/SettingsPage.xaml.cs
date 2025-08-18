using Microsoft.UI;
using Microsoft.UI.Windowing;
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
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text.Json;
using System.Xml.Linq;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace AboutThisPC
{
    public class Settings
    {
        public static DirectoryInfo Directory = GetDirectory();
        public static string File = Path.Combine(Directory.FullName, "settings.json");
        private Dictionary<string, object?> settings;

        public static DirectoryInfo GetDirectory()
        {
            string path = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "AboutThisPC");
            Logger.Print("Initializing settings directory... (path: " + path + ") (exists: " + System.IO.Directory.Exists(path) + ")");
            return System.IO.Directory.CreateDirectory(path);
        }

        public Settings()
        {
            settings = [];
            Logger.Print("Loading settings...");

            if (!System.IO.File.Exists(File))
            {
                Logger.Print("Creating settings file " + File + "...", true);
                System.IO.File.Create(File).Close();
            }
            else
            {
                Logger.Print("Found settings file: " + File);
            }

            try
            {
                string input = System.IO.File.ReadAllText(File);
                settings = JsonSerializer.Deserialize<Dictionary<string, object?>>(input)!;
            } catch (Exception e)
            {
                Logger.Warn("Unable to parse settings! " + e.Message + " Recovering...");
                Reset();
            }
        }

        public T? Get<T>(string key)
        {
            try
            {
                if (!settings.ContainsKey(key)) throw new Exception("Key is not present.");
                var element = settings[key];

                if (element is JsonElement json)
                {
                    if (typeof(T) == typeof(bool)) return (T)(object)json.GetBoolean();
                    if (typeof(T) == typeof(DateTime) && json.GetString() != null) return (T?)(object)DateTime.Parse(json.GetString()!);
                }

                return (T?)element;
            } catch (Exception e)
            {
                Logger.Print("Settings[" + key + "] access warning: " + e.Message);
                return (T?)Defaults()[key] ?? default;
            }
        }

        public bool Set<T>(string key, T? value)
        {
            object? output = value;

            if (typeof(T) == typeof(DateTime) && value != null)
            {
                output = ((DateTime)(object)value).ToString("o");
            }

            try
            {
                settings[key] = value;
                System.IO.File.WriteAllText(File, JsonSerializer.Serialize(settings));
                return true;
            } catch (Exception e)
            {
                Logger.Warn("Unable to set settings key " + key + ": " + e.Message);
                return false;
            }
        }

        public bool Reset()
        {
            settings = [];
            System.IO.File.WriteAllText(File, JsonSerializer.Serialize(settings));
            return true;
        }
        
        public bool Reset<T>(string key)
        {
            return Set<T>(key, default);
        }

        public static Dictionary<string, object?> Defaults()
        {
            return new Dictionary<string, object?>
            {
                ["betaVersions"] = false,
                ["autoCheckForUpdates"] = false,
            };
        }
    }

    public sealed partial class SettingsPage : Page
    {
        public ObservableCollection<SettingObject> AllSettings { get; } = new();

        public SettingsPage(SettingsWindow? window = null)
        {
            AllSettings = [];
            Logger.Print("Adding settings...");

            List<SettingObject> settings = [
                new Setting {
                    Title = "Check For Updates",
                    Description = "Check for updates.",
                    Selector = Selector.Button("Check", async (Button sender, RoutedEventArgs e) => {
                        string current = (string)sender.Content;
                        if (current == "Checking...") return;
                        sender.Content = "Checking...";
                        Version? result = await Version.CheckForUpdates(this.XamlRoot, true);
                        sender.Content = current;
                    }),
                },
                new SettingTitle {
                    Title = "General",
                    Description = "General settings for About This PC.",
                },
                new Setting
                {
                    Title = "Use Beta Versions",
                    Description = "When checking for updates, allow finding beta versions.",
                    Selector = Selector.Boolean("betaVersions"),
                },
                new Setting {
                    Title = "Check For Updates Automatically",
                    Description = "Have About This PC check for updates automatically at launch and periodically.",
                    Selector = Selector.Boolean("autoCheckForUpdates"),
                },
                new Setting {
                    Title = "Reset All Settings",
                    Description = "Reset all About This PC settings to default.",
                    Selector = Selector.Button("Reset", (Button sender, RoutedEventArgs e) => {
                        bool status = App.settings?.Reset() ?? false;

                        if (status == false) {
                            Logger.Warn("Unable to reset settings! Status was false.");
                            _ = App.Error(this.XamlRoot, "We were unable to reset your settings.", "FALSE_STATUS");
                        }

                        if (window != null) {
                            window.Close();
                        } else {
                            Logger.Warn("Unable to reload settings! No method provided.");
                            _ = App.Error(this.XamlRoot, "Please refresh your settings page.", "NULL_WINDOW", true);
                        }
                    }),
                },
            ];

            foreach (SettingObject setting in settings)
            {
                if (setting is Setting) AllSettings.Add((Setting)setting);
                if (setting is SettingTitle) AllSettings.Add((SettingTitle)setting);
            }

            InitializeComponent();
        }

        public static async void Window()
        {
            var dimensions = new App.Dimensions(800, 500).Build();
            var window = new SettingsWindow();
            var appwindow = window.AppWindow;
            var presenter = appwindow?.Presenter as OverlappedPresenter;

            appwindow?.Resize(dimensions);
            window.ExtendsContentIntoTitleBar = true;
            window.Content = new SettingsPage(window);
            await MainWindow.SetIcon(appwindow);

            if (presenter != null)
            {
                presenter.PreferredMinimumWidth = dimensions.Width;
                presenter.PreferredMinimumHeight = dimensions.Height;
            }

            window.Title = "About This PC Settings";
            window.Activate();
        }
    }

    public abstract class SettingObject;

    public class SettingTitle : SettingObject
    {
        public required string Title { get; set; }
        public required string Description { get; set; }
    }

    public class Setting : SettingObject
    {
        public required string Title { get; set; }
        public required string Description { get; set; }
        public required UIElement Selector { get; set; }
    }

    public class Selector
    {
        public static UIElement Boolean(string key)
        {
            ComboBox box = new ComboBox()
            {
                Width = 100,
                PlaceholderText = "Yes/no",
            };

            box.Items.Add(new ComboBoxItem { Content = "Yes", AccessKey = "1" });
            box.Items.Add(new ComboBoxItem { Content = "No", AccessKey = "0" });

            box.SelectionChanged += (sender, e) =>
            {
                if (box.SelectedItem is ComboBoxItem selectedItem)
                {
                    string? choice = selectedItem.Content.ToString();

                    if (choice == "Yes")
                    {
                        App.settings!.Set<bool>(key, true);
                    }
                    else if (choice == "No")
                    {
                        App.settings!.Set<bool>(key, false);
                    }
                }
            };

            bool current = App.settings!.Get<bool>(key);
            Logger.Print("Current selection for " + key + ": " + current);

            foreach (ComboBoxItem item in box.Items)
            {
                if ((string)item.AccessKey == (current ? "1" : "0"))
                {
                    box.SelectedItem = item;
                }
            }

            return box;
        }

        public static UIElement Button(string text, Action<Button, RoutedEventArgs> callback)
        {
            Button button = new Button()
            {
                Content = text,
            };

            button.Click += (object sender, RoutedEventArgs e) =>
            {
                callback((Button)sender, e);
            };

            return button;
        }
    }

    public class SettingTemplateSelector : DataTemplateSelector
    {
        public DataTemplate? SettingTemplate { get; set; }
        public DataTemplate? SettingTitleTemplate { get; set; }

        protected override DataTemplate SelectTemplateCore(object item, DependencyObject container)
        {
            if (item is Setting) return SettingTemplate!;
            else if (item is SettingTitle) return SettingTitleTemplate!;
            else return base.SelectTemplateCore(item, container);
        }
    }
}
