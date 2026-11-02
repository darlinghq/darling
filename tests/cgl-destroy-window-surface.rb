#!/usr/bin/env ruby
# frozen_string_literal: true

require 'open3'
require 'fileutils'

root = ARGV[0] || File.expand_path('..', __dir__)
opengl_c_path = File.join(root, 'src/frameworks/OpenGL/OpenGL.c')

abort("OpenGL.c not found at #{opengl_c_path}") unless File.exist?(opengl_c_path)

src = File.read(opengl_c_path)

abort('FAIL: OpenGL.c must maintain an active context registry') unless src.include?('g_activeContexts')
abort('FAIL: OpenGL.c must implement CGLClearDrawable') unless src.include?('CGLClearDrawable')
abort('FAIL: CGLClearDrawable must not be an empty stub') if src.include?('STUB: CGLClearDrawable')

cgl_destroy_split = src.split('void CGLDestroyWindow')
abort('FAIL: CGLDestroyWindow definition not found in OpenGL.c') if cgl_destroy_split.length < 2
cgl_destroy_body = cgl_destroy_split[1].split('CGL_EXPORT')[0]
abort('FAIL: CGLDestroyWindow must scan active contexts') unless cgl_destroy_body.include?('g_activeContexts')

test_src = File.join(root, 'tests/cgl-destroy-window-surface.m')
abort("Test source not found at #{test_src}") unless File.exist?(test_src)

# Detect whether darling shell can be run, or if we are already inside Darling
has_darling = system('darling shell true >/dev/null 2>&1')

guest_bin = '/tmp/cgl_destroy_window_test_rb'
src_path = has_darling ? "/Volumes/SystemRoot#{test_src}" : test_src
compile_cmd = [
  'clang',
  '-framework', 'Foundation',
  '-framework', 'AppKit',
  '-framework', 'OpenGL',
  src_path,
  '-o', guest_bin
]
compile_cmd.unshift('darling', 'shell') if has_darling

begin
  out, err, status = Open3.capture3(*compile_cmd)
  abort("Compilation failed:\n#{err}\n#{out}") unless status.success?

  display = ENV['DISPLAY'] || ':1'
  run_cmd = if has_darling
              ['darling', 'shell', 'bash', '-c', "DISPLAY=#{display} #{guest_bin}"]
            else
              ['bash', '-c', "DISPLAY=#{display} #{guest_bin}"]
            end
  out, err, status = Open3.capture3(*run_cmd)
  abort("Execution failed (code #{status.exitstatus}):\n#{err}\n#{out}") unless status.success?
  abort("Execution did not report PASS:\n#{out}") unless out.include?('PASS:')

  puts 'PASS: CGLDestroyWindow and CGLClearDrawable disassociate context surface verified'
ensure
  cleanup_cmd = has_darling ? ['darling', 'shell', 'rm', '-f', guest_bin] : ['rm', '-f', guest_bin]
  Open3.capture3(*cleanup_cmd)
end
