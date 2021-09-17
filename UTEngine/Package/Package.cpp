
#include "Precomp.h"
#include "Package.h"
#include "PackageStream.h"
#include "PackageManager.h"
#include "UObject/UObject.h"
#include "UObject/UClass.h"
#include "UObject/UProperty.h"
#include "UObject/UFont.h"
#include "UObject/ULevel.h"
#include "UObject/UMesh.h"
#include "UObject/UTexture.h"
#include "UObject/UActor.h"
#include "UObject/USound.h"
#include "UObject/UMusic.h"
#include "UObject/UTextBuffer.h"
#include "UObject/UClient.h"
#include "File.h"

Package::Package(PackageManager* packageManager, const NameString& name, const std::string& filename) : Packages(packageManager), Name(name), Filename(filename)
{
	ReadTables();

	std::string nameKey = GetNameKey(name);
	bool corePackage = nameKey == "core";
	bool enginePackage = nameKey == "engine";
	bool ipdrvPackage = nameKey == "ipdrv";

	RegisterNativeClass<UObject>(corePackage, "Object");
	RegisterNativeClass<UField>(corePackage, "Field", "Object");
	RegisterNativeClass<UConst>(corePackage, "Const", "Field");
	RegisterNativeClass<UEnum>(corePackage, "Enum", "Field");
	RegisterNativeClass<UStruct>(corePackage, "Struct", "Field");
	RegisterNativeClass<UFunction>(corePackage, "Function", "Struct");
	RegisterNativeClass<UState>(corePackage, "State", "Struct");
	RegisterNativeClass<UClass>(corePackage, "Class", "State");
	RegisterNativeClass<UProperty>(corePackage, "Property", "Field");
	RegisterNativeClass<UPointerProperty>(corePackage, "PointerProperty", "Property");
	RegisterNativeClass<UByteProperty>(corePackage, "ByteProperty", "Property");
	RegisterNativeClass<UObjectProperty>(corePackage, "ObjectProperty", "Property");
	RegisterNativeClass<UClassProperty>(corePackage, "ClassProperty", "ObjectProperty");
	RegisterNativeClass<UFixedArrayProperty>(corePackage, "FixedArrayProperty", "Property");
	RegisterNativeClass<UArrayProperty>(corePackage, "ArrayProperty", "Property");
	RegisterNativeClass<UMapProperty>(corePackage, "MapProperty", "Property");
	RegisterNativeClass<UStructProperty>(corePackage, "StructProperty", "Property");
	RegisterNativeClass<UIntProperty>(corePackage, "IntProperty", "Property");
	RegisterNativeClass<UBoolProperty>(corePackage, "BoolProperty", "Property");
	RegisterNativeClass<UFloatProperty>(corePackage, "FloatProperty", "Property");
	RegisterNativeClass<UNameProperty>(corePackage, "NameProperty", "Property");
	RegisterNativeClass<UStrProperty>(corePackage, "StrProperty", "Property");
	RegisterNativeClass<UStringProperty>(corePackage, "StringProperty", "Property");
	RegisterNativeClass<UTextBuffer>(corePackage, "TextBuffer", "Object");

	if (packageManager->GetEngineVersion() < 400)
	{
		RegisterNativeClass<UObject>(corePackage, "Commandlet", "Object");
		RegisterNativeClass<UObject>(corePackage, "SimpleCommandlet", "Commandlet");
		RegisterNativeClass<UObject>(enginePackage, "RenderIterator", "Object");
	}

	RegisterNativeClass<UFont>(enginePackage, "Font", "Object");
	RegisterNativeClass<UPalette>(enginePackage, "Palette", "Object");
	RegisterNativeClass<USound>(enginePackage, "Sound", "Object");
	RegisterNativeClass<UMusic>(enginePackage, "Music", "Object");

	RegisterNativeClass<UPrimitive>(enginePackage, "Primitive", "Object");
	RegisterNativeClass<UMesh>(enginePackage, "Mesh", "Primitive");
	RegisterNativeClass<ULodMesh>(enginePackage, "LodMesh", "Mesh");
	RegisterNativeClass<USkeletalMesh>(enginePackage, "SkeletalMesh", "LodMesh");
	RegisterNativeClass<UAnimation>(enginePackage, "Animation", "Object");

	RegisterNativeClass<UModel>(enginePackage, "Model", "Primitive");
	RegisterNativeClass<ULevelBase>(enginePackage, "LevelBase", "Object");
	RegisterNativeClass<ULevel>(enginePackage, "Level", "LevelBase");
	RegisterNativeClass<ULevelSummary>(enginePackage, "LevelSummary", "Object");
	RegisterNativeClass<UPolys>(enginePackage, "Polys", "Object");
	RegisterNativeClass<UBspNodes>(enginePackage, "BspNodes", "Object");
	RegisterNativeClass<UBspSurfs>(enginePackage, "BspSurfs", "Object");
	RegisterNativeClass<UVectors>(enginePackage, "Vectors", "Object");
	RegisterNativeClass<UVerts>(enginePackage, "Verts", "Object");

	RegisterNativeClass<UBitmap>(enginePackage, "Texture", "Object");
	RegisterNativeClass<UTexture>(enginePackage, "Texture", "Bitmap");
	RegisterNativeClass<UFractalTexture>(enginePackage, "FractalTexture", "Texture");
	RegisterNativeClass<UFireTexture>(enginePackage, "FireTexture", "FractalTexture");
	RegisterNativeClass<UIceTexture>(enginePackage, "IceTexture", "FractalTexture");
	RegisterNativeClass<UWaterTexture>(enginePackage, "WaterTexture", "FractalTexture");
	RegisterNativeClass<UWaveTexture>(enginePackage, "WaveTexture", "WaterTexture");
	RegisterNativeClass<UWetTexture>(enginePackage, "WetTexture", "WaterTexture");
	RegisterNativeClass<UScriptedTexture>(enginePackage, "ScriptedTexture", "Texture");

	RegisterNativeClass<UClient>(enginePackage, "Client", "Object");
	RegisterNativeClass<UViewport>(enginePackage, "Viewport", "Player");
	RegisterNativeClass<UCanvas>(enginePackage, "Canvas", "Object");
	RegisterNativeClass<UConsole>(enginePackage, "Console", "Object");
	RegisterNativeClass<UPlayer>(enginePackage, "Player", "Object");
	RegisterNativeClass<UNetConnection>(enginePackage, "NetConnection", "Player");
	RegisterNativeClass<UDemoRecConnection>(enginePackage, "DemoRecConnection", "NetConnection");
	RegisterNativeClass<UPendingLevel>(enginePackage, "PendingLevel", "Object");
	RegisterNativeClass<UNetPendingLevel>(enginePackage, "NetPendingLevel", "PendingLevel");
	RegisterNativeClass<UDemoPlayPendingLevel>(enginePackage, "DemoPlayPendingLevel", "PendingLevel");
	RegisterNativeClass<UChannel>(enginePackage, "Channel", "Object");
	RegisterNativeClass<UControlChannel>(enginePackage, "ControlChannel", "Channel");
	RegisterNativeClass<UActorChannel>(enginePackage, "ActorChannel", "Channel");
	RegisterNativeClass<UFileChannel>(enginePackage, "FileChannel", "Channel");

	RegisterNativeClass<UActor>(enginePackage, "Actor", "Object");
	RegisterNativeClass<ULight>(enginePackage, "Light", "Actor");
	RegisterNativeClass<UInventory>(enginePackage, "Inventory", "Actor");
	RegisterNativeClass<UWeapon>(enginePackage, "Weapon", "Inventory");
	RegisterNativeClass<UNavigationPoint>(enginePackage, "NavigationPoint", "Actor");
	RegisterNativeClass<ULiftExit>(enginePackage, "LiftExit", "NavigationPoint");
	RegisterNativeClass<ULiftCenter>(enginePackage, "LiftCenter", "NavigationPoint");
	RegisterNativeClass<UWarpZoneMarker>(enginePackage, "WarpZoneMarker", "NavigationPoint");
	RegisterNativeClass<UInventorySpot>(enginePackage, "InventorySpot", "NavigationPoint");
	RegisterNativeClass<UTriggerMarker>(enginePackage, "TriggerMarker", "NavigationPoint");
	RegisterNativeClass<UButtonMarker>(enginePackage, "ButtonMarker", "NavigationPoint");
	RegisterNativeClass<UPlayerStart>(enginePackage, "PlayerStart", "NavigationPoint");
	RegisterNativeClass<UTeleporter>(enginePackage, "Teleporter", "NavigationPoint");
	RegisterNativeClass<UPathNode>(enginePackage, "PathNode", "NavigationPoint");
	RegisterNativeClass<UDecoration>(enginePackage, "Decoration", "Actor");
	RegisterNativeClass<UCarcass>(enginePackage, "Carcass", "Decoration");
	RegisterNativeClass<UProjectile>(enginePackage, "Projectile", "Actor");
	RegisterNativeClass<UKeypoint>(enginePackage, "Keypoint", "Actor");
	RegisterNativeClass<Ulocationid>(enginePackage, "locationid", "Keypoint");
	RegisterNativeClass<UInterpolationPoint>(enginePackage, "InterpolationPoint", "Keypoint");
	RegisterNativeClass<UTriggers>(enginePackage, "Triggers", "Actor");
	RegisterNativeClass<UTrigger>(enginePackage, "Trigger", "Triggers");
	RegisterNativeClass<UHUD>(enginePackage, "HUD", "Actor");
	RegisterNativeClass<UMenu>(enginePackage, "Menu", "Actor");
	RegisterNativeClass<UInfo>(enginePackage, "Info", "Actor");
	RegisterNativeClass<UMutator>(enginePackage, "Mutator", "Info");
	RegisterNativeClass<UGameInfo>(enginePackage, "GameInfo", "Info");
	RegisterNativeClass<UZoneInfo>(enginePackage, "ZoneInfo", "Info");
	RegisterNativeClass<ULevelInfo>(enginePackage, "LevelInfo", "ZoneInfo");
	RegisterNativeClass<UWarpZoneInfo>(enginePackage, "WarpZoneInfo", "ZoneInfo");
	RegisterNativeClass<USkyZoneInfo>(enginePackage, "SkyZoneInfo", "ZoneInfo");
	RegisterNativeClass<USavedMove>(enginePackage, "SavedMove", "Info");
	RegisterNativeClass<UReplicationInfo>(enginePackage, "ReplicationInfo", "Info");
	RegisterNativeClass<UPlayerReplicationInfo>(enginePackage, "PlayerReplicationInfo", "ReplicationInfo");
	RegisterNativeClass<UGameReplicationInfo>(enginePackage, "GameReplicationInfo", "ReplicationInfo");
	RegisterNativeClass<UInternetInfo>(enginePackage, "InternetInfo", "Info");
	RegisterNativeClass<UStatLog>(enginePackage, "StatLog", "Info");
	RegisterNativeClass<UStatLogFile>(enginePackage, "StatLogFile", "StatLog");
	RegisterNativeClass<UDecal>(enginePackage, "Decal", "Actor");
	RegisterNativeClass<USpawnNotify>(enginePackage, "SpawnNotify", "Actor");
	RegisterNativeClass<UBrush>(enginePackage, "Brush", "Actor");
	RegisterNativeClass<UMover>(enginePackage, "Mover", "Brush");
	RegisterNativeClass<UPawn>(enginePackage, "Pawn", "Actor");
	RegisterNativeClass<UScout>(enginePackage, "Scout", "Pawn");
	RegisterNativeClass<UPlayerPawn>(enginePackage, "PlayerPawn", "Pawn");
	RegisterNativeClass<UCamera>(enginePackage, "Camera", "PlayerPawn");

	RegisterNativeClass<UInternetLink>(ipdrvPackage, "InternetLink", "InternetInfo");
	RegisterNativeClass<UTcpLink>(ipdrvPackage, "TcpLink", "InternetLink");
	RegisterNativeClass<UUdpLink>(ipdrvPackage, "UdpLink", "InternetLink");

	Objects.resize(ExportTable.size());
}

