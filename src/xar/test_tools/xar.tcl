
big_endian

requires 0 "78617221" ;# xar!
ascii 4 "Signature"
set headersize [uint16 "Header Size"]
uint16 "Version"
set compressed_length [uint64 "TOC Compressed Length"]
uint64 "TOC Uncompressed Length"
uint32 "Checksum"
set compressed_data [bytes $compressed_length "TOC Data"]
set xml [zlib_uncompress $compressed_data]

#puts "${xml}"
package require tdom
set doc [dom parse $xml]
set root [$doc documentElement]
set hashes [$root selectNodes "//checksum"]
set signatures [$root selectNodes "//signature"]

section "Hashes"
foreach hash $hashes {
	section [$hash getAttribute "style"] {
		set lengthString [[lindex [$hash getElementsByTagName "size"] 0] text]
		set offsetString [[lindex [$hash getElementsByTagName "offset"] 0] text]
		scan $lengthString "%d" length
		scan $offsetString "%d" heapoffset
		set offset [expr { $heapoffset + $headersize + $compressed_length}]
		entry "Checksum" "<DATA>"  $length $offset
	}
}
endsection 

section "Signatures"
foreach signature $signatures {
	section [$signature getAttribute "style"] {
		set lengthString [[lindex [$signature getElementsByTagName "size"] 0] text]
		set offsetString [[lindex [$signature getElementsByTagName "offset"] 0] text]
		scan $lengthString "%d" length
		scan $offsetString "%d" heapoffset 		
		set offset [expr { $heapoffset + $headersize + $compressed_length}]
		entry "Signature Blob" "<DATA>"  $length $offset
	}
}
endsection

set nodes [$root selectNodes "//file"]
section "Files" {
	foreach node $nodes {
		section [[lindex [$node getElementsByTagName "name"] 0] text] {
			entry "ID" [$node getAttribute "id"]
			entry "Type" [[lindex [$node getElementsByTagName "type"] 0] text]
			
			set dataNode [lindex [$node getElementsByTagName "data"] 0]
			set compressedSizeString [[lindex [$dataNode getElementsByTagName "length"] 0] text]
			set offsetString [[lindex [$dataNode getElementsByTagName "offset"] 0] text]
			
			scan $compressedSizeString "%d" compressedSize
			scan $offsetString "%d" heapoffset
			
			set offset [expr { $heapoffset + $headersize + $compressed_length}]
			
			entry "Size" [[lindex [$dataNode getElementsByTagName "size"] 0] text]
			entry "Compressed Size" $compressedSizeString
			entry "Compressed Blob" "<DATA>" $compressedSize $offset

		}
	}
}