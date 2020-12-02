
#include "Precomp.h"
#include "PackageObject.h"
#include "BinaryStream.h"
#include "Package.h"

PackageObject::PackageObject(::Package* package, ExportTableEntry* entry) : Package(package)
{
	Stream = package->OpenStream();
	Stream->Seek(entry->ObjOffset);

	if (entry->ObjFlags & RF_HasStack)
	{
		int32_t node = Stream->ReadIndex();
		int32_t stateNode = Stream->ReadIndex();
		int64_t probeMask = Stream->ReadInt64();
		int32_t latentAction = Stream->ReadInt32();
		if (node != 0)
		{
			int offset = Stream->ReadIndex();
		}
	}

	if (!(entry->ObjFlags & RF_Native))
	{
		while (true)
		{
			int nameIndex = Stream->ReadIndex();
			std::string name = package->GetName(nameIndex);
			if (name == "None")
				break;

			UnrealProperty prop;
			prop.Name = name;

			uint8_t info = Stream->ReadInt8();
			prop.IsArray = info & 0x80;
			prop.Type = (UnrealPropertyType)(info & 0x0f);

			std::string structName;
			if (prop.Type == UPT_Struct)
				structName = package->GetName(Stream->ReadIndex());

			int size;
			switch ((info & 0x70) >> 4)
			{
			default:
			case 0: size = 1; break;
			case 1: size = 2; break;
			case 2: size = 4; break;
			case 3: size = 12; break;
			case 4: size = 16; break;
			case 5: size = Stream->ReadInt8(); break;
			case 6: size = Stream->ReadInt16(); break;
			case 7: size = Stream->ReadInt32(); break;
			}

			int arrayIndex = 0;
			if (prop.IsArray && prop.Type != UPT_Bool)
			{
				arrayIndex = Stream->ReadUInt8();
				if (arrayIndex & (1 << 7))
				{
					arrayIndex &= ~(1 << 7);
					arrayIndex |= ((int)Stream->ReadUInt8()) << 8;
					if (arrayIndex & (1 << 15))
					{
						arrayIndex &= ~(1 << 15);
						arrayIndex |= ((int)Stream->ReadUInt8()) << 16;
						arrayIndex |= ((int)Stream->ReadUInt8()) << 24;
					}
				}
			}
			prop.ArrayIndex = arrayIndex;

			switch (prop.Type)
			{
			default:
			case UPT_Invalid:
				break;
			case UPT_Byte:
				prop.Scalar.ValueByte = Stream->ReadInt8();
				break;
			case UPT_Int:
				prop.Scalar.ValueInt = Stream->ReadInt32();
				break;
			case UPT_Bool:
				prop.Scalar.ValueBool = prop.IsArray;
				prop.IsArray = false;
				break;
			case UPT_Float:
				prop.Scalar.ValueFloat = Stream->ReadFloat();
				break;
			case UPT_Object:
				prop.Scalar.ValueObject = Stream->ReadIndex();
				break;
			case UPT_Name:
				prop.Scalar.ValueString = package->GetName(Stream->ReadIndex());
				break;
			case UPT_String:
				{
					int len = size;
					std::vector<char> s;
					s.resize(len);
					Stream->ReadBytes(s.data(), (int)s.size());
					s.push_back(0);
					prop.Scalar.ValueString = s.data();
				}
				break;
			case UPT_Class:
				Stream->Skip(size);
				break;
			case UPT_Array:
				throw std::runtime_error("Array properties not implemented");
				// SerializeArray( Ar );
				break;
			case UPT_Struct:
				prop.Scalar.ValueString = structName;
				if (structName == "Vector")
				{
					prop.Scalar.ValueVector.x = Stream->ReadFloat();
					prop.Scalar.ValueVector.y = Stream->ReadFloat();
					prop.Scalar.ValueVector.z = Stream->ReadFloat();
				}
				else if (structName == "Sphere")
				{
					float x = Stream->ReadFloat();
					float y = Stream->ReadFloat();
					float z = Stream->ReadFloat();
				}
				else if (structName == "Coords")
				{
					float Originx = Stream->ReadFloat();
					float Originy = Stream->ReadFloat();
					float Originz = Stream->ReadFloat();

					float XAxisx = Stream->ReadFloat();
					float XAxisy = Stream->ReadFloat();
					float XAxisz = Stream->ReadFloat();

					float YAxisx = Stream->ReadFloat();
					float YAxisy = Stream->ReadFloat();
					float YAxisz = Stream->ReadFloat();

					float ZAxisx = Stream->ReadFloat();
					float ZAxisy = Stream->ReadFloat();
					float ZAxisz = Stream->ReadFloat();
				}
				else if (structName == "ModelCoords")
				{
					// PointXform, VectorXform
					for (int i = 0; i < 2; i++)
					{
						float Originx = Stream->ReadFloat();
						float Originy = Stream->ReadFloat();
						float Originz = Stream->ReadFloat();

						float XAxisx = Stream->ReadFloat();
						float XAxisy = Stream->ReadFloat();
						float XAxisz = Stream->ReadFloat();

						float YAxisx = Stream->ReadFloat();
						float YAxisy = Stream->ReadFloat();
						float YAxisz = Stream->ReadFloat();

						float ZAxisx = Stream->ReadFloat();
						float ZAxisy = Stream->ReadFloat();
						float ZAxisz = Stream->ReadFloat();
					}
				}
				else if (structName == "Rotator")
				{
					prop.Scalar.ValueRotator.Pitch = Stream->ReadInt32();
					prop.Scalar.ValueRotator.Yaw = Stream->ReadInt32();
					prop.Scalar.ValueRotator.Roll = Stream->ReadInt32();
				}
				else if (structName == "Box")
				{
					float minx = Stream->ReadFloat();
					float miny = Stream->ReadFloat();
					float minz = Stream->ReadFloat();

					float maxx = Stream->ReadFloat();
					float maxy = Stream->ReadFloat();
					float maxz = Stream->ReadFloat();

					uint8_t isvalid = Stream->ReadInt8();
				}
				else if (structName == "Color")
				{
					int8_t red = Stream->ReadInt8();
					int8_t green = Stream->ReadInt8();
					int8_t blue = Stream->ReadInt8();
					int8_t alpha = Stream->ReadInt8();
				}
				else if (structName == "Matrix")
				{
					// XPlane >> YPlane >> ZPlane >> WPlane
					for (int i = 0; i < 4; i++)
					{
						float x = Stream->ReadFloat();
						float y = Stream->ReadFloat();
						float z = Stream->ReadFloat();
						float w = Stream->ReadFloat();
					}
				}
				else if (structName == "Plane")
				{
					float x = Stream->ReadFloat();
					float y = Stream->ReadFloat();
					float z = Stream->ReadFloat();
					float w = Stream->ReadFloat();
				}
				else if (structName == "Scale")
				{
					float x = Stream->ReadFloat();
					float y = Stream->ReadFloat();
					float z = Stream->ReadFloat();
					float sheerRate = Stream->ReadFloat();
					UnrealSheerAxis sheerAxis = (UnrealSheerAxis)Stream->ReadInt8();
				}
				else if (structName == "PointRegion")
				{
					int32_t zoneIndex = Stream->ReadIndex(); // AZoneInfo
					int32_t bspLeaf = Stream->ReadInt32();
					uint8_t zoneNumber = Stream->ReadInt8();
				}
				else
				{
					throw std::runtime_error("Unimplemented struct '" + structName + "' encountered");
				}
				break;
			case UPT_Vector:
				prop.Scalar.ValueVector.x = Stream->ReadFloat();
				prop.Scalar.ValueVector.y = Stream->ReadFloat();
				prop.Scalar.ValueVector.z = Stream->ReadFloat();
				break;
			case UPT_Rotator:
				prop.Scalar.ValueRotator.Pitch = Stream->ReadInt32();
				prop.Scalar.ValueRotator.Yaw = Stream->ReadInt32();
				prop.Scalar.ValueRotator.Roll = Stream->ReadInt32();
				break;
			case UPT_Str:
				prop.Scalar.ValueString = Stream->ReadString();
				break;
			case UPT_Map:
				Stream->Skip(size);
				break;
			case UPT_FixedArray:
				Stream->Skip(size);
				break;
			}

			Properties.push_back(prop);
		}
	}
}
