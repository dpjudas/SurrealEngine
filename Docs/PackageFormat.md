# Unreal Engine 1 Package Format
## Data types

All types are little endian.

- `byte` unsigned 8-bit integer
- `uint16` unsigned 16-bit integer
- `uint32` unsigned 32-bit integer
- `uint64` unsigned 64-bit integer
- `index` 32-bit integer encoded as described below
- `name` 8-bit text string encoded as described below

### Index Data Type
A compressed 32 bit integer. Uses the two high bits of the first byte to indicate if its a negative number and if there's more data to read. Then each following byte the high bit is used to indicate additional bits. The following C++ code illustrates how to read it:
```c++
int32_t ReadIndex()
{
	uint8_t value = ReadByte();
	bool signbit = value & (1 << 7);
	bool nextbyte = value & (1 << 6);
	int32_t index = value & 0x3f;
	if (nextbyte)
	{
		int shift = 6;
		do
		{
			value = ReadByte();
			index |= static_cast<int32_t>(value & 0x7f) << shift;
			shift += 7;
		} while ((value & (1 << 7)) && shift < 32);
	}
	if (signbit)
		index = -index;
	return index;
}
```

### Name Data Type
In packages older than 64 a name string was a zero terminated list types. In newer versions its a `index size` followed by the string. The following C++ code illustrates how to read it:
```c++
std::string ReadString()
{
	if (FileHeader.PackageVersion >= 64)
	{
		int len = ReadIndex();
		std::vector<char> s = ReadBytes(len);
		s.push_back(0);
		return s.data();
	}
	else
	{
		std::string s;
		while (true)
		{
			char c = ReadByte();
			if (c == 0) break;
			s.push_back(c);
		}
		return s;
	}
}
```

## File Header
```
struct FileHeader
{
  uint32 Signature = 0x9E2A83C1
  uint16 PackageVersion
  uint16 LicenseeMode
  uint32 PackageFlags
  uint32 NameCount
  uint32 NameOffset // Offset to name table
  uint32 ExportCount
  uint32 ExportOffset // offset to export table
  uint32 ImportCount
  uint32 ImportOffset // offset to import table
  if PackageVersion < 68
    uint32 HeritageCount
    uint32 HeritageOffset // offset to heritable table
  else
    byte   Guid[16]
    uint32 GenerationCount
    struct
    {
      uint32 ExportCount
      uint32 NameCount
    } Generations[GenerationCount]
  endif
}
```
The package format was clearly inspired by DLL files (oh what a bad idea that was). Each package has objects in it, and references to objects in other package files. The export table lists all objects in the package, while the import table lists all objects referenced by the objects in the package. The name table contains all text strings in the package.

## Name Table
```
struct NameTableEntry
{
  name Name
  uint32 Flags
} NameTable[FileHeader.NameCount]
```

## Export Table
```
struct ExportTableEntry
{
  index Class
  index Base
  uint32 Package
  index ObjectName
  index ObjectFlags
  index Size
  index Offset
} ExportTable[FileHeader.ExportCount]
```
The list of all objects in the package.

`Class` and `Base` are references to the object's class and base objects. If the reference is a positive number then it is an index to an entry in the export table. If it is a negative number then it is an entry in the import table. If the value is zero then it is null.

```c++
object GetObject(index objref)
{
	if (objref > 0) return ExportTable[objref - 1];
	else if (objref < 0) return ImportTable[-objref - 1];
	else return null;
}
```

If `Class` is zero then the object is a class object. Class objects are the classes in UnrealScript. `Base` then points at the base class.

If `Class` is not zero then the object is an instance of the class. `Class` points at the class type of the object. For example, a texture has `Class` pointing at the `Texture` class object.

`Size` is the byte size of the object in the package file. `Offset` is the file offset where the object data begins.

## Import Table
```
struct ImportTableEntry
{
	index ClassPackage
	index ClassName
	uint32 ObjectPackage
	index ObjectName
} ImportTable[FileHeader.ImportCount]
```
This is the list of objects not present in this package. They are referenced by the `Class` and `Base` fields in the export table.

