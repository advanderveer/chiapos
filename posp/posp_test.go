package posp

import "testing"

func TestValidateProof(t *testing.T) {
	for i := 0; i < 1000; i++ {
		ValidateProof()
	}
}
