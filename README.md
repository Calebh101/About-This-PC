# About This PC

This is a small project I'm in the process of making, that is supposed to be like the `About This Mac` Apple utility, but for Linux and Windows, using native UI elements (or at least semi-native for some Linux distros), and with some more OS-specific features.

# Linux

About This PC for Linux uses a *lot* of hoping, due to the vast amount of Linux distros (compared to Windows, which modern Windows only has 10 and 11), so bear with me while I improve it over time. I've hopefully prepared enough for this though.

About This PC for Linux is written entirely in C++ for at least a semi-native look.

In the **Overview** page, you'll see your Linux distro's "pretty name" (from the `os-release` standard), along with the version, codename, and other identifiers (based on what's given by your distro) beneath it.

You can also see details like your laptop model, processor, graphics, etcetera. Memory and serial require elevated permissions.

The icon on the side uses a name taken from the `os-release` standard again, and (if found) will search in some directories to try to find the icon file. If none are found, it'll default to an image of Tux.

The eye button in the bottom right corner allows you to show/hide your serial and local IP(s) for screenshots. The settings button (also in the bottom right corner) opens the Settings window.

![Overview Page](https://raw.githubusercontent.com/Calebh101/About-This-PC/master/Assets/screenshots/linux-page1.png)

The next page is the Displays page, which, well, shows you your displays. It lists the name, resolution, size, and refresh rate. Then, depending if the display is detected as internal, will show either a laptop icon or a monitor depending on what it finds.

![Displays Page](https://raw.githubusercontent.com/Calebh101/About-This-PC/master/Assets/screenshots/linux-page2.png)

The next page will show you connected storage devices, their sized, how much of that is used, and a progress bar showing how much is used. It'll also tell you if it's a startup disk, internal, or recognized as external. (This is based on if the drive is hot-pluggable.)

![Storage Page](https://raw.githubusercontent.com/Calebh101/About-This-PC/master/Assets/screenshots/linux-page3.png)

The last page is the Support page, which, like macOS, will show you several links that you can open. My implementation again uses the `os-release` standard, which uses what your distro provides to show some links. I also added on some links to About This PC resources, like the GitHub repo and the GitHub Issues page. (All of mine come *after* your distro's links, so "Homepage" and "Report Bugs" are for your *distro*, not for About This PC.)

![Support Page](https://raw.githubusercontent.com/Calebh101/About-This-PC/master/Assets/screenshots/linux-page4.png)

This next page is not in a tab bar like the others, but is instead different from the other section. This is the "classic" page, that resembles the *newer* Ventura+ About This Mac app. Its window is vertical, and instead of an OS icon, it displays a computer image (laptop, monitor, etcetera) like Apple's version. It shows the exact same info as the Overview tab on the other section, with an eye button again, but it also has a "More Info" button. This button opens up the Overview page.

![Classic Page](https://raw.githubusercontent.com/Calebh101/About-This-PC/master/Assets/screenshots/linux-classic.png)

This page is the Settings page, which is opened by the Settings button in the Overview page. Each setting explains itself.

The Settings window is resizable (me making a resizable window? never) but you can't make it smaller than its original dimensions.

![Settings Page](https://raw.githubusercontent.com/Calebh101/About-This-PC/master/Assets/screenshots/linux-settings.png)

## The System Tray

About This PC for Linux comes with built-in system tray support, that starts when the app is first opened.

- `About This PC`: Opens the main page.
- `About This PC (Classic)`: Opens the "classic" page.
- `Close`: Closes the About This PC window that was interacted with last.
- `Close All`: Closes all About This PC windows.
- `Settings`: Opens the Settings window.
- `Check For Updates`: Check GitHub for updates.
- `Quit`: Quits the service and all open windows.
- `Restart`: Quits the service and all open windows, then reruns itself.

## Where are the buttons in the Overview page?

There are some other buttons on Apple's About This Mac apps, like Software Update and System Report, but since there's no standard for Linux distros for these, I can't include them in the Linux version.

## Boot Arguments

- `--version`: Print the version of the app and exit.
- `--classic`: Load the "classic" page.
- `--verbose`: Load the program in verbose. This gives you a *ton* of extra logging in the terminal.
- `--no-window`: Load the program so that it doesn't show a window at start. This is useful to run the program at startup. The program won't request `pkexec` when run with this argument, and if the helper fails then it will act like `pkexec` was rejected.

# Notes

- About This PC for Linux includes a bundled helper binary (from `linux-helper`) that uses elevated permissions to get advanced info. If you don't allow this to run, then the program will be fine, but it won't be able to load memory and serial information. Run this executable specifically (if you can catch it) with `--version` to see the version.

- About This PC for Linux has a known issue with `sudo`, in that the environment isn't preserved so there are a *lot* of weird quirks. This is why I had to make a special helper project for some data. The current (and probably permanent) workaround is to just, **not run with `sudo`**. This does effect the startup service you can run, in that it won't be able to fetch helper data, leaving some data missing, without running as `sudo`.

- About This PC for Linux utilizes Qt, which is licensed under the GNU Lesser General Public License (LGPL) version 3.
You can obtain a copy of the LGPL here: https://www.gnu.org/licenses/lgpl-3.0.html
Qt is a separate project and is not licensed under this project's license.

- Developing About This PC for Linux using Qt was honestly pretty fun, I would recommend using Qt as your framework for desktop apps like this.

# Windows

About This PC for Windows is written in C# (and XAML) using the WinUI library. This was done so it can have a native look in Windows 11, and so that I can more directly interact with Windows.

However, to package all the files that .NET generates, and since it's just generally confusing to do certain things in C#, there's a separate "service" in `windows-service`, written entirely in Go. It basically acts as the unpackager, runner, manager, and uninstaller, and can even show the system tray icon.

The Windows version of About This PC doesn't need *any* elevated permissions to get all the data the program needs (even serial!), so you don't have to worry about that.

## Boot Arguments

- Application Service (what you actually run)
    - `--version`: Print the version of the app and exit.
    - `--classic`: Load the "classic" page as the default to show instead of the normal page.
    - `--service`: Load the program so that it doesn't show a window at start. This is useful to run the program at startup.
    - `--uninstall`: Prompt to uninstall About This PC, then exit.
    - `--reinstall`: Prompt to reinstall About This PC, then exit.
- Application Executable (these can be used for debug or automation)
    - `--version`: Print the version of the app and exit. This pulls from the application itself instead of just the service around it.
    - `--classic`: Launch the "classic" page. This is used by the service.
    - `--settings`: Launch the settings page. This is used by the service.
    - `--debug`: Attach the running terminal and enable verbose to show logs.

## Notes

- The "service" executable doesn't actually install anything to your desktop or Start menu. This has to be done manually.

- About This PC for Windows includes two executables: `AboutThisPC.exe` and `AboutThisPC-Debug.exe`. They have absolutely no difference, except that `AboutThisPC.exe` is registered as a Windows GUI app, so no terminal will pop up when running it; but consequently, it won't log to an existing terminal. If you want to see what's actually happening, use `AboutThisPC-CLI.exe`, but do note that it does pop up a console, and using it to install will mean that a console will pop up until you install with the GUI version.

- If you're a beginner to WinUI (like I was when I started this), I wouldn't generally recommend using WinUI to make a Windows app. It's pretty confusing, and sometimes even restricting.

# Extra Notes

- Some images (specifically the `computers` images) originated from (rawpixel.com / Freepik)[https://freepik.com] and have been edited by me, due to me not having any drawing skills. (I hope to make my own images in the future!)

- Big thanks to [Wikimedia Commons](https://commons.wikimedia.org/) for existing!
