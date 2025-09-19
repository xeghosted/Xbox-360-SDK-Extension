#CWare Library for Xbox 360
A comprehensive utility library for Xbox 360 homebrew development, providing modern C++ features and utilities tailored for the Xbox 360 environment.

#Features

JSON Parser/Serializer: Full JSON support with parsing and serialization capabilities

STL Extensions: Implementation of C++11 features missing from Xbox 360 SDK:

std::unique_ptr, std::make_unique

std::mutex, std::condition_variable, std::lock_guard

std::future, std::promise, std::async

Atomic operations and memory ordering

Thread Pool: Efficient thread management for concurrent operations

Chrono Library: High-resolution timing and duration utilities

Filesystem Utilities: Cross-platform filesystem operations

Network Library: Async TCP client with Xbox-specific networking

String Utilities: Comprehensive string manipulation functions

Math Library: 2D/3D vector and matrix operations

Memory Utilities: Safe memory operations and casting functions

Xbox-Specific Helpers: XAM function resolution and system utilities

#Installation

copy cware.h into C:\Program Files (x86)\Microsoft Xbox 360 SDK\include\xbox
the include it into your project with #include <cware.xex>
