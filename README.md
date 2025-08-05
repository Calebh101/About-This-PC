# About This PC

This is a small project I'm in the process of making, that is supposed to be like the `About This Mac` Apple utility, but for Linux and Windows, using native UI elements (or at least semi-native for some Linux distros), and with some more OS-specific features.

# Linux

About This PC for Linux uses a *lot* of hoping, due to the vast amount of Linux distros (compared to Windows, which only has 10 and 11), so bear with me while I improve it over time.

In the **Overview** page, you'll see your Linux distro's "pretty name" (from the `os-release` standard), along with the version, codename, and other identifiers (based on what's given by your distro) beneath it.

You can also see details like your laptop model, processor, graphics, etcetera. Memory and serial require elevated permissions.

The icon on the side uses a name taken from the `os-release` standard again, and (if found) will search in some directories to try to find the icon file. If none are found, it'll default to an image of Tux.

The eye button in the bottom corner allows you to show/hide your serial and local IP(s) for screenshots.

![Overview Page](https://raw.githubusercontent.com/Calebh101/About-This-PC/master/assets/screenshots/linux-page1.png)

The next page is the Displays page, which, well, shows you your displays. It lists the name, resolution, size, and refresh rate. Then, depending if the display is detected as internal, will show either a laptop icon or a monitor depending on what it finds.

![Displays Page](https://raw.githubusercontent.com/Calebh101/About-This-PC/master/assets/screenshots/linux-page2.png)

The next page will show you connected storage devices, their sized, how much of that is used, and a progress bar showing how much is used. It'll also tell you if it's a startup disk, internal, or recognized as external. (This is based on if the drive is hot-pluggable.)

![Storage Page](https://raw.githubusercontent.com/Calebh101/About-This-PC/master/assets/screenshots/linux-page3.png)

The last page is the Support page, which, like macOS, will show you several links that you can open. My implementation again uses the `os-release` standard, which uses what your distro provides to show some links. I also added on some links to About This PC resources, like the GitHub repo and the GitHub Issues page. (All of mine come *after* your distro's links, so "Homepage" and "Report Bugs" are for your *distro*, not for About This PC.)

![Support Page](https://raw.githubusercontent.com/Calebh101/About-This-PC/master/assets/screenshots/linux-page4.png)

This next page is not in a tab bar like the others, but is instead different from the other section. This is the "classic" page, that resembles the *newer* Ventura+ About This Mac app. Its window is vertical, and instead of an OS icon, it displays a computer image (laptop, monitor, etcetera) like Apple's version. It shows the exact same info as the Overview tab on the other section, with an eye button again, but it also has a "More Info" button. This button opens up the Overview page.

![Classic Page](https://raw.githubusercontent.com/Calebh101/About-This-PC/master/assets/screenshots/linux-classic.png)

## Where are the links in the Overview page?

There are some other links on Apple's (older) About This Mac app, like Software Update and System Report, but since there's no standard for Linux distros for these, I can't include them in the Linux version.

## Boot Arguments

- `--classic`: Load the "classic" page.
- `--verbose`: Load the program in verbose. This gives you a *ton* of extra logging in the terminal.

# Notes

- About This PC for Linux includes a bundled helper binary (from `linux-helper`) that uses elevated permissions to get advanced info. If you don't allow this to run, then the program will be fine, but it won't be able to load memory and serial information.

- About This PC for Linux utilizes Qt, which is licensed under the GNU Lesser General Public License (LGPL) version 3.
You can obtain a copy of the LGPL here: https://www.gnu.org/licenses/lgpl-3.0.html
Qt is a separate project and is not licensed under this project's license.

- Some images originated from (rawpixel.com / Freepik)[https://freepik.com] and have been edited by me, due to me not having any drawing skills. (I hope to make my own images in the future!)