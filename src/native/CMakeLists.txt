project(native)

include(wrap_elf)

wrap_elf(FreeType libfreetype.so)
wrap_elf(jpeg libjpeg.so)
wrap_elf(png libpng.so)
wrap_elf(tiff libtiff.so)
wrap_elf(X11 libX11.so)
wrap_elf(cairo libcairo.so)
wrap_elf(GL libGL.so)
