package util

import "fmt"

func FloatToIntString(num float64) string {
	return fmt.Sprintf("%0.0f", num)
}
