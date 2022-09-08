package service

import (
	"fmt"
	"testing"

	"windows_monitor/util"
)

func TestGetCpuPercent(t *testing.T) {
	fmt.Println(GetCpuPercent())
	fmt.Println(util.FloatToIntString(GetCpuPercent()))
}

func TestGetMemPercent(t *testing.T) {
	fmt.Println(GetMemPercent())
	fmt.Println(util.FloatToIntString(GetMemPercent()))
}
