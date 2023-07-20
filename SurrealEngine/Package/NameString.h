#pragma once

//typedef std::string NameString;

class NameString
{
public:
	NameString() : Value("None") {}
	NameString(const char* str) : Value(str) { }
	NameString(const std::string& value) : Value(!value.empty() ? value : std::string("None")) { }
	NameString(const NameString& other) = default;
	NameString& operator=(const NameString&) = default;

	int Compare(const NameString& other) const
	{
		return Compare(other.Value.data(), other.Value.size());
	}

	int Compare(const char* other, size_t othersize) const
	{
		size_t len = std::min(Value.size(), othersize);
		for (size_t i = 0; i < len; i++)
		{
			int a = (uint8_t)Value[i];
			int b = (uint8_t)other[i];

			if ((a >= 'A' && a <= 'Z'))
				a = a - 'A' + 'a';

			if ((b >= 'A' && b <= 'Z'))
				b = b - 'A' + 'a';

			if (a != b)
				return a - b;
		}

		if (Value.size() < othersize)
			return -1;
		else if (Value.size() > othersize)
			return 1;
		else
			return 0;
	}

	bool IsNone() const { return *this == NameString(); }
	const std::string& ToString() const { return Value; }

	bool operator==(const char* other) const { size_t othersize = strlen(other); return Value.size() != othersize ? false : Compare(other, othersize) == 0; }
	bool operator==(const std::string& other) const { return Value.size() != other.size() ? false : Compare(other.data(), other.size()) == 0; }
	bool operator!=(const char* other) const { size_t othersize = strlen(other); return Value.size() != othersize ? true : Compare(other, othersize) != 0; }
	bool operator!=(const std::string& other) const { return Value.size() != other.size() ? true : Compare(other.data(), other.size()) != 0; }

	bool operator==(const NameString& other) const { return Value.size() != other.Value.size() ? false : Compare(other) == 0; }
	bool operator!=(const NameString& other) const { return Value.size() != other.Value.size() ? true : Compare(other) != 0; }
	bool operator<(const NameString& other) const { return Compare(other) < 0; }
	bool operator>(const NameString& other) const { return Compare(other) > 0; }
	bool operator<=(const NameString& other) const { return Compare(other) <= 0; }
	bool operator>=(const NameString& other) const { return Compare(other) >= 0; }

	std::string Value;
};
