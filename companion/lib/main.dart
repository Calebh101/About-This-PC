import 'dart:io';

import 'package:companion/windows/home.dart';
import 'package:companion/windows/pipe.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:localpkg_flutter/localpkg.dart';
import 'package:window_manager/window_manager.dart';

final Version version = Version.parse("0.0.0A-R7");

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await windowManager.ensureInitialized();

  await windowManager.setSize(const Size(400, 350));
  await windowManager.setResizable(false);
  await windowManager.setMaximizable(false);
  await windowManager.setTitle("About This PC Launcher");
  await windowManager.setTitleBarStyle(TitleBarStyle.normal);

  runApp(const MainApp());
}

class MainApp extends StatelessWidget {
  const MainApp({super.key});

  @override
  Widget build(BuildContext context) {
    late Widget widget;

    Widget unsupported(String os) => Scaffold(
      body: Text("Unsupported OS: $os"),
    );

    if (kIsWeb) {
      widget = unsupported("web");
    } else if (Platform.isWindows) {
      widget = WindowsHome(pipe: Pipe(r"\\.\pipe\AboutThisPCWindowsService"));
    } else {
      widget = unsupported("${Platform.operatingSystem} ${Platform.operatingSystemVersion}");
    }

    return MaterialApp(
      theme: ThemeData.light(),
      darkTheme: ThemeData.dark(),
      themeMode: ThemeMode.system,
      debugShowCheckedModeBanner: true,
      home: widget,
    );
  }
}
