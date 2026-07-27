#pragma once

#include "UProperty.h"

class MapPropertyValue
{
public:
	MapPropertyValue() = default;
	MapPropertyValue(const MapPropertyValue& other);
	MapPropertyValue(UProperty* prop, const void* data = nullptr);
	~MapPropertyValue();

	MapPropertyValue& operator=(const MapPropertyValue& other);

	bool operator==(const MapPropertyValue& other) const;
	bool operator<(const MapPropertyValue& other) const;

	UProperty* Property = nullptr;
	void* Data = nullptr;

private:
	void Create(const MapPropertyValue& other);
	void Create(UProperty* prop, const void* data);
	void Destroy();
};

class UMapProperty : public UProperty
{
public:
	using UProperty::UProperty;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;

	void SaveHeader(void* data, PropertyHeader& header) override;
	void SaveValue(void* data, PackageStreamWriter* stream) override;

	typedef std::map<MapPropertyValue, MapPropertyValue> Map;

	size_t ElementAlignment() override;
	size_t ElementSize() override;

	void ConstructElement(void* data) override;
	void CopyConstructElement(void* data, const void* src) override;
	void DestructElement(void* data) override;
	void CopyElement(void* data, const void* src) override;
	bool CompareElement(const void* a, const void* b) override;
	bool CompareLessElement(const void* a, const void* b) override;

	std::string PrintValue(const void* data) override;

	GCAllocation* MarkPropertyElement(GCAllocation* marklist, void* data) override;

	UProperty* Key = nullptr;
	UProperty* Value = nullptr;
};
