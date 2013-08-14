#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import re
from subprocess import Popen, PIPE
from datetime import datetime

years = ["2004", "2005", "2008", "2009", "2010", "2011", "2012"]
#years = ["2004"]
base = "/castor/cern.ch/ntof/"
cmd = "nsls"
options = "-l"


# calls the listing command and waits for the results, that is a string
def lsdir ( path ):
	process = Popen([cmd, options, path], stdout=PIPE)
	stdout, stderr = process.communicate()
	
	if stderr is None:
		return stdout
	else:
		sys.stderr.write( "failed to execute \"" + cmd + " " + options + " " + path + "\n" )
		return stderr


# given a string representing the listing output and extract the informations we want.
# That is a tuple made of filename, day, month and year and time (if available, otherwise 00:00).
def parse ( lstr, year, path ):
	dirs = []
	files = []
	
	sys.stderr.write( "Parsing " + path + "...\n" )
	lines = lstr.splitlines()
	total = len( lines )
	index = 0
	for i in lines:
		sys.stderr.write('\r')
        	# the exact output you're looking for:
		sys.stderr.write("[%-20s] %d%%" % ('='* int(index / float(total) * 20), index / float(total) * 100 ))
        	sys.stderr.flush()
        	index += 1
		
		fields = i.split()
		info = fields[0]
		name = fields[8]
		#info, month, dd, yyyy, name = fields[0], fields[5], fields[6], fields[7], fields[8]
		
		timenum = "0"
		# yyyy actually can be a time or an hour :(
		# see https://cern.service-now.com/service-portal/view-incident.do?n=INC342901
		#hour = "00"
		#minute = "00"
		#second = "00"
		
		#if ":" in yyyy:
		#	hour, minute = yyyy.split(":")		
		#	yyyy = year
		#else:
		# if we are on a run file I use the precise command for the date
		if not info.startswith( 'd' ) and name.startswith( "run" ):
		    # skip finding date for segment > 0
			integers = re.findall('\d+', name)
			run = integers[0]
			segment = integers[1]

			if segment == "0":
			  # use a more precise commad to get the creation date and time of the file
				process = Popen(["xrd", "castorpublic", "stat", path + "/" + name], stdout=PIPE)
				stdout, stderr = process.communicate()
				if stderr is None:
					#print( stdout.split()[-1] )
					timenum = stdout.split()[-1]
					#thedate = datetime.fromtimestamp(int(timenum))
					#timestr = thedate.strftime( "%d %m %Y %H %M" )
					#sys.stderr.write( path + " " + timestr + "\n" )
					#month, dd, yyyy, hour, minute = timestr.split()
				else:
					sys.stderr.write( "failed to execute xrd on " + path + "/" + name )
		
		# if the permissions start with "d" it is a directory otherwise a file (the saved informations are the same)
		if info.startswith( 'd' ):
			dirs.append(  (name, timenum) )
		else:
			files.append( (name, timenum) )	
	
	sys.stderr.write('\r')
	return dirs, files


# given a list of filenames extract the run number and its total segments
# the print the final output
def parsefiles ( files, path ):
	segments = dict()
	times = dict()
	
	for f in files:
		if f[0].startswith( "run" ):
			run, segment = re.findall('\d+', f[0])
			if segment == '0':
				times[run] = f[1]
				segments[run] = '0'
			
			if run in segments:
				segments[run] = str( max( int(segments[run]), int(segment) ) )
			else:
				segments[run] = segment
		else:
			sys.stderr.write( "WARNING: found file " + f + " in " + path + "\n" )
	
	#out = []
	
	# final output is in the following format RUN TIME TOTALSEGMENTS PATH
	# here I build a simple list of tuples where each tuple has as first element the run string
	# and as second element the informations we want (still as a string)
	for run in segments:
		if run not in times:
			sys.stderr.write( "WARNING: run " + run + " has no segment 0, skipping it (experiment " + path + ")\n" )
		else:
			print( run + "\t" + times[run] + "\t" + str(int(segments[run]) + 1) + "\t" + path)
			#out.append( (run, "\t" + times[run][0] + "\t" + times[run][1] + "\t" +  times[run][2] + "\t" + 
			#						      times[run][3] + ":" + times[run][4] + "\t" + str(int(segments[run]) + 1) + "\t" + path) )
	
	#return out



def browse( ):
	#finaloutput = []
	
	for year in years:
		i = base + year
		output = lsdir( i )
		# experimentS level
		dirs,files = parse( output, year, i )
		
		for j in dirs:
			# XXX
			#if j[0] != "fission" and j[0] != "TAC6":
			#	continue
				
			current = i + "/" + j[0]
			#print( "experiment " + j[3] + "/" + j[0] + " at " + current )
			output = lsdir( current )
			dirs,files = parse( output, year, current )
			
			# experiment level, check if stream0 directory exists
			# I read only stream0 because the date and the number of segments is the same for any stream
			# and the date and the total segments are the only informations I can get without actually
			# reading the content of the file
			names = [t[0] for t in dirs]
			if "stream0" in names:
				# parse the first stream to get informations about the runs, segments, etc
				#current += "/stream0"
				output = lsdir( current + "/stream0" )
				dirs,files = parse( output, year, current + "/stream0" )
				
				# get informations about runs and segments
				# notice the extend instead of append, we are operating on lists
				#finaloutput.extend( parsefiles( files, current ) )
				parsefiles( files, current )
			else:
				sys.stderr.write( "WARNING: no folder stream0 for experiment " + current + ", skipping\n" )
	
	#return sorted( finaloutput, key=lambda x: int(x[0]) )




if __name__ == '__main__':
	# take as argument the base CASTOR directory for the experiment
	if len( sys.argv ) > 1:
		base = sys.argv[1]
	
	#output = browse( )
	browse()
	
	#for run in output:
	#	print( run[0] + run[1] )
	
	
