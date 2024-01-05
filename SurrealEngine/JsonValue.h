
#pragma once

#include <map>
#include <string>
#include <vector>

enum class JsonType
{
	undefined,
	null,
	object,
	array,
	number,
	boolean,
	string
};

class JsonValue
{
public:
	static JsonValue undefined() { JsonValue v; v._type = JsonType::undefined; return v; }
	static JsonValue null() { JsonValue v; v._type = JsonType::null; return v; }
	static JsonValue object() { JsonValue v; v._type = JsonType::object; return v; }
	static JsonValue array() { JsonValue v; v._type = JsonType::array; return v; }
	static JsonValue array(std::vector<JsonValue> items) { JsonValue v = array(); v._items = std::move(items); return v; }
	static JsonValue array(const std::vector<double>& items) { JsonValue v = array(); v._items.reserve(items.size()); for (const auto& i : items) v._items.push_back(JsonValue::number(i)); return v; }
	static JsonValue array(const std::vector<float>& items) { JsonValue v = array(); v._items.reserve(items.size()); for (const auto& i : items) v._items.push_back(JsonValue::number(i)); return v; }
	static JsonValue array(const std::vector<int>& items) { JsonValue v = array(); v._items.reserve(items.size()); for (const auto& i : items) v._items.push_back(JsonValue::number(i)); return v; }
	static JsonValue array(const std::vector<unsigned int>& items) { JsonValue v = array(); v._items.reserve(items.size()); for (const auto& i : items) v._items.push_back(JsonValue::number(i)); return v; }
	static JsonValue array(const std::vector<short>& items) { JsonValue v = array(); v._items.reserve(items.size()); for (const auto& i : items) v._items.push_back(JsonValue::number(i)); return v; }
	static JsonValue array(const std::vector<unsigned short>& items) { JsonValue v = array(); v._items.reserve(items.size()); for (const auto& i : items) v._items.push_back(JsonValue::number(i)); return v; }
	static JsonValue array(const std::vector<char>& items) { JsonValue v = array(); v._items.reserve(items.size()); for (const auto& i : items) v._items.push_back(JsonValue::number(i)); return v; }
	static JsonValue array(const std::vector<unsigned char>& items) { JsonValue v = array(); v._items.reserve(items.size()); for (const auto& i : items) v._items.push_back(JsonValue::number(i)); return v; }
	static JsonValue array(const std::vector<bool>& items) { JsonValue v = array(); v._items.reserve(items.size()); for (const auto& i : items) v._items.push_back(JsonValue::boolean(i)); return v; }
	static JsonValue array(const std::vector<std::string>& items) { JsonValue v = array(); v._items.reserve(items.size()); for (const auto& i : items) v._items.push_back(JsonValue::string(i)); return v; }
	static JsonValue number(double value) { JsonValue v; v._type = JsonType::number; v._number = value; return v; }
	static JsonValue number(float value) { return JsonValue::number(static_cast<double>(value)); }
	static JsonValue number(int value) { return JsonValue::number(static_cast<double>(value)); }
	static JsonValue number(unsigned int value) { return JsonValue::number(static_cast<double>(value)); }
	static JsonValue number(short value) { return JsonValue::number(static_cast<double>(value)); }
	static JsonValue number(unsigned short value) { return JsonValue::number(static_cast<double>(value)); }
	static JsonValue number(char value) { return JsonValue::number(static_cast<double>(value)); }
	static JsonValue number(unsigned char value) { return JsonValue::number(static_cast<double>(value)); }
	static JsonValue boolean(bool value) { JsonValue v; v._type = JsonType::boolean; v._boolean = value; return v; }
	static JsonValue string(const std::string &value) { JsonValue v; v._type = JsonType::string; v._string = value; return v; }

	static JsonValue parse(const std::string &json);
	std::string to_json() const;

