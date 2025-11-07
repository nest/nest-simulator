#!/bin/sh
# Prevent any output, result will be interpreted based on exit code.
# Output would confuse parser.
diff sender-1-0.dat receiver-1-0.dat >/dev/null 2>&1
