#ifndef KEYMGR_H
#define KEYMGR_H

extern "C" {

// Only these are needed for Unwind
void* _keymgr_get_and_lock_processwide_ptr(unsigned key);
int _keymgr_set_and_unlock_processwide_ptr(unsigned key, void* ptr);

}

#endif

