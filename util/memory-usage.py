#!/usr/bin/python

# Prints a concise summary of RAM and flash usage.
# WARNING: Quick and dirty hack that is very, very fragile...
#
# Hugo Vincent, 27 April 2010.

import os, sys
def sh(command):
	return os.popen(command, "r").read()

def format_kb(num_bytes):
	return '%5.1f kB' % (num_bytes / 1024.0)

target = sys.argv[1]

#------------------------------------------------------------------------------
# Parse symbol table
symbols = []
for line in sh('arm-none-eabi-objdump -t %s' % sys.argv[2]).strip().split('\n'):
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
for line in sh('cat mach/cpu-' + target + '/' + target + '.ld | grep " (r.*) *:"').strip().split('\n'):
	memory = line.strip().split()
	# Parse 'k' suffixes FIXME
	memsize = 1024 * int(memory[-1][:-1])
	memories[memory[0]] = memsize

# Parse stack allocations from linker output
if target == 'lpc1768':
	line = sh('grep "Stack_Size_Total" %s.map' % "RTOSDemo.elg".split('.')[0]).strip().split('\n')[0]
	total_stack = int(line.split()[-1], 16)
else:
	total_stack = 0
	for line in sh('grep "_Stack_Size," mach/cpu-' + target + '/crt0.s | grep "\.equ"').strip().split('\n'):
		if line.strip() == "":
			continue
		total_stack += int(line.split()[2], 16)

# Parse ELF program headers (for definitive total flash and RAM usage)
for line in sh('arm-none-eabi-readelf -l %s' % sys.argv[2]).strip().split('\n'):
	parse = line.strip().split()
	if len(parse) > 0 and parse[0] == 'LOAD':
		if parse[6] == 'RW':
			total_ram = int(parse[5], 16)
			total_init_ram = int(parse[4], 16)
		elif parse[6] == 'R' and parse[7] == 'E':
			total_text = int(parse[5], 16)

#------------------------------------------------------------------------------
# Summarize usage (note: sizes are approximate due to padding for alignment)
print
print 'Flash: total code usage:       %s' % format_kb(total_text + total_init_ram)
print 'RAM:   stack usage:            %s' % format_kb(total_stack)
print '       static heap usage:      %s' % format_kb(total_ram)
print '       dynamic heap available: %s (approx.)' % format_kb(memories['Ram'] - \
		total_stack - total_ram)

#------------------------------------------------------------------------------
# FIXME add a more detailed print out: top 5 flash/RAM users, attempt to summarize library usage

