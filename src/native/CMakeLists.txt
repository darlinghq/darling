project(native)

include(wrap_elf)

wrap_elf(FreeType libfreetype.so)
wrap_elf(jpeg libjpeg.so)
wrap_elf(png libpng.so)
wrap_elf(tiff libtiff.so)
