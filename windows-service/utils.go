package main

import (
	"fmt"
	"log"
	"os"
	"os/exec"

	"github.com/gen2brain/dlgs"
)

func Print(input any) {
	ignoreVerbose := true

	if ignoreVerbose || verbose {
		Output("LOG", input)
	}
}

func Output(prefix string, input any) {
	value := fmt.Sprint(input)
	log.Println(prefix + " " + value)
}

func Warn(input any) {
	Output("WRN", fmt.Sprint(input))
}

func Fatal(input any) {
	value := "Fatal: " + fmt.Sprint(input)
	Output("ERR", value)
	_, e := dlgs.Info("Fatal Exception", value)

	if e != nil {
		Fatal(e)
	}
}

func CloseProcess(cmd *exec.Cmd) bool {
	e := cmd.Process.Signal(os.Interrupt)

	if e != nil {
		Print("Unable to close process: " + e.Error() + " (killing)")
		e = cmd.Process.Kill()

		if e != nil {
			Fatal(e)
			return false
		} else {
			Print("Successfully killed process")
			return true
		}
	} else {
		Print("Successfully closed process")
		return true
	}
}

func GetDetectedVersion(directory string) string {
	filename := directory + "\\VERSION-IDENTIFIER"

	if _, e := os.Stat(filename); os.IsNotExist(e) {
		Print("File " + filename + " does not exist")
		return ""
	}

	content, e := os.ReadFile(filename)

	if e != nil {
		Fatal(e)
		return ""
	}

	Print("Detected file content: " + string(content))
	return string(content)
}

func Contains(slice []string, str string) bool {
	for _, v := range slice {
		if v == str {
			return true
		}
	}

	return false
}
