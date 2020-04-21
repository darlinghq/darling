#!/usr/bin/env ruby

require 'yaml'

if ENV["DRIVERKITROOT"]
    $availCmd = ENV["SDKROOT"] + ENV["DRIVERKITROOT"] + "/usr/local/libexec/availability.pl";
else
    $availCmd = ENV["SDKROOT"] + "/usr/local/libexec/availability.pl";
end

def versionString(vers)
    uvers = ""
    if vers =~ /^(\d+)$/
        uvers = "#{$1}_0"
    elsif vers =~ /^(\d+).(\d+)$/
        uvers = "#{$1}_#{$2}"
    elsif vers =~ /^(\d+).(\d+).(\d+)$/
        if $3 == 0
            uvers = "#{$1}_#{$2}"
        else
            uvers = "#{$1}_#{$2}_#{$3}"
        end
    end
    uvers
end

def versionHex(vers)
    major = 0;
    minor = 0;
    revision = 0;

    if vers =~ /^(\d+)$/
        major = $1.to_i;
    elsif vers =~ /^(\d+).(\d+)$/
        major = $1.to_i;
        minor = $2.to_i;
    elsif vers =~ /^(\d+).(\d+).(\d+)$/
        major = $1.to_i;
        minor = $2.to_i;
        revision = $3.to_i;
    end
    "0x00#{major.to_s(16).rjust(2, '0')}#{minor.to_s(16).rjust(2, '0')}#{revision.to_s(16).rjust(2, '0')}"
end

def expandVersions(prefix, arg)
    versionList = `#{$availCmd} #{arg}`.gsub(/\s+/m, ' ').strip.split(" ")
    versionList.each { |version|
        puts "#define #{prefix}#{versionString(version)}".ljust(48, ' ') + versionHex(version)
    }
end

def expandPlatformVersions(prefix, platform, arg)
    versionList = `#{$availCmd} #{arg}`.gsub(/\s+/m, ' ').strip.split(" ")
    versionList.each { |version|
        puts "static dyld_build_version_t dyld_platform_version_#{prefix}_#{versionString(version)}".ljust(72, ' ') + "= { .platform = #{platform}, .version = #{versionHex(version)} };"
    }
end

def versionSetsForOSes(versionSets, key, platform, target)
    puts "#if #{target}"
    versionSets.each { |k,v|
        puts "#define dyld_#{k}_os_versions dyld_platform_version_#{platform}_#{versionString(v[key].to_s)}"
    }
    puts "#endif /* #{target} */"
end

def expandVersionSets()
    versionSets = YAML.load(`#{$availCmd} --sets`)
    versionSetsForOSes(versionSets, "macos", "macOS", "TARGET_OS_OSX")
    versionSetsForOSes(versionSets, "ios", "iOS", "TARGET_OS_IOS")
    versionSetsForOSes(versionSets, "tvos", "tvOS", "TARGET_OS_TV")
    versionSetsForOSes(versionSets, "watchos", "watchOS", "TARGET_OS_WATCH")
    versionSetsForOSes(versionSets, "bridgeos", "bridgeOS", "TARGET_OS_BRIDGE")
end

ARGF.each do |line|
    if line =~ /^\/\/\@MAC_VERSION_DEFS\@$/
        expandVersions("DYLD_MACOSX_VERSION_", "--macosx")
    elsif line =~ /^\/\/\@IOS_VERSION_DEFS\@$/
        expandVersions("DYLD_IOS_VERSION_", "--ios")
    elsif line =~ /^\/\/\@WATCHOS_VERSION_DEFS\@$/
        expandVersions("DYLD_WATCHOS_VERSION_", "--watchos")
    elsif line =~ /^\/\/\@TVOS_VERSION_DEFS\@$/
        expandVersions("DYLD_TVOS_VERSION_", "--appletvos")
    elsif line =~ /^\/\/\@BRIDGEOS_VERSION_DEFS\@$/
        expandVersions("DYLD_BRIDGEOS_VERSION_", "--bridgeos")
    elsif line =~ /^\/\/\@MACOS_PLATFORM_VERSION_DEFS\@$/
        expandPlatformVersions("macOS", "PLATFORM_MACOS", "--macosx")
    elsif line =~ /^\/\/\@IOS_PLATFORM_VERSION_DEFS\@$/
        expandPlatformVersions("iOS", "PLATFORM_IOS", "--ios")
    elsif line =~ /^\/\/\@WATCHOS_PLATFORM_VERSION_DEFS\@$/
        expandPlatformVersions("watchOS", "PLATFORM_WATCHOS", "--watchos")
    elsif line =~ /^\/\/\@TVOS_PLATFORM_VERSION_DEFS\@$/
        expandPlatformVersions("tvOS", "PLATFORM_TVOS", "--appletvos")
    elsif line =~ /^\/\/\@BRIDGEOS_PLATFORM_VERSION_DEFS\@$/
        expandPlatformVersions("bridgeOS", "PLATFORM_BRIDGEOS", "--bridgeos")
    elsif line =~ /^\/\/\@VERSION_SET_DEFS\@$/
        expandVersionSets()
    else
        puts line
    end
end
