# Notes from .NET IL Assembler by Serge Lidin 

These notes are directly taken from the book mentionned above. They represent to me a summary of the __MOST important__ things I wanted to remember while I was reading this book. When time passes, I always need a quick reference to refresh my memory of what I learned, especially when I didn't have a chance to put into practise the learned knowledge or when my day to day job does not use any of it.

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
<p align="center"><img src="https://i.imgur.com/DMoUMCZ.png" width="500px" height="auto"></p>

- the two principal components of a managed executable are the __metadata__ and the __IL code__.
- the two major CLR subsystems dealing with each component are, respectively, the __loader__ and the __just-in-time (JIT)__ compiler.
- the loader reads the metadata and creates in memory an __internal representation and layout of the classes and their members__. It performs this task on demand, meaning a class is loaded and laid out only when it is referenced. Classes that are never referenced are never loaded. When loading a class, the loader runs a series of consistency checks of the related metadata. 
- the JIT compiler, relying on the results of the loader’s activity, __compiles the methods encoded in IL into the native code__ of the underlying platform. Because the runtime is not an interpreter, it does not execute the IL code. Instead, the IL code is compiled in memory into the native code, and the native code is executed. The JIT compilation is also done on demand, meaning a method is compiled only when it is called. The compiled methods stay cached in memory.
<p align="center"><img src="https://i.imgur.com/u513Av8.png" width="500px" height="auto"></p>

- You can precompile a managed executable from IL to the native code using the __NGEN utility__. 

## Simple Sample: The Code

- see the source file `Simple.il` on the `sources/il/` folder.

### Program Header

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

### Class Declaration

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

### Field Declaration

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

### Method Declaration

```v
.method public static void check( ) cil managed {
    .entrypoint
    .locals init (int32 Retval)
...
}
```
- the first line defines a metadata item named __Method Definition__
(or __MethodDef__).
- keywords public and static define the flags of MethodDef and mean the same as the similarly named flags of __FieldDef__.
- `void` defines the return type of the method.
- the keywords `cil` and `managed` define so-called __implementation flags__ of the MethodDef and indicate that the method body is represented in IL.
    -  a method represented in native code rather than in IL would carry the implementation flags native __unmanaged__.
- `.entrypoint` identifies the current method as the entry point of the application (the assembly).
- `.locals init (int32 Retval)` defines the single local variable of the current method.
    - `init` means the local variables will be zero-initialized at runtime.

```v
AskForNumber:
    ldstr "Enter a number"
    call void [mscorlib]System.Console::WriteLine(string)
```

- `AskForNumber`is a label; the IL disassembler marks every instruction with a label on the same line as the instruction.
- labels are not compiled into metadata or IL; rather, they are used solely for the identification of certain offsets within IL code at compile time.
- an important note: __IL is strictly a stack-based language__. Every instruction takes something (or nothing) from the top of the stack and puts something (or nothing) onto the stack.
- No IL instruction can address a local variable or a method parameter directly, except the instructions of __load__ and __store__ groups, which, respectively, put the value or the address of a variable or a parameter onto the stack or take the value from the stack and put it into a variable or a parameter.
- elements of the IL stack are not bytes or words, but __slots__.
- `ldstr "Enter a number"` is an instruction that creates a string object from the specified string constant and loads a reference to this object onto the stack. The string constant in this case is stored in the metadata.
- `call void [mscorlib]System.Console::WriteLine(string)` is an instruction that calls a console output method from the .NET Framework class library. The string is taken from the stack as the method argument, and nothing is put back, because the method returns void.
- the parameter of this instruction is a metadata item named __Member Reference__ (or __MemberRef__). It refers to the static method named `WriteLine`, which has the signature `void(string)`; the method is a member of class System.
- the MemberRefs are members of TypeRefs, just as FieldDefs and MethodDefs are TypeDef members. However, there are __no separate__ FieldRefs and MethodRefs; the MemberRefs cover references to both fields and methods.

