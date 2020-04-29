<p align="center">
  <img src="images/polaris_image.png">
  <br><br>
  <img src="https://img.shields.io/badge/version-1.0-blue.svg">
  <img src="https://img.shields.io/badge/apache-_2.0-yellow">
  <a href="https://www.freenode.net"><img src="https://img.shields.io/badge/irc-%23polarislang-navy"></a>
  <a href="https://t.me/polarislang"><img src="https://img.shields.io/badge/-polarislang-295887?logo=telegram"></a>
</p>

# About Polaris
**Polaris** is a stack-based, interpreted programming language with strings as its only data type.
It has been designed to be **small** in size and **minimal** in language features. **Polaris** was
conceived to run under **Unix** systems, either as a script running system or in REPL mode.
It has been written in C++98, so it can probably be compiled with any C++ compiler out there.

# Building Polaris

To build Polaris, clone this repository and run `c++ polaris.cpp -std=c++98 -pedantic -o polaris -l sqlite3`.

libsqlite3-dev is required for building Polaris. If you're using a Debian-based Linux (like Ubuntu or Mint), you can
install it with: `sudo apt-get install libsqlite3-dev`.


**Rest of the readme to be completed!**
