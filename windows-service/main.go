package main

import (
	"archive/zip"
	"bufio"
	"bytes"
	_ "embed"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/Microsoft/go-winio"
	"github.com/gen2brain/dlgs"
	"github.com/getlantern/systray"
	"github.com/gofrs/flock"
	"github.com/ncruces/zenity"
)

//go:embed build/archive.zip
var archive []byte

//go:embed build/icon.ico
var appicon []byte

var Version = ""
var IsConsole = "0"
var isDebugExecutable = false
var args = os.Args[1:]
var verbose = Contains(args, "--debug") // Not used for now
var pipeName = `\\.\pipe\AboutThisPCWindowsService`
var localAppData = os.Getenv("LOCALAPPDATA")
var directory = filepath.Join(localAppData, "AboutThisPC")
var lockpath = filepath.Join(os.TempDir(), "AboutThisPC.lck")

var processes []*exec.Cmd
var mu sync.Mutex

func main() {
	if IsConsole != "0" {
		isDebugExecutable = true
	}

	if Contains(args, "--info") {
		dlgs.Info("About This PC", "About This PC\n\nVersion: " + Version + "\nAuthor: Calebh101\n\nFor more information, head to:\nhttps://github.com/Calebh101/About-This-PC")
		os.Exit(0)
		return
	}

	Print("Starting...")
	lock := flock.New(lockpath)
	locked, e := lock.TryLock()

	if e != nil {
		Fatal("Cannot initialize lockfile: " + e.Error())
		return
	}

	if !locked {
		Print("Process is locked")
		onIsLocked()
		return
	}

	defer lock.Unlock()
	go listen()

	start()
	systray.Run(onReady, onExit)
}

func listen() {
	ln, e := winio.ListenPipe(pipeName, nil)

	if e != nil {
		Fatal(e)
	}

	defer ln.Close()
	Print("Listening on localhost:" + pipeName + "...")

	for {
		connection, e := ln.Accept()

		if e != nil {
			Fatal("Client connection error! " + e.Error())
			continue
		}

		reader := bufio.NewReader(connection)
		message, e := reader.ReadString('|')
		message = strings.TrimSuffix(message, "|")

		if e != nil {
			Fatal(e)
			continue
		} else {
			Print("Received message: " + message)
			mode, e := strconv.Atoi(message)

			if e != nil {
				Print("Unable to parse mode '" + message + "': " + e.Error())
				continue
			} else {
				run(mode)
			}
		}
	}
}

func onIsLocked() {
	Print("Running onIsLocked with arguments '" + strings.Join(args, " ") + "'...")
	classic := Contains(args, "--classic")
	mode := 0
	deadline := 5 * time.Second
	connection, e := winio.DialPipe(pipeName, &deadline)
	defer connection.Close()
	hasArgs := false

	for _, arg := range []string{"--uninstall", "--reinstall"} {
		if (Contains(args, arg)) {
			hasArgs = true
		}
	}

	if hasArgs {
		yes, e := dlgs.Question("Warning", "You are trying to run an action while an instance of About This PC is already running. Please quit About This PC to run this action.\n\nContinue creating a new About This PC window?", false)

		if e != nil {
			Fatal(e)
			return
		}

		if !yes {
			return
		}
	}

	if e != nil {
		Fatal(e.Error() + "\n\nIf the process is stopped but the lockfile is still active, this error may occur. Please check that '" + lockpath + "' doesn't exist, and remove it if necessary.")
	}

	if classic {
		mode = 1
	}

	Print("Sending message to server... (mode: " + strconv.Itoa(mode) + ")")
	fmt.Fprint(connection, strconv.Itoa(mode) + "|")
}

func closeAll() {
	mu.Lock()
	for i, cmd := range processes {
		if cmd.Process != nil {
			Print("Found process " + strconv.Itoa(i) + ": " + strconv.Itoa(cmd.Process.Pid))
			CloseProcess(cmd)
		}
	}
	mu.Unlock()
}

