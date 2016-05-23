# memjector
A project to track memory leaks which happen between specific points in time by attaching and detaching modules to a running process

## Supported
Currently only supports Windows x86 processes.  Mixed processes running both .NET and native code are supported, but only the native callstack will be able to be tracked.

## Dependencies
- boost (tested with 1.60.0)
- zlib (required to build boost::iostreams with zlib support on Windows)
- Microsoft Platform SDK
- Microsoft Debugging Tools for Windows (for dbghelp.dll)