```v
call string [mscorlib]System.Console::ReadLine()
ldsflda valuetype CharArray8 Format
ldsflda int32 Odd.or.Even::val
call vararg int32 sscanf(string,int8*,...,int32*)
```
- `call string [mscorlib]System.Console::ReadLine()` calls a console input method from the .NET Framework class library. Nothing is taken from the stack, and a string is put onto the stack as a result of this call.
- `ldsflda valuetype CharArray8 Format` __loads the address of the static field__ Format of type valuetype CharArray8.
    - IL has separate instructions for loading instance and static fields (`ldfld` and `ldsfld`) or their addresses (`ldflda` and `ldsflda`). Also note that the “address” loaded onto the stack is not exactly an address (or a C/C++ pointer) but rather a reference to the item (a field in this sample).
- `ldsflda int32 Odd.or.Even::val` loads the address of the static field val, which is a
member of the class `Odd.or.Even`, of type int32.
- in IL, all references must be __fully qualified__.
- `call vararg int32 sscanf(string,int8*,...,int32*)` calls the global static method `sscanf`. This method takes three items currently on the stack (the string returned from `System.Console::ReadLine`, the reference to the global field Format, and the reference to the field Odd.or.Even::val) and puts the result of type int32 onto the stack.

```v
stloc Retval
ldloc Retval
brfalse Error
```

- `stloc Retval` takes the result of the call to sscanf from the stack and stores it in the local variable Retval.
- you need to save this value in a local variable because you will need it later. `ldloc Retval` copies the value of Retval back onto the stack. You need to check this value, which was taken off the stack by the stloc instruction.
- `brfalse Error` takes an item from the stack, and if it is 0, it branches (switches the computation flow) to the
label Error.

```v
ldsfld int32 Odd.or.Even::val
ldc.i4 1
and
brfalse ItsEven
ldstr "odd!"
br PrintAndReturn
```

- `ldsfld int32 Odd.or.Even::val` loads the value of the static field Odd.or.Even::val onto the stack.
- `ldc.i4 1` is an instruction that loads the constant 1 of type int32 onto the stack.
- `and` takes two items from the stack (the value of the field val and the integer constant 1), performs a bitwise AND operation, and puts the result onto the stack.
- `brfalse ItsEven` takes an item from the stack (the result of the bitwise AND operation), and if it is 0, it branches to the label ItsEven.
- `ldstr "odd!"` is an instruction that loads the string odd! onto the stack.
- `br PrintAndReturn` is an instruction that does not touch the stack and branches unconditionally to the label `PrintAndReturn`.
- `ret` which is fairly obvious: it returns whatever is on the stack.

### Global Items

- these are the global items of the _OddOrEven_ application:
```v
{
    ...
} // End of namespace
.field public static valuetype CharArray8 Format at FormatData
```
- `.field public static valuetype CharArray8 Format at FormatData` declares a static field named `Format` of type _valuetype CharArray8_. As you might remember, you used a reference to this field in the method `Odd.or.Even::check`.
- this field differs from, for example, the field `Odd.or.Even::val` because it is declared outside any class scope and hence does not belong to any class. It is thus a __global item__.
- the metadata of every module contains one special TypeDef named `<Module>` that contains all global items not belonging to any class.
- all the classes declared __within a module__ have full access to the global items of this module, including the __private ones__ !

### Mapped Fields

- this is the mapped field of the _OddOrEven_ application:
```v
.field public static valuetype CharArray8 Format at FormatData
```
- the declaration of the field Format contains one more new item, the clause `at FormatData`.
- this clause indicates the `Format` field is located in the __data section__ of the module and its location is identified by the data label `FormatData`.
- compilers widely use this technique of mapping fields to data for __field initialization__.

### Data Declaration

- this is the data declaration of the _OddOrEven_ application:
```v
.data FormatData = bytearray(25 64 00 00 00 00 00 00)
```
- it defines a data segment labeled `FormatData`. This segment is 8 bytes long and has ASCII codes of the characters % (0x25) and d (0x64) in the first 2 bytes and zeros
in the remaining 6 bytes.
- the segment is described as `bytearray`, which is the most ubiquitous way to describe data in ILAsm. The numbers within the parentheses represent the hexadecimal values of the bytes, without the 0x prefix. The byte values should be separated by spaces.

### Value Type As Placeholder

```v
.class public explicit CharArray8   
    extends [mscorlib]System.ValueType { .size 8 }
```

- it declares a __value type__ that has no members but has an explicitly specified size, 8 bytes. Declaring such a value type is a common way to declare "just a piece of memory".

### Calling Unmanaged Code

