# GNU based system software toolchain

![Project Image](project-image-url)

> This is a ReadMe template to help save you time and effort.
---

### Table of Contents
You're sections headers will be used to reference location of destination.

- [Description](#description)
- [How To Use](#how-to-use)
- [References](#references)
- [License](#license)
- [Author Info](#author-info)

---

## Description

This project is concerned with implementing a complete toolchain for translating an assembly (.s) file into executable machine code, modeled after the GNU toolchain tools.

#### Technologies

- Flex [Flex is utilized as part of the lexing process]
- Bison [Bison is employed in the parsing phase]
- The C++ Standard Template Library (STL) [It is used in conjunction with C++ to facilitate the implementation of data structures and operations performed on them]
- Make [Make is used to automate the building and compilation of project by specifying dependencies and the steps required to build the project]


[Back To The Top](#gnu-based-system-software-toolchain)

---

## How To Use

#### Installation
The tool chain was designed and implemented to be used on the Linux operating system, and therefore uses the make tool to link all files. (If you want to use this tool from the Windows operating system, you need to use a tool that will allow you to run <b>Make</b> inside the Windows operating system, such as <b>Cygwin</b>)

<br />

You can find more about the Make tool and its use, as well as the documentation on the website:
```html
    https://www.gnu.org/software/make/
```
In addition to the Make tool, <b>Flex</b> and <b>Bison</b> tools were also used in this project. Flex was used as a lexeme generator, while Bison was used for parsing. You can find more about these tools, as well as their documentation for use, at the following link:

```html
    https://github.com/westes/flex
```

```html
    https://www.gnu.org/software/bison/manual/
```


#### API Reference
Using the make tool we can run several commands:

<br />

```html
    make all (or just make)
```

Using the make command of the Make tool, the script will be started automatically for the file named <i><b>makefile</b></i> in the current directory, ie our makefile file. Its role is to start flex and bison and create a parser and lexer to which the .s assembler file we want to assemble will be passed later.

<br />

```html
    clean_assembly
```
clean_assembly command will only delete all auxiliary files that were created during assembly creation (auxiliary files used when creating lexer and parser)


[Back To The Top](#gnu-based-system-software-toolchain)

---

## References
Make:
```html
    https://www.gnu.org/software/make/
```
Flex:
```html
    https://github.com/westes/flex
```
Bison:
```html
    https://www.gnu.org/software/bison/manual/
```
GNU assembly documentation (used for inspiration)
```html
    https://sourceware.org/binutils/docs/as/
```

GNU linker documentation (used for inspiration)
```html
    https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_mono/ld.html
```



[Back To The Top](#gnu-based-system-software-toolchain)

---

## License

MIT License

Copyright (c) [2023] [zarkobabic]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

[Back To The Top](#gnu-based-system-software-toolchain)

---

## Author Info

Žarko Babić - Student at School of Electrical engineering at University of Belgrade

[Back To The Top](#gnu-based-system-software-toolchain)
Footer
© 2023 GitHub, Inc.
Footer navigation
Terms
Privacy
Security
Statu