Package::~Package()
{
}

std::vector<UClass*> Package::GetAllClasses()
{
	std::vector<UClass*> classes;
	int objref = 1;
	for (ExportTableEntry& e : ExportTable)
	{
		if (e.ObjClass == 0)
		{
			classes.push_back(UObject::Cast<UClass>(GetUObject(objref)));
		}
		objref++;
	}
	return classes;
}

UObject* Package::NewObject(const NameString& objname, UClass* objclass, ObjectFlags flags, bool initProperties)
{
	for (UClass* cur = objclass; cur != nullptr; cur = static_cast<UClass*>(cur->BaseStruct))
	{
		auto it = NativeClasses.find(GetNameKey(cur->Name));
		if (it != NativeClasses.end())
		{
			UObject* obj = it->second(objname, objclass, flags);
			if (initProperties)
			{
				obj->PropertyData.Init(objclass);
				obj->SetObject("Class", obj->Class);
				obj->SetString("Name", obj->Name);
				obj->SetInt("ObjectFlags", (int)obj->Flags);
			}
			return obj;
		}
	}

	throw std::runtime_error("Could not find the native class for " + objname);
}

void Package::LoadExportObject(int index)
{
	const ExportTableEntry* entry = &ExportTable[index];

	SetDelayLoadActive delayload(Packages);

	NameString objname = GetName(entry->ObjName);

	if (entry->ObjClass != 0)
	{
		UClass* objclass = UObject::Cast<UClass>(GetUObject(entry->ObjClass));
		if (!objclass)
		{
			objclass = UObject::Cast<UClass>(GetUObject(entry->ObjClass));
			throw std::runtime_error("Could not find the object class for " + objname);
		}

		Objects[index].reset(NewObject(objname, objclass, ExportTable[index].ObjFlags, false));
		Objects[index]->DelayLoad.reset(new ObjectDelayLoad(this, index, objname, objclass));
		Packages->delayLoads.push_back(Objects[index].get());
	}
	else
	{
		UClass* objbase = UObject::Cast<UClass>(GetUObject(entry->ObjBase));
		if (!objbase && objname != "Object")
			objbase = UObject::Cast<UClass>(Packages->GetPackage("Core")->GetUObject("Class", "Object"));
		auto obj = std::make_unique<UClass>(objname, objbase, ExportTable[index].ObjFlags);
		Objects[index] = std::move(obj);
		Objects[index]->DelayLoad.reset(new ObjectDelayLoad(this, index, objname, objbase));
		Packages->delayLoads.push_back(Objects[index].get());
	}
}

