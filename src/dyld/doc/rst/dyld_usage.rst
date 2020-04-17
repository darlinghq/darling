dyld_usage - report dynamic linker activity in real-time
==========================================================

SYNOPSIS
--------

:program:`dyld_usage` **[-e] [-f mode] [-t seconds] [-R rawfile [-S start_time]
[-E end_time]] [pid | cmd [pid | cmd] ...]**

DESCRIPTION
-----------
The :program:`dyld_usage` utility presents an ongoing display of information
pertaining to dynamic linker activity within one or more processes. It requires
root privileges due to the kernel tracing facility it uses to operate. By
default :program:`dyld_usage` monitors `dyld` activity in all processes except
the running `dyld_usage` process, `Terminal`, `telnetd`, `sshd`, `rlogind`,
`tcsh`, `csh`, and `sh`. These defaults can be overridden such that output is
limited to include or exclude a list of processes specified on the command line.
Processes may be specified either by file name or by process id.

The output of :program:`dyld_usage` is formatted according to the size of your
window. A narrow window will display fewer columns of data. Use a wide window
for maximum data display.

OPTIONS
-------
:program:`dyld_usage` supports the following options:

.. option:: -e

  Exclude the specified list of pids and commands from the sample, and exclude
  :program:`dyld_usage` by default.

.. option:: -R

  specifies a raw trace file to process.

.. option:: -t

  specifies timeout in seconds (for use in automated tools).

DISPLAY
-------

The data columns displayed are as follows:

  TIMESTAMP
    Time of day when call occurred.

  OPERATION
    The `dyld` operation triggered by the process. Typically these operations
    are triggered by process launch or via a ``dlopen`` or ``dlsym`` system
    call. System call entries include both the parameters to the system call and
    the system call's return code (e.g., 0 on success).
    
  TIME INTERVAL
    The elapsed time spent in the dynamic linker operation or system call.

  PROCESS NAME
    The process that generated the dynamic linker activity. If space allows, the
    thread id will be appended to the process name (i.e., Mail.nnn).


SAMPLE USAGE
------------

 ``sudo dyld_usage Mail``
 
 :program:`dyld_usage` will display dynamic link operations for all instances of
 processes named Mail.

SEE ALSO
--------

:manpage:`dyld(1)`, :manpage:`fs_usage(1)`

