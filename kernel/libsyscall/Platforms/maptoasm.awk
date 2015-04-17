/^(\S+)\s+(\S+)$/ {
	print ".globl",$1
	print $1,"=",$2
}

