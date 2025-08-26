package main

import (
	"archive/zip"
	"bytes"
	_ "embed"
	"fmt"
	"io"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/gen2brain/dlgs"
	"github.com/getlantern/systray"
)

//go:embed build/archive.zip
var archive []byte

//go:embed build/icon.ico
var appicon []byte

var Version = ""
var args = os.Args[1:]

var localAppData = os.Getenv("LOCALAPPDATA")
var directory = filepath.Join(localAppData, "AboutThisPC")

func Fatal[T any](input T) {
	e := fmt.Sprint(input)
	log.Fatalln(e)
}

func main() {
	systray.Run(onReady, onExit)
	start()
}

func onReady() {
	systray.SetTitle("About This PC")
	systray.SetTooltip("About This PC " + Version + " by Calebh101")
	systray.SetIcon(appicon)

	actionOpen := systray.AddMenuItem("About This PC", "Open a new window.")
	actionQuit := systray.AddMenuItem("Quit", "Quit the application.")

	go func() {
		for {
			select {
			case <-actionOpen.ClickedCh:
				run()
				return
			case <-actionQuit.ClickedCh:
				systray.Quit()
				return
			}
		}
	}()
}

func onExit() {
	log.Println("Exiting...")
}

func getDetectedVersion(directory string) string {
	filename := directory + "\\VERSION-IDENTIFIER"

	if _, e := os.Stat(filename); os.IsNotExist(e) {
		log.Println("File " + filename + " does not exist")
		return ""
	}

	content, e := os.ReadFile(filename)

	if e != nil {
		Fatal(e)
		return ""
	}

	log.Println("Detected file content: " + string(content))
	return string(content)
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

	foundUninstall := false
	foundService := false

	for _, arg := range args {
		if strings.Contains(arg, "--uninstall") {
			foundUninstall = true
			break
		} else if strings.Contains(arg, "--service") {
			foundService = true
		} else {
			foundArgs = append(foundArgs, arg)
			args = foundArgs
		}
	}

	if foundUninstall {
		ok, e := dlgs.Question("Confirm", "Are you sure you want to uninstall About This PC? This will erase all About This PC data and settings.", true)

		if e != nil {
			Fatal(e)
			return
		}

		if ok {
			log.Println("Uninstalling application...")
			e := os.RemoveAll(directory)

			if e != nil {
				Fatal(e)
				return
			} else {
				log.Println("Directory " + directory + " removed.")
			}

			_, e = dlgs.Info("All Done", "About This PC has been uninstalled.")

			if e != nil {
				Fatal(e)
				return
			}

			return
		} else {
			log.Println("Aborting")
		}

		return
	}

	var currentVersion = getDetectedVersion(directory)

	if currentVersion == "" || currentVersion != Version {
		extract()
	}

	if foundService {
		run()
	}
}

func run() {
	cmd := exec.Command(directory+"\\AboutThisPC.exe", args...)
	log.Println("Running application...")
	err := cmd.Run()

	if err != nil {
		Fatal(err)
		return
	}
}

func extract() {
	log.Println("Found archive of " + strconv.Itoa(len(archive)) + " bytes")
	readerAt := bytes.NewReader(archive)
	zr, e := zip.NewReader(readerAt, int64(len(archive)))

	if e != nil {
		Fatal(e)
	}

	log.Println("Extracting files to " + directory + "...")
	ok, e := dlgs.Question("Confirm", "Are you sure you want to install About This PC? This will overwrite your current installation. Settings will not be overwritten.\n\nThis will install About This PC to "+directory+".", true)

	if e != nil {
		Fatal(e)
		return
	}

	if !ok {
		log.Println("Aborting...")
		return
	}

	for _, file := range zr.File {
		log.Println("Found file: " + file.Name)
		extractPath := filepath.Join(directory, file.Name)

		if file.FileInfo().IsDir() {
			if e := os.MkdirAll(extractPath, os.ModePerm); e != nil {
				Fatal(e)
			}
			continue
		}

		if e := os.MkdirAll(filepath.Dir(extractPath), os.ModePerm); e != nil {
			Fatal(e)
		}

		srcFile, e := file.Open()
		if e != nil {
			Fatal(e)
		}

		dstFile, e := os.Create(extractPath)
		if e != nil {
			srcFile.Close()
			Fatal(e)
		}

		if _, e := io.Copy(dstFile, srcFile); e != nil {
			srcFile.Close()
			dstFile.Close()
			Fatal(e)
		}

		srcFile.Close()
		dstFile.Close()
	}

	log.Println("Copied " + strconv.Itoa(len(zr.File)) + " files!")
	data := []byte(Version)
	e = os.WriteFile(directory+"\\VERSION-IDENTIFIER", data, 0644)

	if e != nil {
		Fatal(e)
	}

	_, e = dlgs.Info("All Done", "About This PC has been installed.")

	if e != nil {
		Fatal(e)
	}
}
