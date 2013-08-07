#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import re
import fileinput

if __name__ == '__main__':
	previous = ""
	previoussum = 0.0
	previouscount = 0
	
	for line in fileinput.input():
		#print( line.strip() )
		
		time, value = line.strip().split()
		
		if previous != time and previous != "":
			print( "{0} {1}".format( previous, previoussum / previouscount ) )
			previous = time
			previoussum = float( value )
			previouscount = 1
		else:	
			previoussum += float( value )
			previouscount += 1
			previous = time
		
		
		#sys.stdin.read(1)
