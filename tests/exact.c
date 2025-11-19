#include "testing.h"

/**
 * This test tries to allocate exactly kMaxAllocationSize bytes.
 *
 * Reason(s) you may fail this test:
 * - Incorrect checking what happens when kMaxAllocationSize is requested.
 */

int main() {
  mallocing(kMaxAllocationSize);
}
