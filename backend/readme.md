# People Party Backend

This is the c++ project of the People Party backend! This project uses CMake,
Conan and doctest. Tests are written using the doctest testing framework.
All dependencies should be automatically installed and imported by Conan so it should be 
easy to get started.

## Getting started
1. Clone the repo
```sh
git clone https://github.com/Kraanter/PeopleParty.git
```
or
```sh
git clone git@github.com:Kraanter/PeopleParty.git
```
when using ssh authentication.

2. Install Conan
See [conan.io](https://conan.io/downloads)

3. Install CMake
Find the correct distribution on [this page](https://cmake.org/download/) and 
install it. When using some fancy IDE like CLion it will already be included in 
there.

4. Install dependancies
Go to the root of the backend project
```sh
cd backend
```
run conan install
```sh
conan install . --output-folder=build --build=missing
```

5. Generate the CMake files
```sh
cd build
```
```sh
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
```
windows users might need to run this instead
```sh
cmake .. -G "Visual Studio 15 2017" -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake"
```

## Run backend
After you generated your cmake directory run the following commands, replace 
`NAME_OF_YOUR_CMAKE_DIRECTORY` with the name of the directory where you generated 
your cmake files.
```sh
cmake --build NAME_OF_YOUR_CMAKE_DIRECTORY/. && NAME_OF_YOUR_CMAKE_DIRECTORY/People-Party-Backend
```

## Run tests
After you generated your cmake directory run CTest. Replace 
`NAME_OF_YOUR_CMAKE_DIRECTORY` with the name of your cmake directory.
```sh
ctest --test-dir NAME_OF_YOUR_CMAKE_DIRECTORY
```
