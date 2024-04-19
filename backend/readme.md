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
Run conan install
- Linux & MacOS:
```sh
conan install . --output-folder=build --build=missing --settings=build_type=Debug
```
- Windows:
```sh
conan install . --output-folder=build --build=missing --settings=build_type=Debug -s compiler.cppstd=17
```

5. Generate the CMake files
```sh
cd build
```
```sh
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
```
windows users might need to run this instead
```sh
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake" -DCMAKE_BUILD_TYPE=Debug
```

## Run backend
After you generated your cmake directory run the following commands, replace 
with the name of the directory where you generated 
your cmake files.
- Linux & MacOS:
```sh
cmake --build build/. 
build/PeopleParty-Backend
```
- Windows:
```sh
cmake --build build/.
build/Debug/PeopleParty-Backend
```

## Run tests
- Linux & MacOS:
```sh
build/PeopleParty-Backend-Tests
```
- Windows:
```sh
build/Debug/PeopleParty-Backend-Tests
```
