package main

import (
	"archive/zip"
	"bytes"
	_ "embed"
	"io"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/gen2brain/dlgs"
)

//go:embed build/archive.zip
var archive []byte

//go:embed build/icon.ico
var appicon []byte

var Version = ""
var args = os.Args[1:]

func getDetectedVersion(directory string) string {
	filename := directory + "\\VERSION-IDENTIFIER"

	if _, e := os.Stat(filename); os.IsNotExist(e) {
		log.Println("File " + filename + " does not exist")
		return ""
	}

	content, e := os.ReadFile(filename)

	if e != nil {
		log.Fatalln(e)
		return ""
	}

	log.Println("Detected file content: " + string(content))
	return string(content)
}

func main() {
	if Version == "" {
		log.Fatalln("Version cannot be empty!")
		return
	}

	localAppData := os.Getenv("LOCALAPPDATA")

	if localAppData == "" {
		log.Fatalln("LOCALAPPDATA environment variable not set!")
		return
	}

	directory := filepath.Join(localAppData, "AboutThisPC")
	var e = os.MkdirAll(directory, os.ModePerm)

	if e != nil {
		log.Fatalln(e)
		return
	}

	var foundArgs []string
	foundUninstall := false

	for _, arg := range args {
		if strings.Contains(arg, "--uninstall") {
			foundUninstall = true
			break
		} else {
			foundArgs = append(foundArgs, arg)
		}
	}

	if foundUninstall {
		ok, e := dlgs.Question("Confirm", "Are you sure you want to uninstall About This PC? This will erase all About This PC data and settings.", true)

		if e != nil {
			log.Fatalln(e)
			return
		}

		if ok {
			log.Println("Uninstalling application...")
			e := os.RemoveAll(directory)

			if e != nil {
				log.Fatalln(e)
				return
			} else {
				log.Println("Directory " + directory + " removed.")
			}

			_, e = dlgs.Info("All Done", "About This PC has been uninstalled.")

			if e != nil {
				log.Fatalln(e)
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
		extract(directory)
	}

	run(directory, foundArgs)
}

func run(directory string, args []string) {
	cmd := exec.Command(directory+"\\AboutThisPC.exe", args...)
	log.Println("Running application...")
	err := cmd.Run()

	if err != nil {
		log.Fatalln(err)
		return
	}
}

func extract(directory string) {
	log.Println("Found archive of " + strconv.Itoa(len(archive)) + " bytes")
	readerAt := bytes.NewReader(archive)
	zr, e := zip.NewReader(readerAt, int64(len(archive)))

	if e != nil {
		log.Fatalln(e)
	}

	log.Println("Extracting files to " + directory + "...")
	ok, e := dlgs.Question("Confirm", "Are you sure you want to install About This PC? This will overwrite your current installation. Settings will not be overwritten.\n\nThis will install About This PC to "+directory+".", true)

	if e != nil {
		log.Fatalln(e)
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
				log.Fatalln(e)
			}
			continue
		}

		if e := os.MkdirAll(filepath.Dir(extractPath), os.ModePerm); e != nil {
			log.Fatalln(e)
		}

		srcFile, e := file.Open()
		if e != nil {
			log.Fatalln(e)
		}

		dstFile, e := os.Create(extractPath)
		if e != nil {
			srcFile.Close()
			log.Fatalln(e)
		}

		if _, e := io.Copy(dstFile, srcFile); e != nil {
			srcFile.Close()
			dstFile.Close()
			log.Fatalln(e)
		}

		srcFile.Close()
		dstFile.Close()
	}

	log.Println("Copied " + strconv.Itoa(len(zr.File)) + " files!")
	data := []byte(Version)
	e = os.WriteFile(directory+"\\VERSION-IDENTIFIER", data, 0644)

	if e != nil {
		log.Fatal(e)
	}
}
