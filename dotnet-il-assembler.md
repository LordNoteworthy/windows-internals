# Chapter 1 - simple sample

- ILAsm: __IL assembly language__.
- Don’t confuse it with ILASM, which is the abbreviation for the __IL assembler__.

## Basics of the Common Language Runtime

- the __common language runtime__ is a run-time environment in which .NET applications run. 
- the set of rules guaranteeing the __interoperability__ of .NET applications is known as the __Common Language Specification (CLS)__.
    - it limits the naming conventions, the data types, the function types, and certain other elements, forming a common denominator for different languages. 
    - it is important to remember, however, that the CLS is merely a __recommendation__ and has no bearing whatsoever on common language runtime functionality. i
    - if your application is not __CLS compliant__, it might be __valid__ in terms of the common language runtime, but you have no guarantee that it will be able to __interoperate__ with other applications on all levels.
- the abstract intermediate representation of the .NET applications, intended for the common language runtime environment, includes two main components:
    - __metadata__ is a system of descriptors of all structural items of the application—classes, their members and attributes, global items, and so on—and their relationships.
    - __managed code__ represents the functionality of the application’s methods (functions) encoded in an abstract binary form known as __Microsoft intermediate language (MSIL)__ or __common intermediate language (CIL)__. To make things easy, we can refer to it as simply __intermediate language (IL)__.
- CLR management includes, but is not limited to, three major activities:
    - __Type control__ involves the verification and conversion of item types during execution.
    - __Managed exception handling__ is functionally similar to “unmanaged” structured exception handling, but it is performed by the runtime rather than by the operating system. 
    - __Garbage collection__ involves the automatic identification and disposal of objects no longer in use.
- a .NET application, consists of __one or more managed executables__, each of which carries __metadata__ and (optionally) __managed code__.
- managed code is optional because it is always possible to build a managed executable containing no methods.
- managed .NET applications are called __assemblies__.
- the managed executables are referred to as __modules__. You can create single-module assemblies and multimodule assemblies. 
- as illustrated below, each assembly contains one __prime module__, which carries the assembly __identity information in its metadata__.
<p align="center"><img src="https://i.imgur.com/DMoUMCZ.png" width="400px" height="auto"></p>

- the two principal components of a managed executable are the __metadata__ and the __IL code__.
- the two major CLR subsystems dealing with each component are, respectively, the __loader__ and the __just-in-time (JIT)__ compiler.
- the loader reads the metadata and creates in memory an __internal representation and layout of the classes and their members__. It performs this task on demand, meaning a class is loaded and laid out only when it is referenced. Classes that are never referenced are never loaded. When loading a class, the loader runs a series of consistency checks of the related metadata. 
- the JIT compiler, relying on the results of the loader’s activity, __compiles the methods encoded in IL into the native code__ of the underlying platform. Because the runtime is not an interpreter, it does not execute the IL code. Instead, the IL code is compiled in memory into the native code, and the native code is executed. The JIT compilation is also done on demand, meaning a method is compiled only when it is called. The compiled methods stay cached in memory.
<p align="center"><img src="https://i.imgur.com/u513Av8.png" width="400px" height="auto"></p>

- You can precompile a managed executable from IL to the native code using the __NGEN utility__. 