func onReady() {
	systray.SetTitle("About This PC")
	systray.SetTooltip("About This PC " + Version + " by Calebh101")
	systray.SetIcon(appicon)

	actionOpen := systray.AddMenuItem("About This PC", "Open a new window.")
	actionOpenClassic := systray.AddMenuItem("About This PC (Classic)", "Open a new window in classic mode.")
	actionOpenSettings := systray.AddMenuItem("About This PC Settings", "Open a new settings window.")
	systray.AddSeparator()
	actionCloseOne := systray.AddMenuItem("Close", "Close the most recent window.")
	actionCloseAll := systray.AddMenuItem("Close All", "Close all windows.")
	systray.AddSeparator()
	actionQuit := systray.AddMenuItem("Quit", "Quit the service.")
	actionRestart := systray.AddMenuItem("Restart", "Restart the service.")

	go func() {
		for {
			select {
			case <-actionOpen.ClickedCh:
				run(0)
			case <-actionOpenClassic.ClickedCh:
				run(1)
			case <-actionOpenSettings.ClickedCh:
				run(2)
			case <-actionCloseOne.ClickedCh:
				mu.Lock()
				cmds := processes

				for i, j := 0, len(cmds)-1; i < j; i, j = i+1, j-1 {
					cmds[i], cmds[j] = cmds[j], cmds[i]
				}

				for i, cmd := range cmds {
					if cmd.Process != nil {
						Print("Found process " + strconv.Itoa(i) + ": " + strconv.Itoa(cmd.Process.Pid))
						CloseProcess(cmd)
						break
					}
				}

				mu.Unlock()
			case <-actionCloseAll.ClickedCh:
				closeAll()
			case <-actionRestart.ClickedCh:
				closeAll()
				restart()
			case <-actionQuit.ClickedCh:
				closeAll()
				systray.Quit()
			}
		}
	}()
}

