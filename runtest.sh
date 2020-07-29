#!/bin/bash

/usr/bin/time -f "Elapsed (wall clock) time (h:mm:ss or m:ss): %E\nMaximum resident set size in kibibytes: %M" ./exactDP < $1
