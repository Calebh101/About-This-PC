package main

import (
	"archive/zip"
	"bufio"
	"bytes"
	_ "embed"
	"fmt"
	"io"
	"net"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"sync"

	"github.com/gen2brain/dlgs"
	"github.com/getlantern/systray"
	"github.com/nightlyone/lockfile"
)

//go:embed build/archive.zip
var archive []byte

//go:embed build/icon.ico
var appicon []byte

var Version = ""
var IsConsole = "0"
var isInConsole = false
var args = os.Args[1:]
var verbose = Contains(args, "--debug") // Not used for now

var socketport = 9525
var localAppData = os.Getenv("LOCALAPPDATA")
var directory = filepath.Join(localAppData, "AboutThisPC")

var processes []*exec.Cmd
var mu sync.Mutex

func main() {
	if IsConsole != "0" {
		isInConsole = true
	}

	if Contains(args, "--version") {
		os.Stdout.Write([]byte(Version))
		os.Exit(0)
		return
	}

	Print("Starting...")
	lock, e := lockfile.New(filepath.Join(directory, "AboutThisPC.lck"))

	if e != nil {
		Fatal("Cannot initialize lockfile: " + e.Error())
		return
	}

	if e = lock.TryLock(); e != nil {
		Print("Cannot lock lockfile: " + e.Error())
		onIsLocked(lock)
		return
	}

	go listen()
	start()
	systray.Run(onReady, onExit)
}

func listen() {
	ln, e := net.Listen("tcp", "127.0.0.1:"+strconv.Itoa(socketport))

	if e != nil {
		Fatal(e)
	}

	defer ln.Close()
	Print("Listening on localhost:" + strconv.Itoa(socketport) + "...")

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

func onIsLocked(lock lockfile.Lockfile) {
	Print("Running onIsLocked with arguments '" + strings.Join(args, " ") + "'...")
	classic := Contains(args, "--classic")
	mode := 0
	connection, e := net.Dial("tcp", "127.0.0.1:"+strconv.Itoa(socketport))

	if e != nil {
		Fatal(e)
	}

	if classic {
		mode = 1
	}

	defer connection.Close()
	Print("Sending message to server... (mode: " + strconv.Itoa(mode) + ")")
	fmt.Fprint(connection, strconv.Itoa(mode)+"|")
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
				mu.Lock()

				for i, cmd := range processes {
					if cmd.Process != nil {
						Print("Found process " + strconv.Itoa(i) + ": " + strconv.Itoa(cmd.Process.Pid))
						CloseProcess(cmd)
					}
				}

				mu.Unlock()
			case <-actionRestart.ClickedCh:
				restart()
			case <-actionQuit.ClickedCh:
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

	foundService := false
	foundClassic := false

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
			if extract() {
				os.Exit(0)
			}
		} else if strings.Contains(arg, "--service") {
			foundService = true
		} else if strings.Contains(arg, "--classic") {
			foundClassic = true
		} else {
			foundArgs = append(foundArgs, arg)
			args = foundArgs
		}
	}

	var currentVersion = GetDetectedVersion(directory)

	if currentVersion == "" || currentVersion != Version {
		extract()
	}

	if !foundService {
		mode := 0

		if foundClassic {
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
	cmd := exec.Command(directory+"\\AboutThisPC.exe", finalArgs...)
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

	if isInConsole {
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

	for _, file := range zr.File {
		Print("Found file: " + file.Name)
		extractPath := filepath.Join(directory, file.Name)

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

	if copySelf() {
		count++
	} else {
		return false
	}

	Print("Copied " + strconv.Itoa(len(zr.File)) + " files!")
	data := []byte(Version)
	e = os.WriteFile(directory+"\\VERSION-IDENTIFIER", data, 0644)

	if e != nil {
		Fatal(e)
		return false
	}

	_, e = dlgs.Info("All Done", "About This PC has been installed.")

	if e != nil {
		Fatal(e)
		return false
	} else {
		return true
	}
}

func copySelf() bool {
	file, e := os.Executable()
	Print("Copying self-path " + file + "...")
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
