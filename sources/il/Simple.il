//----------- Program header
.assembly extern  mscorlib { auto }
.assembly OddOrEven  { }
.module OddOrEven.exe
//----------- Class declaration
.namespace Odd.or {
    .class public auto ansi Even extends [mscorlib]System.Object {
//----------- Field declaration
        .field public static int32 val
//----------- Method declaration
        .method public static void check( ) cil managed {
            .entrypoint
            .locals init (int32 Retval)
        AskForNumber:
            ldstr "Enter a number"
            call void [mscorlib]System.Console::WriteLine (string)
            call string [mscorlib]System.Console::ReadLine ()
            ldsflda valuetype CharArray8 Format
            ldsflda int32 Odd.or.Even::val
            call vararg int32 sscanf(string,int8*,...,int32*)
            stloc Retval
            ldloc Retval
            brfalse Error
            ldsfld int32 Odd.or.Even::val
            ldc.i4 1
            and
            brfalse ItsEven
            ldstr "odd!"
            br PrintAndReturn
        ItsEven:
            ldstr "even!"
            br PrintAndReturn
        Error:
            ldstr "How rude!"
        PrintAndReturn:
            call void [mscorlib]System.Console::WriteLine (string)
            ldloc Retval
            brtrue AskForNumber
            ret
        } // End of method
    } // End of class
} // End of namespace
//----------- Global items
.field public static valuetype CharArray8 Format at FormatData
//----------- Data declaration
.data FormatData = bytearray(25 64 00 00 00 00 00 00) // % d . . . . . .
//----------- Value type as placeholder
.class public explicit CharArray8 
              extends [mscorlib]System.ValueType { .size 8  }
//----------- Calling unmanaged code
.method public static pinvokeimpl("msvcrt.dll" cdecl) 
    vararg int32 sscanf(string,int8*) cil managed preservesig { }
