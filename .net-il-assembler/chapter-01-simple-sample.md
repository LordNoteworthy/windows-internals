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

