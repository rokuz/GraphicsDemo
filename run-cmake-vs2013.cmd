@echo off

pushd .

SET project_path="%CD%\demo\"
SET build_path="%CD%\build_vs2013\"
cmake-gui %build_path% 

popd