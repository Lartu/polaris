<p align="center">
  <img src="images/polaris_image.png">
  <br><br>
  <img src="https://img.shields.io/badge/version-1.1-blue.svg">
  <img src="https://img.shields.io/badge/apache-_2.0-yellow">
  <a href="https://www.freenode.net"><img src="https://img.shields.io/badge/irc-%23polarislang-navy"></a>
  <a href="https://t.me/polarislang"><img src="https://img.shields.io/badge/-polarislang-295887?logo=telegram"></a>
</p>

**Polaris** is a stack-based, concatenative, interpreted programming language with strings as its only data type.
It has been designed to be **small** in size and **minimal** in language features. **Polaris** was
conceived to run under **Unix** systems.
It has been written in C++98, so it can probably be compiled with any C++ compiler out there.

```c++
/* This is a little Polaris example */

("Tell me your name: " print input >name) >askName
("Hello there, " @name join "\n" join print) >sayHello

%askName %sayHello
```

# Building Polaris
To build Polaris, clone this repository and run `make`. You can install it using `make install`.

# Documentation
The Polaris documentation can be [here](https://www.lartu.net/languages/polaris/).

Some code examples can be found in the [Examples](examples) directory.

# License
The Polaris Interpreter is distributed under the Apache 2.0 License.