```v
.method public static pinvokeimpl("msvcrt.dll" cdecl)
    vararg int32 sscanf(string,int8*) cil managed preservesig { }
```
- it declares an __unmanaged method__, to be called from __managed code__. 
- the attribute `pinvokeimpl("msvcrt.dll" cdecl)` indicates that this is an unmanaged method, called using the mechanism known as __platform invocation or P/Invoke__. This attribute also indicates that this method resides in the unmanaged DLL `Msvcrt.dll` and has the calling convention `cdecl`.
- __platform invocation__ is the mechanism the common language runtime provides to facilitate the calls from the managed code to unmanaged functions. Behind the scenes, the runtime constructs the so-called _stub_, or _thunk_, which allows the addressing of the unmanaged function and conversion of managed argument types to the appropriate unmanaged types and back.
- the implementation flag __preservesig__ indicates that the return of the method is to be preserved. In CLR versions 1.0, 1.1 and 2.0, this implementation flag was not needed in case of a P/Invoke method, because only COM methods underwent signature change. However, in version 4.0 the signature change was for some reason extended to P/Invoke methods as well.

# Chapter 4: The Structure of a Managed Executable File

- the file format of a managed module is an extension of the standard Microsoft Windows Portable Executable and Common Object File Format (__PE/COFF__). Thus, formally, any managed module is a proper PE/COFF file, with additional features that identify it as a managed executable file.
<p align="center"><img src="https://i.imgur.com/SfPRKBq.png" width="350px" height="auto"></p>

## PE/COFF Headers

the IL assembler generates the following sections in a PE file:
- `.text`: A read-only section containing the common language runtime header, the metadata, the IL code, managed exception handling information, and managed resources
- `.sdata`: A read/write section containing data
- `.reloc`: A read-only section containing relocations
- `.rsrc`: A read-only section containing unmanaged resources
- `.tls`: A read/write section containing thread local storage data.

## Common Language Runtime Header

- skipped the COM header and flags definitions.

### EntryPointToken

-  contains a token (metadata identifier) of either a method definition (MethodDef) or a file reference (File).
- a __MethodDef token__ identifies a method defined in the module (a managed PE file) as the entry point method.
- a __File token__ is used in one case only: in the runtime header of the prime module of a multimodule assembly, when the entry point method is defined in another module (identified by the file reference) of this assembly. In this case, the module identified by the file reference must contain the respective MethodDef token in the EntryPointToken field of its runtime header.
- The method referred to by the EntryPointToken/EntryPointRVA field of the common language runtime header has nothing to do with the function to which the _AddressOfEntryPoint_ field of the PE header points. AddressOfEntryPoint always points to the __runtime invocation stub__, which is invisible to the runtime, is not reflected
in metadata and hence cannot have a token.

### VTableFixups Field

- is a data directory containing the RVA and the size of the image file’s v-table fixup table.
- managed and unmanaged methods use different data formats, so when a __managed method must be called from unmanaged code__, the common language runtime creates a marshaling thunk for it, which performs the data conversions, and the address of this thunk is placed in the respective address table. 
- if the managed method is called from the unmanaged code embedded in the current managed PE file, the thunk address goes to the file’s v-table.

### StrongNameSignature Field

- contains the RVA and size of the __strong name hash__, which is used by the runtime to establish the authenticity of the image file.
- after the image file has been created, it is hashed using the private encryption keys provided by the producer of the image file, and the resulting hash blob is written into the space allocated inside the image file.
- if even a single byte in the image file is subsequently modified, the authenticity check fails, and the image file cannot be loaded.

### Relocation Section

- the only fixup type emitted by the existing managed compilers in 32-bit executables is `IMAGE_REL_BASED_HIGHLOW`. In 64-bit executables, it is `IMAGE_REL_BASED_DIR64`.
- A 32-bit pure-IL PE file, as a rule, contains only __one fixup__ in the .reloc section. This is for the benefit of the common language runtime __start-up stub__, the only segment of native code in a pure-IL image file. This fixup is for the image file’s IAT, containing a single entry: the __CLR entry point__.
- A 64-bit pure-IL PE file contains one fixup on X64 architecture and two fixups on Itanium architecture (additional fixup needed for the global pointer).
- Windows XP or newer, as a common language runtime–aware operating system, needs neither the runtime start-up stub nor the IAT to invoke the runtime. Thus, if the common language runtime header flags indicate that the image file is IL only (`COMIMAGE_FLAGS_ILONLY`), the operating system ignores the `.reloc` section altogether.

