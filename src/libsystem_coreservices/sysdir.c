#include <NSSystemDirectories.h>
#include <sysdir.h>

sysdir_search_path_enumeration_state sysdir_start_search_path_enumeration(sysdir_search_path_directory_t dir,
		sysdir_search_path_domain_mask_t domainMask)
{
	return NSStartSearchPathEnumeration((NSSearchPathDirectory) dir, (NSSearchPathDomainMask) domainMask);
}

sysdir_search_path_enumeration_state sysdir_get_next_search_path_enumeration(sysdir_search_path_enumeration_state state, 
		char *path)
{
	return (sysdir_search_path_enumeration_state) NSGetNextSearchPathEnumeration((NSSearchPathEnumerationState) state, path);
}