	const JsonValue &prop(const std::string &name) const { auto it = _properties.find(name); if (it != _properties.end()) return it->second; static JsonValue undef; return undef; }
	const JsonValue &prop(const char *name) const { auto it = _properties.find(name); if (it != _properties.end()) return it->second; static JsonValue undef; return undef; }
	JsonValue &prop(const std::string &name) { return _properties[name]; }
	JsonValue &prop(const char *name) { return _properties[name]; }

	void add(const std::string name, const JsonValue& value);

	void remove(const std::string &name) { auto it = _properties.find(name); if (it != _properties.end()) _properties.erase(it); }
	void remove(const char *name) { auto it = _properties.find(name); if (it != _properties.end()) _properties.erase(it); }

	size_t size() const { return _items.size(); }
	JsonValue &at(size_t index) { return _items.at(index); }
	const JsonValue &at(size_t index) const { return _items.at(index); }
	void erase(size_t offset, size_t length) { _items.erase(_items.begin() + offset, _items.begin() + offset + length); }

	void clear() { _items.clear(); _properties.clear(); }

	JsonType type() const { return _type; }
	bool is_undefined() const { return type() == JsonType::undefined; }
	bool is_null() const { return type() == JsonType::null; }
	bool is_object() const { return type() == JsonType::object; }
	bool is_array() const { return type() == JsonType::array; }
	bool is_number() const { return type() == JsonType::number; }
	bool is_boolean() const { return type() == JsonType::boolean; }

	std::map<std::string, JsonValue> &properties() { return _properties; }
	const std::map<std::string, JsonValue> &properties() const { return _properties; }

	std::vector<JsonValue> &items() { return _items; }
	const std::vector<JsonValue> &items() const { return _items; }

	double to_number() const { return _number; }
	bool to_boolean() const { return _boolean; }
	const std::string &to_string() const { return _string; }

	double to_double() const { return to_number(); }
	float to_float() const { return static_cast<float>(to_number()); }
	int to_int() const { return static_cast<int>(to_number()); }
	unsigned int to_uint() const { return static_cast<unsigned int>(to_number()); }
	short to_short() const { return static_cast<short>(to_number()); }
	unsigned short to_ushort() const { return static_cast<unsigned short>(to_number()); }
	char to_char() const { return static_cast<char>(to_number()); }
	unsigned char to_uchar() const { return static_cast<unsigned char>(to_number()); }

	void set_undefined() { *this = JsonValue::undefined(); }
	void set_null() { *this = JsonValue::null(); }
	void set_object() { *this = JsonValue::object(); }
	void set_array() { *this = JsonValue::array(); }
	void set_string(const std::string &v) { *this = JsonValue::string(v); }
	void set_string(const char *v) { *this = JsonValue::string(v); }
	void set_number(double v) { *this = JsonValue::number(v); }
	void set_number(float v) { *this = JsonValue::number(v); }
	void set_number(int v) { *this = JsonValue::number(v); }
	void set_number(unsigned int v) { *this = JsonValue::number(v); }
	void set_number(short v) { *this = JsonValue::number(v); }
	void set_number(unsigned short v) { *this = JsonValue::number(v); }
	void set_number(char v) { *this = JsonValue::number(v); }
	void set_number(unsigned char v) { *this = JsonValue::number(v); }
	void set_boolean(bool v) { *this = JsonValue::boolean(v); }

	JsonValue &operator[](const std::string &name) { return prop(name); }
	JsonValue &operator[](const char *name) { return prop(name); }
	const JsonValue &operator[](const std::string &name) const { return prop(name); }
	const JsonValue &operator[](const char *name) const { return prop(name); }

	JsonValue &operator[](size_t index) { return at(index); }
	const JsonValue &operator[](size_t index) const { return at(index); }

private:
	JsonType _type = JsonType::undefined;
	std::vector<JsonValue> _items;
	std::map<std::string, JsonValue> _properties;
	std::string _string;
	double _number = 0.0;
	bool _boolean = false;
};