### Text Section

- structure of a `.text` section emitted by the IL assembler:
<p align="center"><img src="https://i.imgur.com/N5ko4RG.png" width="350px" height="auto"></p>

### Data Sections

- data section (.sdata) of an image file generated by the IL assembler is a read/write section. It contains __data constants__, the __v-table__ described in the “V-Table” section, the __unmanaged export table__, and the __thread local storage__ directory structure. The data declared as thread-specific is located in a different section, the `.tls` section.

### Data Constants

- the term __data constants__ might be a little __misleading__. Located in a read/write section, data constants can certainly be __overwritten__, so technically they can hardly be called constants.
- data constants represent the __mappings of the static fields__ and usually contain data initializing the mapped fields.
- field mapping is a convenient way to initialize any static field with ANSI strings, blobs, or structures. An alternative way to initialize static fields—and a more orthodox way in terms of the common language runtime—is to do it explicitly by executing code in __class constructors__.

### V-Table

- the v-table in a pure-managed module is used for __exposing the managed methods for consumption from the unmanaged code__ and consists of entries, each entry consisting of one or more slots.
- each slot of the v-table contains a __metadata token__ of the respective method, which at execution time is replaced with the address of the method itself or the address of a marshaling thunk providing unmanaged entry to the method.

### Unmanaged Export Table

- the unmanaged export table in an unmanaged image file occupies a separate section named `.edata`. In image files generated by the IL assembler, the unmanaged export table resides in the `.sdata` section, together with the v-table it references.
- in a managed file, the Export Address table contains the RVAs not of the exported entry points (methods) themselves but rather of __unmanaged export stubs__ giving access to these entry points. Export stubs, in turn, contain references to respective v-table slots.
- The IL assembler __does not allow forward exports__, so the entries in an Export Address table of an image file generated by this compiler always represent the RVAs of unmanaged export stubs.
- IL assembler does not allow unnamed exports.

### Thread Local Storage

- ILAsm and VC++ allow you to define data constants belonging to thread local storage and to map static fields to these data constants.

### Resources

- you can embed two distinct kinds of resources in a managed PE file: __unmanaged platform-specific__ resources and __managed resources specific to CLR__.
- these two kinds of resources, which have nothing in common, reside in different
sections of a managed image file and are accessed by different sets of APIs.

### Unmanaged Resources

- unmanaged resources (reside in `.rsrc` section) are indexed by __type, name, and language__ and are binary sorted by these three characteristics in that order.
- a set of Resource directory tables represents this indexing as follows: each directory table is followed by an array of directory entries, which contain the integer reference number (ID) or name of the respective level (the type, name, or language level) and the address of the next-level directory table or of a data description (a leaf node of the tree).

### Managed Resources

- the Resources field of the CLR header contains the RVA and size of the managed resources embedded in the PE file. It has nothing to do with the Resource directory of the PE header, which specifies the RVA and size of unmanaged platform-specific resources.
- managed resources are stored in the `.text` section contiguously.
- metadata carries __ManifestResource__ records, one for each managed resource, containing the name of the managed resource and the offset of the beginning of the resource from the starting RVA specified in the Resources field of the CLR header.

## Summary

- steps the IL assembler takes to create a managed PE file:

- __Phase 1: Initialization__
    - Internal buffers are initialized.
    - The empty template of a PE file is created in memory, including an MS-DOS header and stub, a PE signature, a COFF header, and a PE header.
    - The Import Address table and the CLR header are allocated in the .text section.
- __Phase 2: Source Code Parsing__
    - Metadata is collected in internal buffers.
    - The method bodies (IL code and managed exception handling tables) are collected in
internal buffers.
    - Data constants are emitted to the .sdata and .tls sections. 