func restart() {
	selfPath, e := os.Executable()

	if e != nil {
		Fatal(e)
		return
	}

	cmd := exec.Command(selfPath, os.Args[1:]...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	cmd.Stdin = os.Stdin

	Print("Starting new process...")
	e = cmd.Start()

	if e != nil {
		Fatal(e)
		return
	}

	Print("Restarting...")
	os.Exit(0)
}

func onExit() {
	Print("Exiting...")
}

func start() {
	if Version == "" {
		Fatal("Version cannot be empty!")
		return
	}

	if localAppData == "" {
		Fatal("LOCALAPPDATA environment variable not set!")
		return
	}

	var foundArgs []string
	var e = os.MkdirAll(directory, os.ModePerm)

	if e != nil {
		Fatal(e)
		return
	}

	for _, arg := range args {
		if strings.Contains(arg, "--uninstall") {
			Print("Found uninstall argument")
			ok, e := dlgs.Question("Confirm", "Are you sure you want to uninstall About This PC? This will erase all About This PC data and settings.", true)

			if e != nil {
				Fatal(e)
				return
			}

			if ok {
				Print("Uninstalling application...")
				e := os.RemoveAll(directory)

				if e != nil {
					Fatal(e)
					return
				} else {
					Print("Directory " + directory + " removed.")
				}

				_, e = dlgs.Info("All Done", "About This PC has been uninstalled.")

				if e != nil {
					Fatal(e)
					return
				}

				os.Exit(0)
				return
			} else {
				Print("Aborting")
			}

			return
		} else if strings.Contains(arg, "--reinstall") {
			Print("Found reinstall argument")

			if extract() {
				os.Exit(0)
			}
		} else {
			Print("Found custom argument: " + arg)
			foundArgs = append(foundArgs, arg)
			args = foundArgs
		}
	}

	var currentVersion = GetDetectedVersion(directory)

	if currentVersion == "" || currentVersion != Version {
		extract()
	}

	if !Contains(args, "--service") {
		mode := 0

		if Contains(args, "--classic") {
			mode = 1
		}

		run(mode)
	}
}

// Modes (for [mode])
// 	 0: Normal
// 	 1: Classic
//   2: Settings

func run(mode int) {
	classic := mode == 1
	settings := mode == 2

	Print("Detected run with mode " + strconv.Itoa(mode))
	var extraArg = []string{}

	if classic {
		extraArg = []string{"--classic"}
	} else if settings {
		extraArg = []string{"--settings"}
	}

	finalArgs := append(extraArg, args...)
	cmd := exec.Command(directory + "\\AboutThisPC.exe", finalArgs...)

	mu.Lock()
	processes = append(processes, cmd)
	mu.Unlock()

	Print("Running application...")
	e := cmd.Start()

	if e != nil {
		Fatal(e)
		return
	}

	go func() {
		e := cmd.Wait()

		if e != nil {
			Print("Process exited with error: " + e.Error())
		} else {
			Print("Process exited successfully")
		}

		mu.Lock()

		for i, p := range processes {
			if p == cmd {
				processes = append(processes[:i], processes[i+1:]...)
				break
			}
		}

		mu.Unlock()
	}()
}

func extract() bool {
	Print("Found archive of " + strconv.Itoa(len(archive)) + " bytes")
	readerAt := bytes.NewReader(archive)
	zr, e := zip.NewReader(readerAt, int64(len(archive)))
	count := 0

	if e != nil {
		Fatal(e)
		return false
	}

	Print("Extracting files to " + directory + "...")
	ok, e := dlgs.Question("Confirm", "Are you sure you want to install About This PC? This will overwrite your current installation, if present. Settings will not be overwritten.\n\nThis will install About This PC to "+directory+".", true)

	if e != nil {
		Fatal(e)
		return false
	}

	if !ok {
		Print("Aborting...")
		return false
	}

	if isDebugExecutable {
		ok, e := dlgs.Question("Confirm", "Since you are installing with a debug executable, the installed version will use a debug executable as well. To revert this, you can reinstall with the Windows GUI.", true)

		if e != nil {
			Fatal(e)
			return false
		}

		if !ok {
			Print("Aborting...")
			return false
		}
	}

	progress, _ := zenity.Progress(
        zenity.Title("Installing About This PC"),
        zenity.MaxValue(len(zr.File)+1),
    )

    defer progress.Close()

	for _, file := range zr.File {
		Print("Copying file " + file.Name + "... (archive)")
		extractPath := filepath.Join(directory, file.Name)

		progress.Text("Extracting: " + file.Name)
		progress.Value(count)

		if file.FileInfo().IsDir() {
			if e := os.MkdirAll(extractPath, os.ModePerm); e != nil {
				Fatal(e)
				return false
			}

			continue
		}

		if e := os.MkdirAll(filepath.Dir(extractPath), os.ModePerm); e != nil {
			Fatal(e)
			return false
		}

		srcFile, e := file.Open()

		if e != nil {
			Fatal(e)
			return false
		}

		dstFile, e := os.Create(extractPath)

		if e != nil {
			srcFile.Close()
			Fatal(e)
			return false
		}

		if _, e := io.Copy(dstFile, srcFile); e != nil {
			srcFile.Close()
			dstFile.Close()
			Fatal(e)
			return false
		}

		srcFile.Close()
		dstFile.Close()
		count++
	}

	progress.Value(count)
	progress.Text("Copying: self")

	if copySelf() {
		count++
	} else {
		return false
	}

	progress.Value(count)
	Print("Copied " + strconv.Itoa(len(zr.File)) + " files!")
	data := []byte(Version)
	e = os.WriteFile(directory+"\\VERSION-IDENTIFIER", data, 0644)

	if e != nil {
		Fatal(e)
		return false
	}

	_, e = dlgs.Info("All Done", "About This PC has been installed. It will now be started and opened.")

	if e != nil {
		Fatal(e)
		return false
	} else {
		return true
	}
}

func copySelf() bool {
	file, e := os.Executable()
	Print("Copying file " + file + "... (self)")
	extractPath := filepath.Join(directory, "AboutThisPC-Service.exe")

	if file == extractPath {
		Print("Aborting, due to already existing")
		return false
	}

	if e != nil {
		Fatal(e)
		return false
	}

	if e := os.MkdirAll(filepath.Dir(extractPath), os.ModePerm); e != nil {
		Fatal(e)
		return false
	}

	srcFile, e := os.Open(file)

	if e != nil {
		Fatal(e)
		return false
	}

	dstFile, e := os.Create(extractPath)

	if e != nil {
		srcFile.Close()
		Fatal(e)
		return false
	}

	if _, e := io.Copy(dstFile, srcFile); e != nil {
		srcFile.Close()
		dstFile.Close()
		Fatal(e)
		return false
	}

	srcFile.Close()
	dstFile.Close()
	return true
}
