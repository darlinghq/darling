python sys.path.insert(0, '.')
python import gdb_maloader

define mreload
  python reload(gdb_maloader)
end

define mbt
  mreload
  python gdb_maloader.bt()
end
