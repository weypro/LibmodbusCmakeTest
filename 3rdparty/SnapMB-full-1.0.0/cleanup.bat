del /s *.obj
del /s *.ipch
del /s *.pch
del /s *.idb
del /s *.pdb
del /s *.tlog
del /s *.ilk
del /s *.dbg

del /s /q /build/temp/win32/*.*
del /s /q /build/temp/win64/*.*
del /s /q /build/temp/mingw32/*.*
del /s /q /build/temp/mingw64/*.*
del /s /q /build/temp/linux/*.*
del /s /q /build/temp/bsd/*.*
del /s /q /build/temp/macOS/*.*
del /s /q /tools/windows/Lazarus/lib/i386-win32/*.*
del /s /q /tools/windows/Lazarus/lib/x86_64-win64/*.*

rmdir /Q /S /build/temp/win32/SnapMB.tlog
rmdir /Q /S /build/temp/win64/SnapMB.tlog
rmdir /Q /S /build/windows/.vs
rmdir /Q /S /examples/c#/.vs
rmdir /Q /S /examples/cpp_embedded/.vs
rmdir /Q /S /examples/cpp/Windows/VisualStudio/.vs
