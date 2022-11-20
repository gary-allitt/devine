@echo off 
rem use cygwin to work around this issue 
rem https://stackoverflow.com/questions/11137702/rd-exits-with-errorlevel-set-to-0-on-error-when-deletion-fails-etc
if exist prepare rm -rf prepare
if %ERRORLEVEL% neq 0 (
  echo failed 
  goto eof 
)
if exist package rm -rf package
if %ERRORLEVEL% neq 0 (
  echo failed 
  goto eof 
)
md package 
md prepare
pushd prepare
copy ..\release
windeployqt .
del moc_* 
del *.log
del *.recipe
del *.pdb
del *.tlog
del *.res
del *vcxproj* 
del *.obj 
del *.cbt
del d3dcompiler_47.dll
del opengl32sw.dll
pkzip25 -add -pat -rec devine *
copy devine.zip ..\package
:eof
popd
rm -rf prepare
