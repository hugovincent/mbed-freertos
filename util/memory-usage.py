#!/usr/bin/python

# Prints a concise summary of RAM and flash usage.
# WARNING: Quick and dirty hack that is very, very fragile...
#
# Hugo Vincent, 27 April 2010.

import os, sys
def sh(command):
	return os.popen(command, "r").read()

def format_kb(num_bytes):
	return '%.2f kB' % (num_bytes / 1024.0)

#------------------------------------------------------------------------------
# Parse symbol table
symbols = []
for line in sh('arm-none-eabi-objdump -t %s' % sys.argv[1]).strip().split('\n'):
	parse = line.strip().split()

	# Handle lines without an attribute
	offs = 0
	if len(parse) == 6:
		offs = 1
	# Handle non-symbol lines
	elif len(parse) < 5:
		continue

	# Get segment name and discard uninteresting segments
	segment = parse[2+offs]
	if segment[0:6] == '.debug' or segment in ['.comment', '*ABS*', '.ARM.attributes']:
		continue

	# Get remaining info and store
	size = int(parse[3+offs], 16)
	name = parse[4+offs]
	symbols.append([segment, name, size])

# Parse RAM/flash capacities from linker-script
memories = {}
for line in sh('cat util/lpc2368.ld | grep " (r.) *:"').strip().split('\n'):
	memory = line.strip().split()
	# Parse 'k' suffixes
	memsize = 1024 * int(memory[-1][:-1])
	memories[memory[0]] = memsize

# Parse stack allocations from boot code (assembly)
total_stack = 0
for line in sh('grep "_Stack_Size," util/crt0.s | grep "\.equ"').strip().split('\n'):
	total_stack += int(line.split()[2], 16)

# Parse ELF program headers (for definitive total flash and RAM usage)
program_headers = sh('arm-none-eabi-readelf -l %s' % sys.argv[1])
total_ram = int(program_headers.split('\n')[8].split()[5:-2][0], 16)
total_text = int(program_headers.split('\n')[7].split()[5:-3][0], 16)

#------------------------------------------------------------------------------
# Summarize usage
print
print 'Flash: total code usage:   %s' % format_kb(total_text + total_ram) # Initialized RAM data is loaded from flash
print 'RAM:   total static usage: %s' % format_kb(total_ram)
print '       stack usage:        %s' % format_kb(total_stack)
print '       available heap:     %s' % format_kb(memories['Ram'] - total_stack - total_ram)

#------------------------------------------------------------------------------
# FIXME add a more detailed print out: top 5 flash/RAM users, attempt to summarize library usage

