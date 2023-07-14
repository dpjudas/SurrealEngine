#pragma once

//typedef std::string NameString;

class NameString
{
public:
	NameString() { Value = "None"; }
	NameString(const char* str) : NameString(std::string(str)) { }
	NameString(const std::string& value) : Value(!value.empty() ? value : std::string("None")) { }
	NameString(const NameString& other) = default;
	NameString& operator=(const NameString&) = default;

	int Compare(const NameString& other) const
	{
		size_t len = std::min(Value.size(), other.Value.size());
		for (size_t i = 0; i < len; i++)
		{
			int a = (uint8_t)Value[i];
			int b = (uint8_t)other.Value[i];

			if ((a >= 'A' && a <= 'Z'))
				a = a - 'A' + 'a';

			if ((b >= 'A' && b <= 'Z'))
				b = b - 'A' + 'a';

			if (a != b)
				return a - b;
		}

		if (Value.size() < other.Value.size())
			return -1;
		else if (Value.size() > other.Value.size())
			return 1;
		else
			return 0;
	}

	bool IsNone() const { return *this == NameString(); }
	const std::string& ToString() const { return Value; }

	bool operator==(const char* other) const { return *this == NameString(other); }
	bool operator==(const std::string& other) const { return *this == NameString(other); }
	bool operator!=(const char* other) const { return *this != NameString(other); }
	bool operator!=(const std::string& other) const { return *this != NameString(other); }

	bool operator==(const NameString& other) const { return Value.size() != other.Value.size() ? false : Compare(other) == 0; }
	bool operator!=(const NameString& other) const { return Value.size() != other.Value.size() ? true : Compare(other) != 0; }
	bool operator<(const NameString& other) const { return Compare(other) < 0; }
	bool operator>(const NameString& other) const { return Compare(other) > 0; }
	bool operator<=(const NameString& other) const { return Compare(other) <= 0; }
	bool operator>=(const NameString& other) const { return Compare(other) >= 0; }

	std::string Value;
};