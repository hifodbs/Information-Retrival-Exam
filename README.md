# Information-Retrival-Exam

## Requisites
Need cmake for build the project

## Instruction

Run in this order this command from the root of the project.
The first two point for building the project the third one is for running.
- ```cmake -S . -B build```
- ```cmake --build build```
- ```build/IR```
The application it must run from the root folder, because of relative paths.

## Possible arguments
It possible to pass one argument 
- std Standard iterative system
- red Reduced iterative system

INFO: if no arugments are passed the system will execute the comparison test

## Documents
All the function are documented and most of them are documented following the Doxygen documentation.

## Notes
I used Clion for develop the project as IDE.
Ignore the the makefile.
