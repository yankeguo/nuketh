package main

import (
	"compress/gzip"
	"encoding/csv"
	"encoding/hex"
	"errors"
	"flag"
	"io"
	"log"
	"os"
	"path/filepath"
	"strings"

	"github.com/yankeguo/rg"
)

func handleFile(filename string, w io.Writer) (err error) {
	defer rg.Guard(&err)

	f := rg.Must(os.Open(filename))
	defer f.Close()

	gr := rg.Must(gzip.NewReader(f))
	defer gr.Close()

	cr := csv.NewReader(gr)

	head := rg.Must(cr.Read())

	idxAddress := -1

	for i, item := range head {
		if strings.EqualFold(item, "address") {
			idxAddress = i
			break
		}
	}

	if idxAddress == -1 {
		err = errors.New("missing address column")
		return
	}

	var (
		item []string
		addr string
		data []byte
	)

	for {
		if item, err = cr.Read(); err != nil {
			if err == io.EOF {
				err = nil
			}
			return
		}

		// address
		addr = item[idxAddress]
		if !strings.HasPrefix(addr, "0x") {
			err = errors.New("invalid address")
			return
		}
		addr = strings.ToLower(addr[2:])

		// binary address
		data = rg.Must(hex.DecodeString(addr))
		if len(data) != 20 {
			err = errors.New("invalid address")
			return
		}
		rg.Must(w.Write(data))
	}
}

func main() {
	var err error
	defer func() {
		if err == nil {
			return
		}
		log.Println("exited with error:", err.Error())
		os.Exit(1)
	}()
	defer rg.Guard(&err)

	var (
		optFrom string
		optTo   string
	)

	flag.StringVar(&optFrom, "from", "", "source dir of gzip compressed csv file")
	flag.StringVar(&optTo, "to", "nuketh.bal", "target binary addresses file")
	flag.Parse()

	if optFrom == "" {
		err = errors.New("missing --from")
		return
	}

	if optTo == "" {
		err = errors.New("missing --to")
		return
	}

	o := rg.Must(os.Create(optTo))
	defer o.Close()

	for _, item := range rg.Must(os.ReadDir(optFrom)) {
		if item.IsDir() {
			continue
		}
		if strings.HasPrefix(item.Name(), ".") {
			continue
		}
		log.Println("handling", item.Name())
		rg.Must0(handleFile(filepath.Join(optFrom, item.Name()), o))
	}
}