UObject* Package::GetUObject(int objref)
{
	if (objref > 0) // Export table object
	{
		int index = objref - 1;

		if ((size_t)index > Objects.size())
			throw std::runtime_error("Invalid object reference");

		if (!Objects[index])
			LoadExportObject(index);

		if (Packages->delayLoadActive == 0)
			Packages->DelayLoadNow();

		return Objects[index].get();
	}
	else if (objref < 0) // Import table object
	{
		ImportTableEntry* entry = GetImportEntry(objref);
		ImportTableEntry* entrypackage = GetImportEntry(entry->ObjPackage);

		NameString groupName;
		if (entrypackage->ObjPackage != 0)
			groupName = GetName(entrypackage->ObjName);

		while (entrypackage->ObjPackage != 0)
			entrypackage = GetImportEntry(entrypackage->ObjPackage);

		NameString packageName = GetName(entrypackage->ObjName);
		NameString objectName = GetName(entry->ObjName);
		NameString className = GetName(entry->ClassName);
		// NameString classPackage = GetName(entry->ClassPackage);

		UObject* obj = Packages->GetPackage(packageName)->GetUObject(className, objectName, groupName);

		// What a garbage engine!
		if (!obj && packageName == "UnrealI")
			obj = Packages->GetPackage("UnrealShare")->GetUObject(className, objectName, groupName);
		else if (!obj && packageName == "UnrealShare")
			obj = Packages->GetPackage("UnrealI")->GetUObject(className, objectName, groupName);

		return obj;
	}
	else
	{
		return nullptr;
	}
}

