import subprocess

path = "/home/james/darling/src/external/xnu/darling/src/libsystem_kernel/libsyscall/mach/mach_vm.c"

with open(path, 'r') as f:
    content = f.read()

content = content.replace(
    "\tkern_return_t rv;\n\n\trv = _kernelrpc_vm_map(target, address, size, mask, flags, object,\n\t    offset, copy, cur_protection, max_protection, inheritance);",
    "\tkern_return_t rv;\n\tmach_vm_address_t mach_addr;\n\n\tmach_addr = (mach_vm_address_t)*address;\n\trv = _kernelrpc_vm_map(target, (vm_address_t *)&mach_addr, size, mask, flags, object,\n\t    offset, copy, cur_protection, max_protection, inheritance);\n\t*address = (vm_address_t)mach_addr;"
)

content = content.replace(
    "\tkern_return_t rv;\n\n\trv = _kernelrpc_vm_remap(target, address, size, mask, flags,\n\t    src_task, src_address, copy, cur_protection, max_protection,\n\t    inheritance);",
    "\tkern_return_t rv;\n\tmach_vm_address_t mach_addr;\n\n\tmach_addr = (mach_vm_address_t)*address;\n\trv = _kernelrpc_vm_remap(target, (vm_address_t *)&mach_addr, size, mask, flags,\n\t    src_task, src_address, copy, cur_protection, max_protection,\n\t    inheritance);\n\t*address = (vm_address_t)mach_addr;"
)

content = content.replace(
    "\tkern_return_t rv;\n\n\t/* {max,cur}_protection is inout */\n\trv = _kernelrpc_vm_remap_new(target, address, size, mask, flags,\n\t    src_task, src_address, copy, cur_protection, max_protection,\n\t    inheritance);",
    "\tkern_return_t rv;\n\tmach_vm_address_t mach_addr;\n\n\tmach_addr = (mach_vm_address_t)*address;\n\t/* {max,cur}_protection is inout */\n\trv = _kernelrpc_vm_remap_new(target, (vm_address_t *)&mach_addr, size, mask, flags,\n\t    src_task, src_address, copy, cur_protection, max_protection,\n\t    inheritance);\n\t*address = (vm_address_t)mach_addr;"
)

with open(path, 'w') as f:
    f.write(content)

result = subprocess.run(['grep', '-n', 'mach_vm_address_t mach_addr', path], capture_output=True, text=True)
print(result.stdout if result.stdout else "WARNING: No matches found")
