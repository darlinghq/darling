int main(void)
{ 
  extern void _keymgr_set_per_thread_data(unsigned int key, void *keydata) ;
  _keymgr_set_per_thread_data (500, 0);
  return 0;
}
