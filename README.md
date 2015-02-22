# uraster

Micro simple software rasterizer in a single C++11 header file.  Does not use OpenGL.  Pure C++11.   Mostly useful as a way of demonstrating the rendering pipeline

![Image from uraster](https://raw.githubusercontent.com/Steve132/uraster/master/example/screenshot.jpg)

A day ago, I saw this post [http://www.reddit.com/r/GraphicsProgramming/comments/2whjam/creating_a_software_renderer/](Asking how to create a software renderer).

I started writing my response, and it looked something like this:

> One should not create a software rasterizer unless they are giant masochists.  However, if you were to do it, and first you need to ....
    
After a while I stopped.  After writing my tutorial on how to make a C++ rasterizer in english, I realized it was nearly unreadable.

New idea:  I'll just write the tutorial in pseudocode.  Then, I looked at the pseudocode.  It was unreadable because it wasn't *quite* a language

New idea: I'll write the skeleton of the header with comments explaining what to put in the code!  I did this.

Surely you can see where this is going.   

I ended up actually filling in the code, and implementing a simple rasterization pipeline all in one C++11 header standalone header file.  The interface is similar to a dramatically simplified version of OpenGL2.0 with shader support, that is only capable of drawing interleaved arrays of vertices from index buffers.

There are a couple examples showing how you use it, and I'll add comments or a tutorial to explain the code at a later date.

##Dependencies##

It needs Eigen3 for the main header, cmake to build the examples, and CImg (to display) the animation and texturing example.
