
#include "Precomp.h"
#include "UObject.h"

UObject::UObject(ObjectStream* stream) : ClassName(stream->GetClsName())
{
	if (AllFlags(stream->GetFlags(), ObjectFlags::HasStack))
	{
		int32_t node = stream->ReadIndex();
		int32_t stateNode = stream->ReadIndex();
		int64_t probeMask = stream->ReadInt64();
		int32_t latentAction = stream->ReadInt32();
		if (node != 0)
		{
			int offset = stream->ReadIndex();
		}
	}

	if (!AllFlags(stream->GetFlags(), ObjectFlags::Native))
	{
		while (true)
		{
			std::string name = stream->ReadName();
			if (name == "None")
				break;

			UnrealProperty prop;
			prop.Name = name;

			uint8_t info = stream->ReadInt8();
			prop.IsArray = info & 0x80;
			prop.Type = (UnrealPropertyType)(info & 0x0f);

			std::string structName;
			if (prop.Type == UPT_Struct)
				structName = stream->ReadName();

			int size;
			switch ((info & 0x70) >> 4)
			{
			default:
			case 0: size = 1; break;
			case 1: size = 2; break;
			case 2: size = 4; break;
			case 3: size = 12; break;
			case 4: size = 16; break;
			case 5: size = stream->ReadInt8(); break;
			case 6: size = stream->ReadInt16(); break;
			case 7: size = stream->ReadInt32(); break;
			}

			int arrayIndex = 0;
			if (prop.IsArray && prop.Type != UPT_Bool)
			{
				arrayIndex = stream->ReadUInt8();
				if (arrayIndex & (1 << 7))
				{
					arrayIndex &= ~(1 << 7);
					arrayIndex |= ((int)stream->ReadUInt8()) << 8;
					if (arrayIndex & (1 << 15))
					{
						arrayIndex &= ~(1 << 15);
						arrayIndex |= ((int)stream->ReadUInt8()) << 16;
						arrayIndex |= ((int)stream->ReadUInt8()) << 24;
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
				prop.Scalar.ValueByte = stream->ReadInt8();
				break;
			case UPT_Int:
				prop.Scalar.ValueInt = stream->ReadInt32();
				break;
			case UPT_Bool:
				prop.Scalar.ValueBool = prop.IsArray;
				prop.IsArray = false;
				break;
			case UPT_Float:
				prop.Scalar.ValueFloat = stream->ReadFloat();
				break;
			case UPT_Object:
				prop.Scalar.ValueObject = stream->ReadIndex();
				break;
			case UPT_Name:
				prop.Scalar.ValueString = stream->ReadName();
				break;
			case UPT_String:
			{
				int len = size;
				std::vector<char> s;
				s.resize(len);
				stream->ReadBytes(s.data(), (int)s.size());
				s.push_back(0);
				prop.Scalar.ValueString = s.data();
			}
			break;
			case UPT_Class:
				stream->Skip(size);
				break;
			case UPT_Array:
				throw std::runtime_error("Array properties not implemented");
				// SerializeArray( Ar );
				break;
			case UPT_Struct:
				prop.Scalar.ValueString = structName;
				if (structName == "Vector")
				{
					prop.Scalar.ValueVector.x = stream->ReadFloat();
					prop.Scalar.ValueVector.y = stream->ReadFloat();
					prop.Scalar.ValueVector.z = stream->ReadFloat();
				}
				else if (structName == "Sphere")
				{
					float x = stream->ReadFloat();
					float y = stream->ReadFloat();
					float z = stream->ReadFloat();
				}
				else if (structName == "Coords")
				{
					float Originx = stream->ReadFloat();
					float Originy = stream->ReadFloat();
					float Originz = stream->ReadFloat();

					float XAxisx = stream->ReadFloat();
					float XAxisy = stream->ReadFloat();
					float XAxisz = stream->ReadFloat();

					float YAxisx = stream->ReadFloat();
					float YAxisy = stream->ReadFloat();
					float YAxisz = stream->ReadFloat();

					float ZAxisx = stream->ReadFloat();
					float ZAxisy = stream->ReadFloat();
					float ZAxisz = stream->ReadFloat();
				}
				else if (structName == "ModelCoords")
				{
					// PointXform, VectorXform
					for (int i = 0; i < 2; i++)
					{
						float Originx = stream->ReadFloat();
						float Originy = stream->ReadFloat();
						float Originz = stream->ReadFloat();

						float XAxisx = stream->ReadFloat();
						float XAxisy = stream->ReadFloat();
						float XAxisz = stream->ReadFloat();

						float YAxisx = stream->ReadFloat();
						float YAxisy = stream->ReadFloat();
						float YAxisz = stream->ReadFloat();

						float ZAxisx = stream->ReadFloat();
						float ZAxisy = stream->ReadFloat();
						float ZAxisz = stream->ReadFloat();
					}
				}
				else if (structName == "Rotator")
				{
					prop.Scalar.ValueRotator.Pitch = stream->ReadInt32();
					prop.Scalar.ValueRotator.Yaw = stream->ReadInt32();
					prop.Scalar.ValueRotator.Roll = stream->ReadInt32();
				}
				else if (structName == "Box")
				{
					float minx = stream->ReadFloat();
					float miny = stream->ReadFloat();
					float minz = stream->ReadFloat();

					float maxx = stream->ReadFloat();
					float maxy = stream->ReadFloat();
					float maxz = stream->ReadFloat();

					uint8_t isvalid = stream->ReadInt8();
				}
				else if (structName == "Color")
				{
					int8_t red = stream->ReadInt8();
					int8_t green = stream->ReadInt8();
					int8_t blue = stream->ReadInt8();
					int8_t alpha = stream->ReadInt8();
				}
				else if (structName == "Matrix")
				{
					// XPlane >> YPlane >> ZPlane >> WPlane
					for (int i = 0; i < 4; i++)
					{
						float x = stream->ReadFloat();
						float y = stream->ReadFloat();
						float z = stream->ReadFloat();
						float w = stream->ReadFloat();
					}
				}
				else if (structName == "Plane")
				{
					float x = stream->ReadFloat();
					float y = stream->ReadFloat();
					float z = stream->ReadFloat();
					float w = stream->ReadFloat();
				}
				else if (structName == "Scale")
				{
					float x = stream->ReadFloat();
					float y = stream->ReadFloat();
					float z = stream->ReadFloat();
					float sheerRate = stream->ReadFloat();
					UnrealSheerAxis sheerAxis = (UnrealSheerAxis)stream->ReadInt8();
				}
				else if (structName == "PointRegion")
				{
					int32_t zoneIndex = stream->ReadIndex(); // AZoneInfo
					int32_t bspLeaf = stream->ReadInt32();
					uint8_t zoneNumber = stream->ReadInt8();
				}
				else
				{
					throw std::runtime_error("Unimplemented struct '" + structName + "' encountered");
				}
				break;
			case UPT_Vector:
				prop.Scalar.ValueVector.x = stream->ReadFloat();
				prop.Scalar.ValueVector.y = stream->ReadFloat();
				prop.Scalar.ValueVector.z = stream->ReadFloat();
				break;
			case UPT_Rotator:
				prop.Scalar.ValueRotator.Pitch = stream->ReadInt32();
				prop.Scalar.ValueRotator.Yaw = stream->ReadInt32();
				prop.Scalar.ValueRotator.Roll = stream->ReadInt32();
				break;
			case UPT_Str:
				prop.Scalar.ValueString = stream->ReadString();
				break;
			case UPT_Map:
				stream->Skip(size);
				break;
			case UPT_FixedArray:
				stream->Skip(size);
				break;
			}

			Properties.Properties.push_back(prop);
		}
	}
}
