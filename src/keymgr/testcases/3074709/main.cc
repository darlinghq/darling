#include <mach-o/dyld.h>
#include <stdio.h>

typedef void (*thrower)();
typedef void (*call_thrower)(thrower);
typedef void (*call_call_thrower)(call_thrower);

int main()
{
  NSObjectFileImage image, image2;
  NSObjectFileImageReturnCode retCode;
  NSModule module, module2;
  NSSymbol sym;
  call_call_thrower func;
  call_thrower do_call_thrower;

  retCode = NSCreateObjectFileImageFromFile ("plugin2", &image2);
  if (retCode != NSObjectFileImageSuccess)
    {
      fprintf (stderr, "failed to load plugin2\n");
      return 1;
    }
  module2 = NSLinkModule(image2, "plugin2", 
			(NSLINKMODULE_OPTION_BINDNOW 
			 | NSLINKMODULE_OPTION_PRIVATE));
  
  sym = NSLookupSymbolInModule (module2, "_do_call_thrower");
  if (sym == NULL)
    {
      fprintf (stderr, "couldn't find `do_call_thrower' in plugin2\n");
      return 1;
    }
  do_call_thrower = (call_thrower) NSAddressOfSymbol (sym);
  
  retCode = NSCreateObjectFileImageFromFile ("plugin", &image);
  if (retCode != NSObjectFileImageSuccess)
    {
      fprintf (stderr, "failed to load plugin\n");
      return 1;
    }
  module = NSLinkModule(image, "plugin", 
			(NSLINKMODULE_OPTION_BINDNOW 
			 | NSLINKMODULE_OPTION_PRIVATE));
  sym = NSLookupSymbolInModule (module, "_func");
  if (sym == NULL)
    {
      fprintf (stderr, "couldn't find `func' in plugin\n");
      return 1;
    }
  func = (call_call_thrower) NSAddressOfSymbol (sym);
  func (do_call_thrower);

  NSDestroyObjectFileImage (image);
  NSUnLinkModule (module, NSUNLINKMODULE_OPTION_RESET_LAZY_REFERENCES);

  NSDestroyObjectFileImage (image2);
  NSUnLinkModule (module2, NSUNLINKMODULE_OPTION_RESET_LAZY_REFERENCES);

  return 0;
}