- __Phase 3: Image Generation__
    - Space for the strong name signature is allocated in the .text section.
    - Metadata is analyzed and rearranged.
    - Internal (to the module) references are resolved in the IL code.
    - Method bodies are emitted to the .text section.
    - The TLS directory table is emitted to the .sdata section.
    - The debug directory is emitted to the .text section.
    - Space for metadata is allocated in the .text section.
    - Space for embedded managed resources is allocated in the .text section.
    - Unmanaged export stubs are emitted to the .text section.
    - The VTFixup table is emitted to the .text section.
    - The v-table is emitted to the .sdata section.
    - Unmanaged export tables are emitted to the .sdata section.
    - Last changes in the metadata—the RVAs of mapped fields are fixed up.
    - Metadata is emitted into the preallocated space in the .text section.
    - Managed resources are emitted into the preallocated space in the .text section.
    - The runtime start-up stub is emitted to the .text section.
    - Unmanaged resources are read from the .res file and emitted to the .rsrc section.
    - Necessary base relocations are emitted to the .reloc section.
- __Phase 4: Completion__
    - The image file is written as a disk file.
    - The strong name signing procedure is applied to the file by invoking the strong name utility (sn.exe).

# Metadata Tables Organization

## What Is Metadata?

- in the context of the CLR, metadata means a __system of descriptors of all items__ that are declared or referenced in a module. 
- the CLR programming model is inherently object oriented, so the items represented in metadata are __classes and their members__, with their accompanying __attributes, properties, and relationships__.
- metadata is an integral part of a managed module, which means each managed module always carries a complete, high-level, formal description of its logical structure.
- structurally, metadata is a __normalized relational database__. This means that metadata is organized as a set of __cross-referencing__ rectangular tables—as opposed to, for example, a hierarchical database that has a tree structure.
- each column of a metadata table contains either data or a reference to a row of another table. Metadata does not contain any duplicate data fields; each category of data resides in only one table of the metadata database. If another table needs to employ the same data, it references the table that holds the data.
- an example of optimized metadata:<p align="center"><img src="https://i.imgur.com/HAGIZWS.png" width="400px" height="auto"></p>
- it is possible, however (perhaps as a result of sloppy metadata emission or of incremental compilation), to have the child tables interleaved with regard to their owner classes. In such a case, additional intermediate metadata tables are engaged, providing noninterleaved lookup tables sorted by the owner class. Instead of referencing the method records, class records reference the records of an __intermediate table (a pointer table)__, and those records in turn reference the method records, as diagrammed below. Metadata that uses such intermediate lookup tables is referred to as __unoptimized or uncompressed__: <p align="center"><img src="https://i.imgur.com/HSV8hjI.png" width="400px" height="auto"></p>

## Heaps and Tables

- logically, metadata is represented as a set of __named streams++, with each stream representing a category of metadata.
- rhese streams are divided into two types: __metadata heaps__ and __metadata tables__.

### Heaps

- metadata heap is a storage of trivial structure, holding a __contiguous sequence of items__.
- heaps are used in metadata to store __strings and binary objects__.
- there are three kinds of metadata heaps:
    - __String heap__: This kind of heap contains __zero-terminated character strings__, encoded in UTF-8.
    - __GUID heap__: This kind of heap contains __16-byte binary objects__, immediately following each other. The size of the binary objects is fixed.
    - __Blob heap__: This kind of heap contains binary objects of __arbitrary__ size.

### General Metadata Header

- the general metadata header consists of a __storage signature__ and a __storage header__.
- six named streams can be present in the metadata:
    - __#Strings__: A string heap containing the names of metadata items (class names, method names, field names, and so on). The stream does not contain literal constants defined or referenced in the methods of the module.
    - __#Blob__: A blob heap containing internal metadata binary objects, such as default values, signatures, and so on.
    - __#GUID__: A GUID heap containing all sorts of globally unique identifiers.
    -  __#US__: A blob heap containing user-defined strings. This stream contains string constants defined in the user code.
    - __#~__: A compressed (optimized) metadata stream. This stream contains an optimized system of metadata tables.
    - __#-__: An uncompressed (unoptimized) metadata stream. This stream contains an unoptimized system of metadata tables, which includes at least one intermediate lookup table (pointer table).
- the figure on the left side illustrates the general structure of metadata, and in the one in the right side, you can see the way streams are referenced by other streams as well as by external “consumers” such as metadata APIs and the IL code.
https://i.imgur.com/.png <p align="center"><img src="https://i.imgur.com/fHmklf8.png" width="700px" height="auto"></p>
- 