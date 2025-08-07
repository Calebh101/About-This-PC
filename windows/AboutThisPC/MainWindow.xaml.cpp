#include "pch.h"
#include "MainWindow.xaml.h"
#include <winrt/Microsoft.UI.Windowing.h>
#include <Windows.Graphics.h>

#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::AboutThisPC::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
        // You can add code here that runs when the window is created,
        // but don't set the window size here.
    }
}