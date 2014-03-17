@echo off

pushd .

SET project_path="%CD%\demo\"
SET build_path="%CD%\build_vs2012\"
cmake-gui %build_path% 

popd