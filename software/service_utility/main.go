package main

// create binary data using
// ~/go/bin/go-bindata -o binary.go ../firmware/partition_table/partition-table.bin ../firmware/bootloader/bootloader.bin ../firmware/flipdot-firmware.bin

import (
	"flag"
	"fmt"
	"github.com/fluepke/esptool/common/serial"
	"github.com/fluepke/esptool/esp32"
	"log"
	"os"
)

var (
	serialPortPath = flag.String("serial.port", "", "Serial port the flipdot is attached to")
	logger         = log.New(os.Stdout, bold("[LOG]: "), log.Ltime|log.Lshortfile)
)

func bold(s string) string {
	return fmt.Sprintf("\033[1m%s\033[0m", s)
}

func flash(esp32 *esp32.ESP32ROM, asset string, offset uint32) {
	logger.Printf("Writing %s to 0x%X\n", asset, offset)
	data, err := Asset(asset)
	if err != nil {
		log.Printf("Meh. Contact maintainer: %v", err)
		os.Exit(4)
	}

	if err = esp32.WriteFlash(offset, data); err != nil {
		logger.Printf("Write to flash failed: %v", err)
		os.Exit(5)
	}
}

func main() {
	flag.Parse()
	serialConfig := serial.NewConfig(*serialPortPath, 115200)
	serialPort, err := serial.OpenPort(serialConfig)
	if err != nil {
		logger.Printf("Could not open serial port: %v", err)
		os.Exit(1)
	}

	esp32 := esp32.NewESP32ROM(serialPort, logger)

	for i := 0; i < 3; i++ {
		err := esp32.Connect(5)
		if err == nil {
			break
		}
	}

	if err != nil {
		logger.Printf("Could not connect to ESP32: %v", err)
		os.Exit(2)
	}

	err = esp32.ChangeBaudrate(921600)
	if err != nil {
		logger.Printf("Failed to change baudrate: %v", err)
		os.Exit(3)
	}

	// flash partition table
	flash(esp32, "../firmware/partition_table/partition-table.bin", 0x800)
	flash(esp32, "../firmware/bootloader/bootloader.bin", 0xd000)
	flash(esp32, "../firmware/flipdot-firmware.bin", 0x10000)

	logger.Println("Firmware written to device. Have fun.")
}
