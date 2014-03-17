@echo off

pushd .

7za x deps-vs2012.7z -y

SET project_path="%CD%\demo\"
SET build_path="%CD%\build_vs2012\"
mkdir %build_path%> NUL

SET build_folder=build_vs2012
cd %build_path%

cmake -G"Visual Studio 11" %project_path% 
cmake-gui %build_path% 

popd