The exact rules for how to look up objects here is not fully understood yet. See the `Package::GetUObject` and `Package::FindObjectReference` functions in Surreal Engine's Package.cpp for how to do it. These rules were derived from the UShock project and why they have to look like they do is still unknown. There's also some hacky stuff going on here in the Unreal Engine where the `UnrealI` and `UnrealShare` packages share objects.

## Heritage Table
```
struct HeritageTableEntry
{
  byte Guid[16]
} HeritageTable[FileHeader.HeritageCount]
```
No idea what this table was about. Most likely some kind of revision history baked into the package. Don't think it is used for anything.

## Object Format
All objects begin with the following header:
```
struct ObjectHeader
{
  // Savegame objects serializes their unrealscript state
  if ExportTable[obj].ObjectFlags & HasStack
    index Node
    index StateNode
    uint64 ProbeMask
    uint32 LatentAction
    if (Node != 0)
      index Offset
    endif
  endif

  // Instanced objects serializes their unrealscript property values
  if ExportTable[obj].Class == 0
    propertyblock properties
  endif
}
```
There are basically three different variants of the object header. First, there's class objects, which have an empty header. Then there are normal instanced objects (i.e. a texture or a level), they begin with an unrealscript property block. And then there's savegame objects, which also stores information about the unrealscript state for the object when being saved.

The data for each object type follows the header.

## Property Block
The property block is a list of properties terminated by a `None` name:
```c++
while (true)
{
  auto entry = NameTable[ReadIndex()];
  if (entry.Name == "None") break;
  Properties.Set(entry.Name, ReadPropertyValue());
}
```
Each property value begins with a small header:
```
struct PropertyHeader
{
	UnrealPropertyType type
	int arrayIndex
	bool boolValue
	index structName
	int size
}

enum UnrealPropertyType
{
	UPT_Invalid    = 0
	UPT_Byte       = 1
	UPT_Int        = 2
	UPT_Bool       = 3
	UPT_Float      = 4
	UPT_Object     = 5
	UPT_Name       = 6
	UPT_String     = 7
	UPT_Class      = 8
	UPT_Array      = 9
	UPT_Struct     = 10
	UPT_Vector     = 11
	UPT_Rotator    = 12
	UPT_Str        = 13
	UPT_Map        = 14
	UPT_FixedArray = 15
}
```
However, not all the fields of the header are always present. The header is decoded as follows:
```c++
uint8_t info = ReadByte();
bool infoBit = info & 0x80;

PropertyHeader header;
header.type = (UnrealPropertyType)(info & 0x0f);

if (header.type == UPT_Struct)
	header.structName = ReadIndex();

switch ((info & 0x70) >> 4)
{
case 0: header.size = 1; break;
case 1: header.size = 2; break;
case 2: header.size = 4; break;
case 3: header.size = 12; break;
case 4: header.size = 16; break;
case 5: header.size = ReadByte(); break;
case 6: header.size = ReadUInt16(); break;
case 7: header.size = ReadUInt32(); break;
}

header.arrayIndex = 0;
if (infoBit && header.type != UPT_Bool)
{
	int byte1 = ReadByte();
	if ((byte1 & 0xc0) == 0xc0)
	{
		byte1 &= 0x3f;
		int byte2 = ReadByte();
		int byte3 = ReadByte();
		int byte4 = ReadByte();
		header.arrayIndex = (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
	}
	else if (byte1 & 0x80)
	{
		byte1 &= 0x7f;
		int byte2 = ReadByte();
		header.arrayIndex = (byte1 << 8) | byte2;
	}
	else
	{
		header.arrayIndex = byte1;
	}
}
else if (header.type == UPT_Bool)
{
	header.boolValue = infoBit;
}
```
For some of the property types the value is stored in the header itself. For the remaining types the data follows the header:

