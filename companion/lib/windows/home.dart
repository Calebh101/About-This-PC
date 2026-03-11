import 'dart:async';
import 'dart:io';

import 'package:companion/main.dart';
import 'package:companion/windows/pipe.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:localpkg_flutter/dialogue.dart';

final File versionIdentifier = File("${Platform.environment['LOCALAPPDATA']}\\AboutThisPC\\VERSION-IDENTIFIER");
final File mainExe = File("${File(Platform.resolvedExecutable).parent.path}\\AboutThisPC.exe");
final File installedExe = File("${Platform.environment['LOCALAPPDATA']}\\AboutThisPC\\AboutThisPC-Service.exe");
final File containedVersion = File("${File(Platform.resolvedExecutable).parent.path}\\version");
const Duration tooltipDelay = Duration(seconds: 2);

class WindowsHome extends StatefulWidget {
  final Pipe pipe;
  const WindowsHome({super.key, required this.pipe});

  @override
  State<WindowsHome> createState() => _WindowsHomeState();
}

class _WindowsHomeState extends State<WindowsHome> {
  bool isOpen = false;
  bool mainExeExists = false;
  bool installedExeExists = false;
  String? installed;
  String? contained;
  Timer? timer;

  void check() async {
    try {
      installed = (await versionIdentifier.readAsString()).trim();
    } catch (e) {
      if (kDebugMode) print("Installed version error: $e");
      installed = null;
    }

    try {
      contained = (await containedVersion.readAsString()).trim();
    } catch (e) {
      if (kDebugMode) print("Contained version error: $e");
      contained = null;
    }

    try {
      isOpen = widget.pipe.isOpen;
      mainExeExists = await mainExe.exists();
      installedExeExists = await installedExe.exists();
      setState(() {});
    } catch (e) {
      print("Check error: $e");
      setState(() {});
    }
  }

  Future<void> run(List<String> args) async {
    try {
      late String path;

      if (mainExeExists) {
        path = mainExe.path;
      } else if (installedExeExists) {
        path = installedExe.path;
      } else {
        throw Exception();
      }

      await Process.run(path, args);
    } catch (e) {
      SnackBarManager.show(context, "Error: $e");
    }
  }

  @override
  void initState() {
    super.initState();
    check();
    timer = Timer.periodic(Duration(seconds: 1), (timer) => check());
  }

  @override
  void dispose() {
    timer?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Spacer(),
            Text("About This PC", style: TextStyle(fontSize: 36)),
            Text("By Calebh101"),
            Spacer(),
            Text("Launcher Version: $version"),
            Text("Installed Version: ${installed ?? "Not Installed"}"),
            if (contained != null) Text("Version In Directory: ${contained ?? "Not Installed"}"),
            //Text("Service Status: ${isOpen ? "Running" : "Stopped"}"),
            Spacer(),
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Tooltip(
                  message: "Stop the currently running About This PC service.",
                  waitDuration: tooltipDelay,
                  child: TextButton(onPressed: isOpen ? () {
                    widget.pipe.sendMessage(Pipe.messageStop);
                  } : null, child: Text("Stop")),
                ),
                Tooltip(
                  message: mainExeExists ? "Start the locally discovered AboutThisPC.exe executable. This may prompt for install." : (installedExeExists ? "Start the system's installed About This PC." : "Start About This PC."),
                  waitDuration: tooltipDelay,
                  child: TextButton(onPressed: isOpen || (!mainExeExists && !installedExeExists) ? null : () async {
                    run([]);
                  }, child: Text("Run")),
                ),
                if (contained != null) Tooltip(
                  message: "Uninstall About This PC for this user.",
                  waitDuration: tooltipDelay,
                  child: TextButton(onPressed: isOpen || installed == null || (!mainExeExists && !installedExeExists) ? null : () async {
                    run(["--uninstall"]);
                  }, child: Text("Uninstall")),
                ),
                if (contained != null) Tooltip(
                  message: "Reinstall About This PC for this user.",
                  waitDuration: tooltipDelay,
                  child: TextButton(onPressed: isOpen || installed == null || (!mainExeExists && !installedExeExists) ? null : () async {
                    run(["--reinstall"]);
                  }, child: Text("Reinstall")),
                ),
              ],
            ),
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Tooltip(
                  message: "Open a new About This PC window.",
                  waitDuration: tooltipDelay,
                  child: TextButton(onPressed: isOpen ? () {
                    widget.pipe.sendMessage(Pipe.messageRunNormal);
                  } : null, child: Text("Open")),
                ),
                Tooltip(
                  message: "Open a new About This PC classic window.",
                  waitDuration: tooltipDelay,
                  child: TextButton(onPressed: isOpen ? () {
                    widget.pipe.sendMessage(Pipe.messageRunClassic);
                  } : null, child: Text("Open (Classic)")),
                ),
                Tooltip(
                  message: "Open a new About This PC settings window. This is not the launcher's settings.",
                  waitDuration: tooltipDelay,
                  child: TextButton(onPressed: isOpen ? () {
                    widget.pipe.sendMessage(Pipe.messageRunSettings);
                  } : null, child: Text("Settings")),
                ),
                Tooltip(
                  message: "Close all About This PC windows, except for launcher windows.",
                  waitDuration: tooltipDelay,
                  child: TextButton(onPressed: isOpen ? () {
                    widget.pipe.sendMessage(Pipe.messageCloseAll);
                  } : null, child: Text("Close All")),
                ),
              ],
            ),
            Spacer(),
          ],
        ),
      ),
    );
  }
}