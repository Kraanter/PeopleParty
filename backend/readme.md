# People Party Backend

This is the c++ project of the People Party backend! This project uses CMake,
CTest and doctest. Tests are written using the doctest testing framework and
they should be ran using CTest (but you can also just run the test executable).
All dependencies should be automatically imported by the CMake so it should be 
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

2. Install CMake
Find the correct distribution on [this page](https://cmake.org/download/) and 
install it. When using some fancy IDE like CLion it will already be included in 
there.

3. Generate CMake
Generate the cmake files in any way you like. You can use this command
```sh
mkdir out && cd out && cmake build ..
```
or any other way you see fit.

4. Running the backend
First build the binaries
```sh
cmake --build .
```
then run the backend like this
```sh
./People-Party-Backend
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
