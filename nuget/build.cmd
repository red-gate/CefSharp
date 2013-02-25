call "%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat" x86
set ARGS=
if not [%1]==[] set ARGS=/p:NuSpec=%1
msbuild nuget.proj /t:BuildPackage %ARGS%
