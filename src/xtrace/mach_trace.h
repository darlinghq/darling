#ifdef __cplusplus
extern "C" {
#endif

const char* xtrace_msg_type_to_str(mach_msg_type_name_t type_name, int full);
void xtrace_print_kern_return(kern_return_t kr);

#ifdef __cplusplus
}
#endif