UObject* Package::GetUObject(const NameString& className, const NameString& objectName, const NameString& groupName)
{
	return GetUObject(FindObjectReference(className, objectName, groupName));
}

int Package::FindObjectReference(const NameString& className, const NameString& objectName, const NameString& groupName)
{
	bool isClass = CompareNames(className, "Class");

	size_t count = ExportTable.size();
	for (size_t index = 0; index < count; index++)
	{
		ExportTableEntry& entry = ExportTable[index];
		if (!CompareNames(GetName(entry.ObjName), objectName))
			continue;

		if (!groupName.empty())
		{
			if (entry.ObjPackage > 0)
			{
				auto package = GetExportEntry(entry.ObjPackage);
				if (package && !CompareNames(groupName, GetName(package->ObjName)))
					continue;
			}
			else if (entry.ObjPackage < 0)
			{
				auto package = GetImportEntry(entry.ObjPackage);
				if (package && !CompareNames(groupName, GetName(package->ObjName)))
					continue;
			}
			else
			{
				continue;
			}
		}

		if (isClass)
		{
			if (entry.ObjClass == 0)
			{
				return (int)index + 1;
			}
			else if (entry.ObjClass < 0)
			{
				auto classImport = GetImportEntry(entry.ObjClass);
				if (classImport && CompareNames(className, GetName(classImport->ObjName)))
					return (int)index + 1;
			}
			else
			{
				auto classExport = &ExportTable[entry.ObjClass + 1];
				if (classExport && CompareNames(className, GetName(classExport->ObjName)))
					return (int)index + 1;
			}
		}
		else if (entry.ObjClass != 0)
		{
			UClass* cls = UObject::Cast<UClass>(GetUObject(entry.ObjClass));
			while (cls)
			{
				if (CompareNames(className, cls->Name))
					return (int)index + 1;
				cls = static_cast<UClass*>(cls->BaseStruct);
			}
		}
	}

	return 0;
}

