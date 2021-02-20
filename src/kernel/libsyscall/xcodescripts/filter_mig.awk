#!/usr/bin/awk -f

# Usage: foo <template> <file>
# Searches through file for instances of 'kern_return_t $FOO'
# where $FOO is an line in the template file
# and prepends the first line in the template file.

# Example template format:
#       __WATCHOS_PROHIBITED
#       act_get_state
#       thread_get_state
# 

# BEGIN { print ARGV[1]; print ARGV[2] }

# In the first file, build array of lines
NR==FNR {
	if (NR==1)
		prefix=$0
	else
		templates[$0];
	next
}

# In the second file, match kern_return_t <template>
# at the beginning of the line
# print the prefix line if found

/^kern_return_t/ {
#	print "match"
	if ($2 in templates) {
		print prefix
	}
}

# Pass through everything in the second file
{ print }
