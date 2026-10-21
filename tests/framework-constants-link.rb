# Host test: the CFString constants a framework defines are defined once.
# usage: ruby tests/framework-constants-link.rb [REPO_ROOT]
# For each framework, the single-line "const CFStringRef k... = CFSTR(...)"
# definitions are cut out of every source file in its CMake SOURCES list,
# compiled one object per file against a stand-in CFSTR, and linked into one
# shared object, as the framework build does. A symbol defined in two files
# fails the link ("multiple definition" or "duplicate symbol").
require 'tmpdir'
require 'open3'
root = ARGV.fetch(0, File.expand_path('..', __dir__))
frameworks = %w[VideoToolbox CoreVideo MediaAccessibility]
DEF = /^\s*(?:const\s+CFStringRef|CFStringRef\s+const)\s+(k\w+)\s*=\s*CFSTR\("[^"]*"\);/
# Name, framework, defining text. Each added constant must be defined exactly
# once in its framework's sources, with this type and value, and be declared in
# a public header with the same type. CoreLocation and ImageCaptureCore are
# checked here too: they are not CFSTR constants the link step above can cut out.
EXPECTED = {
  'kCLLocationAccuracyBestForNavigation' => ['CoreLocation', 'CLLocationAccuracy', '-2.0'],
  'kCLLocationAccuracyBest' => ['CoreLocation', 'CLLocationAccuracy', '-1.0'],
  'kCLLocationAccuracyNearestTenMeters' => ['CoreLocation', 'CLLocationAccuracy', '10.0'],
  'kCLLocationAccuracyHundredMeters' => ['CoreLocation', 'CLLocationAccuracy', '100.0'],
  'kCLLocationAccuracyKilometer' => ['CoreLocation', 'CLLocationAccuracy', '1000.0'],
  'kCLLocationAccuracyThreeKilometers' => ['CoreLocation', 'CLLocationAccuracy', '3000.0'],
  'ICOverwrite' => ['ImageCaptureCore', 'ICDownloadOption', '@"ICOverwrite"'],
  'ICSaveAsFilename' => ['ImageCaptureCore', 'ICDownloadOption', '@"ICSaveAsFilename"'],
  'ICSavedFilename' => ['ImageCaptureCore', 'ICDownloadOption', '@"ICSavedFilename"'],
  'ICTransportTypeMassStorage' => ['ImageCaptureCore', 'ICDeviceTransport', '@"ICTransportTypeMassStorage"'],
  'kMACaptionAppearanceSettingsChangedNotification' => ['MediaAccessibility', 'CFStringRef', 'CFSTR("com.apple.mediaaccessibility.captionAppearanceSettingsChanged")'],
  'kVTCompressionPropertyKey_BaseLayerFrameRateFraction' => ['VideoToolbox', 'CFStringRef', 'CFSTR("BaseLayerFrameRateFraction")'],
  'kVTVideoEncoderSpecification_EnableLowLatencyRateControl' => ['VideoToolbox', 'CFStringRef', 'CFSTR("EnableLowLatencyRateControl")'],
  'kCVImageBufferTransferFunction_sRGB' => ['CoreVideo', 'CFStringRef', 'CFSTR("IEC_sRGB")'],
  'kCVBufferPropagatedAttachmentsKey' => ['CoreVideo', 'CFStringRef', 'CFSTR("PropagatedAttachments")'],
  'kCVImageBufferContentLightLevelInfoKey' => ['CoreVideo', 'CFStringRef', 'CFSTR("ContentLightLevelInfo")'],
  'kCVImageBufferMasteringDisplayColorVolumeKey' => ['CoreVideo', 'CFStringRef', 'CFSTR("MasteringDisplayColorVolume")'],
  'kCVPixelBufferPoolAllocationThresholdKey' => ['CoreVideo', 'CFStringRef', 'CFSTR("BufferPoolAllocationThreshold")'],
}
failed = false
EXPECTED.each do |name, (fw, type, value)|
  base = "#{root}/src/frameworks/#{fw}"
  defs = Dir.glob("#{base}/src/**/*.{c,m,mm}").flat_map { |f| File.readlines(f).grep(/\b#{name}\s*=/) }
  ok = defs.size == 1 && defs[0] =~ /\A\s*(?:const\s+#{type}|#{type}\s+const)\s+#{name}\s*=\s*#{Regexp.escape(value)};/
  unless ok
    puts "FAIL #{fw}: #{name} must be defined once as #{type} = #{value}; found #{defs.map(&:strip).inspect}"
    failed = true
  end
  headers = Dir.glob("#{base}/include/**/*.h").map { |h| File.read(h) }.join
  unless headers =~ /extern\s+(?:const\s+#{type}|#{type}\s+const)\s+#{name}\s*;/
    puts "FAIL #{fw}: #{name} is not declared in a public header as #{type}"
    failed = true
  end
end
abort 'framework constants regression' if failed

Dir.mktmpdir('constants-link-') do |dir|
  File.write("#{dir}/cf.h", "typedef const struct __CFString *CFStringRef;\n#define CFSTR(x) ((CFStringRef) (x))\n")
  failed = false
  frameworks.each do |fw|
    base = "#{root}/src/frameworks/#{fw}"
    cmake = File.read("#{base}/CMakeLists.txt")
    list = cmake[/SOURCES(.*?)DEPENDENCIES/m, 1] or abort "#{fw}: no SOURCES"
    # CFSTR needs the CoreFoundation constant-string class at link time, directly or through Foundation.
    deps = cmake[/DEPENDENCIES(.*?)\)/m, 1].to_s.split
    uses_cfstr = list.scan(%r{\S+\.(?:c|m|mm)\b}).any? { |src| File.read("#{base}/#{src}").include?('CFSTR(') }
    if uses_cfstr && !(deps & %w[CoreFoundation Foundation]).any?
      puts "FAIL #{fw}: sources use CFSTR but DEPENDENCIES lists neither CoreFoundation nor Foundation"
      failed = true
    end
    objs = []
    names = []
    list.scan(%r{\S+\.(?:c|m|mm)\b}).each_with_index do |src, i|
      defs = File.readlines("#{base}/#{src}").select { |l| l =~ DEF }
      next if defs.empty?
      names.concat(defs.map { |l| l[DEF, 1] })
      File.write("#{dir}/#{fw}#{i}.c", "#include \"cf.h\"\n" + defs.join)
      out, st = Open3.capture2e('clang', '-fPIC', '-c', "#{dir}/#{fw}#{i}.c", '-o', "#{dir}/#{fw}#{i}.o")
      abort out unless st.success?
      objs << "#{dir}/#{fw}#{i}.o"
    end
    out, st = Open3.capture2e('clang', '-shared', *objs, '-o', "#{dir}/#{fw}.so")
    unless st.success?
      puts "FAIL #{fw}: #{out.lines.grep(/multiple definition|duplicate symbol/).first(5).join}"
      failed = true
      next
    end
    puts "#{fw}: #{names.size} constants link once"
  end
  abort 'framework constants regression' if failed
  puts 'PASS: framework constants link once'
end
