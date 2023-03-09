# What is this about?
As part of a 1.5-year long Computer Graphics course that I took [here](https://astromedicomp.org/), I had to
create a solo demo in C using OpenGL 1.0 (Yes! Immediate Mode OpenGL!) and a data
structure of my choice before moving ahead with the more advanced Core Profile OpenGL.

In the sketches I created, I needed to render a tree which could grow and interact with
the wind. I found from Wikipedia that the scientific way to generate trees in CG is through
the use of Lindenmayer Systems, or L-Systems for short.

After reading first few chapters of Lindenmayer's own book, "The Algorithmic Beauty of Plants",
I decided to implement a simple L-System API for use in my project.

Although I later decided not to use the grow/shrink feature in my project, these short programs
demonstrate that the API is indeed capable of allowing this to happen.

This repository contains the source code for a 'cool' experiment I wrote while working on the API
and my project.

# What is this NOT about?
This may quite possibly be the "wrong way" of writing C APIs!

Heck! Instead, I am learning what is right and what is not! What you see here is purely
experimental code, and I have taken no efforts to optimize anything. My only motive at
the time was to write something that runs and is logical enough for me to use.

Also, at least for now, the API supports only Deterministic Context-Free (D0L) L-Systems.

## Building with the supplied `.bat` files
### Pre-requisites:
a. Any Windows NT (3.1 and above) Operating System.
b. You must have Visual Studio Command Line Utilities installed.

1. Open an instance of the `x64 Native Tools Command Prompt`.

2. Navigate to the `d0l-fractal-trees` folder using `cd`.

3. Run the `.bat` file corresponding to the app you want to build,
   for example, run `Build2DTree.bat` to build `2DTree.exe`.

4. You will find the generated executable file at `d0l-fractal-trees/bin`.

# Controls

`L` 		: increase `iterations` <br />
`Shift + L` : decrease `iterations` <br />
<br />
`D`		: increase `delta` <br />
`Shift + D`	: decrease `delta` <br />
<br />
(Only for 3DTree) <br />
`R` 		: toggle `rotation` <br />