- UPT_Invalid: Should never be present in a package file
- UPT_Byte: `byte` follows
- UPT_Int: `uint32` follows
- UPT_Bool: `header.boolValue` contains the value
- UPT_Float: `float` follows
- UPT_Object: `index` follows (object reference)
- UPT_Name: `index` follows (index into `NameTable`)
- UPT_String: `byte[header.size]` follows
- UPT_Class: Unknown. Not seen in 436
- UPT_Array: Unknown. Not seen in 436
- UPT_Struct: `header.structName` is the struct name of the data that follows
- UPT_Vector: `float x,y,z` follows
- UPT_Rotator: `uint32 pitch,yaw,roll` follows
- UPT_Str: `name` follows
- UPT_Map: `header.size`bytes follows
- UPT_FixedArray: `header.size` bytes follows

`header.arrayIndex` is the index of the value if the property is an array.

## Bitmap Object
```
struct Bitmap : ObjectHeader
{
}
```
The bitmap object contains no data on its own. It is the base class of the Texture object.

## Texture Object
```
struct Texture : Bitmap
{
  byte MipsCount
  struct Mipmap
  {
    if FileHeader.PackageVersion >= 63
	    uint32 OffsetToWidth
    endif
    index Size
    byte Data[Size]
    uint32 Width
    uint32 Height
    byte UBits
    byte VBits
  } Mipmaps[MipsCount]

  if Properties.bHasComp == true
    byte CompMipsCount
    struct CompMipmap
    {
      if FileHeader.PackageVersion >= 68
	      uint32 OffsetToWidth
      endif
      index Size
      byte Data[Size]
      uint32 Width
      uint32 Height
      byte UBits
      byte VBits
    } CompMipmaps[CompMipsCount]
  endif
}
```
The image format used by the texture is stored in the `Format` object property. If the `bHasComp` property exists and is true, then the texture contains additional data for a compressed version of the texture. The image format for the compressed version is stored in the `CompFormat` property.

## FractalTexture Object
```
struct FractalTexture : Texture
{
  byte Pixels[Properties.UClamp * Properties.VClamp]
}
```
Not really sure what this image is used for. Maybe `IceTexture` or one of the other derived texture objects uses it for something?

## FireTexture Object
```
struct FireTexture : FractalTexture
{
  index SparksCount
  struct Spark
  {
	  byte Type
	  byte Heat
	  byte X
	  byte Y
	  byte ByteA
	  byte ByteB
	  byte ByteC
	  byte ByteD
  } Sparks[SparksCount]
}
```
Fire textures are updated each frame by spawning sparks as described by the sparks array (particle generators on a texture). The `Type` field of the spark sets what kind of particle effect each emitter creates.

## Palette Object
```
struct Palette : ObjectHeader
{
  index Count
  uint32 Colors[Count] // Alpha,Red,Green,Blue
  if FileHeader.PackageVersion < 66
    alpha channel is not set in the Colors array
  endif
}
```
The palette used by a 8-bit texture.

## Font Object
```
struct Font : ObjectHeader
{
  if FileHeader.PackageVersion <= 63
    Unknown
  else
    index PageCount
    struct Page
    {
      index Texture // object reference to a Texture object
      index CharacterCount
      struct Character
      {
        uint32 StartU
        uint32 StartV
        uint32 USize
        uint32 VSize
      } Characters[CharacterCount]
    } Pages[PageCount]
    uint32 CharactersPerPage
  endif
}
```
The font object describes where each character glyph is positioned in font textures.

## Sound Object
```
struct Sound : ObjectHeader
{
  index Format // Index into NameTable
  if FileHeader.PackageVersion >= 63
    uint32 SkipOffset
  endif
  index Size
  byte Data[Size]
}
```
Sound effects. `Format` is the file extension for the sound.

## Music Object
```
struct Music : ObjectHeader
{
  index Format // Index into NameTable
  if FileHeader.PackageVersion >= 61
    uint32 SkipOffset
  endif
  index Size
  byte Data[Size]
}
```
Tracker music. `Format` is the file extension for the music.

## TextBuffer Object
```
struct TextBuffer : ObjectHeader
{
  uint32 Pos
  uint32 Top
  index Size
  byte Text[Size]
}
```
Unrealscript source code.

