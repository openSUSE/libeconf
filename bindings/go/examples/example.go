// Command example demonstrates basic usage of the econf Go bindings:
// writing an ini file to disk, reading it back, and printing its contents.
package main

import (
	"fmt"
	"log"
	"os"
	"path/filepath"

	"github.com/openSUSE/libeconf/bindings/go/econf"
)

func main() {
	dir, err := os.MkdirTemp("", "econf-example-")
	if err != nil {
		log.Fatal(err)
	}
	defer os.RemoveAll(dir)

	f, err := econf.NewIniFile()
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()

	if err := f.SetString("main", "greeting", "hello from libeconf"); err != nil {
		log.Fatal(err)
	}
	if err := f.SetInt("main", "answer", 42); err != nil {
		log.Fatal(err)
	}

	const fileName = "example.conf"
	if err := f.WriteFile(dir, fileName); err != nil {
		log.Fatal(err)
	}

	read, err := econf.ReadFile(filepath.Join(dir, fileName), "=", "#")
	if err != nil {
		log.Fatal(err)
	}
	defer read.Close()

	groups, err := read.Groups()
	if err != nil {
		log.Fatal(err)
	}

	for _, group := range groups {
		keys, err := read.Keys(group)
		if err != nil {
			log.Fatal(err)
		}
		for _, key := range keys {
			value, err := read.String(group, key)
			if err != nil {
				log.Fatal(err)
			}
			fmt.Printf("[%s] %s = %s\n", group, key, value)
		}
	}
}
