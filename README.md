# memjector
A project to track memory leaks which happen between specific points in time by attaching and detaching modules to a running process

## Dependencies
- boost (tested with 1.60.0)
- zlib (required to build boost::iostreams with zlib support on Windows)
- Microsoft Platform SDK
- Microsoft Debugging Tools for Windows (for dbghelp.dll)
