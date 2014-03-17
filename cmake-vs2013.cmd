@echo on

pushd .

7za x deps-vs2013.7z -y

SET project_path="%CD%\demo\"
SET build_path="%CD%\build_vs2013\"
mkdir %build_path%> NUL

SET build_folder=build_vs2013
cd %build_path%

cmake -G"Visual Studio 12" %project_path% 
cmake-gui %build_path% 

popd