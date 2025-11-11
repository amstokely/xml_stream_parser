
### XML Stream Parser 

This project provides a modern C++20 implementation of the MPAS XML stream parser.
By default, it uses **pugixml** for XML parsing, but the parser is designed with **dependency injection**, allowing alternative XML backends to be used by implementing a small adapter interface.

The build system is **CMake**, and the test suite is implemented using **Boost.UT**.

---
