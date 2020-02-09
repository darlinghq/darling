#ifdef __cplusplus
extern "C" {
#endif

const char* xtrace_msg_type_to_str(mach_msg_type_name_t type_name, int full);
int xtrace_kern_return_to_str(char* buf, kern_return_t kr);

#ifdef __cplusplus
}
#endif

