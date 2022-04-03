@echo off

mkdir ".\bin\"
mkdir ".\bin\"

cd ".\bin\"

echo "Running CMake with build type 'Debug'..."
cmake ".." -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug

cmake --build . --config Debug