const NameString& Package::GetName(int index) const
{
	if (index >= 0 && (size_t)index < NameTable.size())
		return NameTable[index].Name;
	else
		throw std::runtime_error("Name index out of bounds!");
}

ExportTableEntry* Package::GetExportEntry(int objref)
{
	if (objref == 0)
		return nullptr;
	else if (objref < 0)
		throw std::runtime_error("Expected an export table entry");

	int index = objref - 1;
	if ((size_t)index >= ExportTable.size())
		throw std::runtime_error("Export table entry out of bounds!");

	return ExportTable.data() + index;
}

ImportTableEntry* Package::GetImportEntry(int objref)
{
	if (objref == 0)
		return nullptr;
	else if (objref > 0)
		throw std::runtime_error("Expected an import table entry");

	int index = -objref - 1;
	if ((size_t)index >= ImportTable.size())
		throw std::runtime_error("Import table entry out of bounds!");

	return ImportTable.data() + index;
}

void Package::ReadTables()
{
	auto stream = Packages->GetStream(this);

	uint32_t signature = stream->ReadInt32();
	if (signature != 0x9E2A83C1)
		throw std::runtime_error("Not an unreal package file");

	Version = stream->ReadInt16();
	uint16_t licenseeMode = stream->ReadInt16();

	if (Version < 60 || Version >= 100)
		throw std::runtime_error("Unsupported unreal package version");

	Flags = (PackageFlags)stream->ReadUInt32();

	uint32_t nameCount = stream->ReadInt32();
	uint32_t nameOffset = stream->ReadInt32();

	uint32_t exportCount = stream->ReadInt32();
	uint32_t exportOffset = stream->ReadInt32();

	uint32_t importCount = stream->ReadInt32();
	uint32_t importOffset = stream->ReadInt32();

	if (Version < 68)
	{
		uint32_t heritageCount = stream->ReadInt32();
		uint32_t heritageOffset = stream->ReadInt32();
	}
	else
	{
		uint8_t guid[16];
		stream->ReadBytes(guid, 16);
		uint32_t generationCount = stream->ReadInt32();
		for (uint32_t i = 0; i < generationCount; i++)
		{
			uint32_t genExportCount = stream->ReadInt32();
			uint32_t genNameCount = stream->ReadInt32();
		}
	}

	stream->Seek(nameOffset);
	for (uint32_t i = 0; i < nameCount; i++)
	{
		NameTableEntry entry;
		entry.Name = stream->ReadString();
		entry.Flags = stream->ReadInt32();
		NameTable.push_back(entry);
		NameHash[GetNameKey(entry.Name)] = i;
	}

	stream->Seek(exportOffset);
	for (uint32_t i = 0; i < exportCount; i++)
	{
		ExportTableEntry entry;
		entry.ObjClass = stream->ReadIndex();
		entry.ObjBase = stream->ReadIndex();
		entry.ObjPackage = stream->ReadInt32();
		entry.ObjName = stream->ReadIndex();
		entry.ObjFlags = (ObjectFlags)stream->ReadInt32();
		entry.ObjSize = stream->ReadIndex();
		entry.ObjOffset = (entry.ObjSize > 0) ? stream->ReadIndex() : -1;
		ExportTable.push_back(entry);
	}

	stream->Seek(importOffset);
	for (uint32_t i = 0; i < importCount; i++)
	{
		ImportTableEntry entry;
		entry.ClassPackage = stream->ReadIndex();
		entry.ClassName = stream->ReadIndex();
		entry.ObjPackage = stream->ReadInt32();
		entry.ObjName = stream->ReadIndex();
		ImportTable.push_back(entry);
	}
}

std::unique_ptr<ObjectStream> Package::OpenObjectStream(int index, const NameString& name, UClass* base)
{
	const auto& entry = ExportTable[index];
	if (entry.ObjSize > 0)
	{
		std::unique_ptr<uint64_t[]> buffer(new uint64_t[(entry.ObjSize + 7) / 8]);
		auto stream = Packages->GetStream(this);
		stream->Seek(entry.ObjOffset);
		stream->ReadBytes(buffer.get(), entry.ObjSize);
		return std::make_unique<ObjectStream>(this, std::move(buffer), entry.ObjOffset, entry.ObjSize, entry.ObjFlags, name, base);
	}
	else
	{
		return std::make_unique<ObjectStream>(this, std::unique_ptr<uint64_t[]>(), 0, 0, entry.ObjFlags, name, base);
	}
}
