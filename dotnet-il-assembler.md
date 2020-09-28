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

### Simple Sample: The Code

- see the source file `Simple.il` on the `sources/il/` folder.

#### Program Header

- this is the program header of the OddOrEven application:
```v
.assembly extern mscorlib { auto } 
.assembly OddOrEven { }
.module OddOrEven.exe
```
- `.assembly extern mscorlib { auto }`:
    - defines a metadata item named __Assembly Reference__ (or __AssemblyRef__), identifying the external managed application (assembly) used in this program.
    - in this case, the external application is __Mscorlib.dll__, the main assembly of the .NET Framework classes. It assembly contains declarations of all the __base classes__ from which all other classes are __derived__.
- `.assembly OddOrEven { }`: defines a metadata item named __Assembly__, which, to no one’s surprise, identifies the current application (assembly).
- `.module OddOrEven.exe` defines a metadata item named __Module__, identifying the current module. Each module, prime or otherwise, carries this identification in its metadata.

#### Class Declaration

- this is the class declaration of the OddOrEven application:

```v
.namespace Odd.or {
    .class public auto ansi Even extends [mscorlib]System.Object {
    ...
    } // End of class
} // End of namespace
```
- `namespace Odd.or { ... }` declares a __namespace__. A namespace does not represent a separate metadata item. Rather, a namespace is a __common prefix__ of the full names of all the classes declared within the scope of the namespace declaration.
- `.class public auto ansi Even extends [mscorlib]System.Object { ... }`:
    - defines a metadata item named __Type Definition__ (or __TypeDef__).
    - each class, structure, or enumeration defined in the current module is described by a respective __TypeDef__ record in the metadata.
    - the name of the class is __Even__.
    - the keywords __public__, __auto__, and __ansi__ define the __flags__ of the TypeDef item. 
    - the keyword __public__, which defines the visibility of the class, means the class is visible outside the current assembly (opposite of __private__.) 
    - the keyword __auto__ in this context defines the class layout style (automatic, the default), directing the loader to lay out this class however it sees fit. Alternatives are __sequential__ (which preserves the specified sequence of the fields) and __explicit__ (which explicitly specifies the offset for each field, giving the loader exact instructions for laying out the class).
    - the keyword __ansi__ defines the mode of string conversion within the class when interoperating with the unmanaged code. Alternative keywords are __unicode__.
- the clause extends `[mscorlib]System.Object` defines the parent, or base class, of the class `Odd.or.Even`.
    - the code `[mscorlib]System.Object` represents a metadata item named __Type Reference__ (or __TypeRef__). This particular TypeRef has System as its namespace, Object as its name, and AssemblyRef mscorlib as the resolution scope. 
    - each class defined __outside__ the current module is addressed by TypeRef. 
- in ILAsm, you can declare a TypeDef with some of its attributes and members, close the TypeDef’s scope, and then reopen the same TypeDef later in the source code to declare more of its attributes and members. This technique is referred to as __class amendment__.

#### Field Declaration

- `.field public static int32 val` defines a metadata item named __Field Definition__ (or __FieldDef__). The keywords __public__ and __static__ define the flags of the FieldDef. 
    - the keyword public identifies the accessibility of this field and means the field can be accessed by any member for whom this __class is visible__.
- __assembly__ flag specifies that the field can be accessed from anywhere within this __assembly__ but not from outside.
- __family__ flag specifies that the field can be accessed from any of the classes descending from `Odd.or.Even`.
- __famandassem__ flag specifies that the field can be accessed from any of those descendants of `Odd.or.Even` that are defined in this assembly. 
- __famorassem__ flag specifies that the field can be accessed from anywhere within this assembly as well as from any descendant of `Odd.or.Even`, even if the descendant is declared __outside__ this assembly.
- __private__ flag specifies that the field can be accessed from `Odd.or.Even only`.
- __privatescope__ flag specifies that the field can be accessed from anywhere within the current module. This flag is the __default__.
    - the privatescope flag is a special case, and strongly to not use it.
    - private scope items are exempt from the requirement of having a unique parent/name/signature triad, which means you can define two or more private scope items within the same class that have the same name and the same type. Some compilers emit private scope items for their internal purposes. It is the compiler’s problem to distinguish one private scope item from another; if you decide to use private scope items, you should at least give them unique names. Because the default accessibility is privatescope, which can be a problem, it’s important to remember to specify the accessibility flags.
- the keyword __static__ means the field is static; that is, it is __shared by all instances__ of class `Odd.or.Even`. 
    - if you did not designate the field as static, it would be an __instance field__, individual to a specific instance of the class.

#### Method Declaration

