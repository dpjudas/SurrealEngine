
#include "Precomp.h"
#include "JsonValue.h"
#include "UTF8Reader.h"
#include <stdexcept>
#include <cstring>

class JsonValueImpl
{
public:
	static void write(const JsonValue &value, std::string &json, int indent);
	static void write_array(const JsonValue &value, std::string &json, int indent);
	static void write_object(const JsonValue &value, std::string &json, int indent);
	static void write_string(const std::string &str, std::string &json);
	static void write_number(const JsonValue &value, std::string &json);
	static void write_indent(std::string& json, int indent);

	static JsonValue read(const std::string &json, size_t &pos);
	static JsonValue read_object(const std::string &json, size_t &pos);
	static JsonValue read_array(const std::string &json, size_t &pos);
	static std::string read_string(const std::string &json, size_t &pos);
	static JsonValue read_number(const std::string &json, size_t &pos);
	static JsonValue read_boolean(const std::string &json, size_t &pos);
	static JsonValue read_null(const std::string &json, size_t &pos);
	static void read_whitespace(const std::string &json, size_t &pos);

	static std::string from_utf32(unsigned int value);
};

std::string JsonValue::to_json(bool formatted) const
{
	std::string result;
	JsonValueImpl::write(*this, result, formatted ? 0 : -1);
	return result;
}

JsonValue JsonValue::parse(const std::string &json)
{
	size_t pos = 0;
	return JsonValueImpl::read(json, pos);
}

void JsonValue::add(const std::string name, const JsonValue& value)
{
	_properties.insert_or_assign(_properties.end(), name, value);
}

/////////////////////////////////////////////////////////////////////////

void JsonValueImpl::write(const JsonValue &value, std::string &json, int indent)
{
	switch (value.type())
	{
	case JsonType::null:
		json += "null";
		break;
	case JsonType::object:
		write_object(value, json, indent);
		break;
	case JsonType::array:
		write_array(value, json, indent);
		break;
	case JsonType::string:
		write_string(value.to_string(), json);
		break;
	case JsonType::number:
		write_number(value, json);
		break;
	case JsonType::boolean:
		json += value.to_boolean() ? "true" : "false";
		break;
	case JsonType::undefined:
		break;
	}
}

void JsonValueImpl::write_array(const JsonValue &value, std::string &json, int indent)
{
	json += "[";
	for (size_t i = 0; i < value.items().size(); i++)
	{
		if (i > 0)
		{
			json += ",";
			if (indent >= 0)
				json.push_back(' ');
		}
		write(value.items()[i], json, indent);
	}
	json += "]";
}

void JsonValueImpl::write_object(const JsonValue &value, std::string &json, int indent)
{
	json += "{";

	if (indent >= 0)
		indent += 2;

	std::map<std::string, JsonValue>::const_iterator it;
	for (it = value.properties().begin(); it != value.properties().end(); ++it)
	{
		if (it != value.properties().begin())
			json += ",";
		write_indent(json, indent);
		write_string(it->first, json);
		json += ":";
		if (indent >= 0)
			json.push_back(' ');
		write(it->second, json, indent);
	}

	if (indent >= 0)
		indent -= 2;

	write_indent(json, indent);
	json += "}";
}

void JsonValueImpl::write_indent(std::string& json, int indent)
{
	if (indent < 0) return;

#ifdef WIN32
	json.push_back('\r');
#endif
	json.push_back('\n');
	json.resize(json.size() + indent, ' ');
}

void JsonValueImpl::write_string(const std::string &str, std::string &json)
{
	json.push_back('"');

	UTF8Reader reader(str.data(), str.size());
	while (!reader.is_end())
	{
		uint32_t c = reader.character();
		if (c == '"' || c == '\\')
		{
			json.push_back('\\');
			json.push_back(c);
		}
		else if (c >= 32 && c <= 127)
		{
			json.push_back(c);
		}
		else if (c == '\b')
		{
			json.push_back('\\');
			json.push_back('b');
		}
		else if (c == '\f')
		{
			json.push_back('\\');
			json.push_back('f');
		}
		else if (c == '\n')
		{
			json.push_back('\\');
			json.push_back('n');
		}
		else if (c == '\r')
		{
			json.push_back('\\');
			json.push_back('r');
		}
		else if (c == '\t')
		{
			json.push_back('\\');
			json.push_back('t');
		}
		else if (c <= 0xffff)
		{
			json.push_back('\\');
			json.push_back('u');
			for (int i = 0; i < 4; i++)
			{
				uint32_t v = (c >> (12 - i * 4)) & 15;
				if (v < 10)
					json.push_back('0' + v);
				else
					json.push_back('a' + (v - 10));
			}
		}
		else
		{
			uint32_t u = c - 0x10000;
			uint32_t w1 = 0xd800 + (u >> 10);
			uint32_t w2 = 0xdc00 + (u & ((1 << 10) - 1));

			json.push_back('\\');
			json.push_back('u');
			for (int i = 0; i < 4; i++)
			{
				uint32_t v = (w1 >> (12 - i * 4)) & 15;
				if (v < 10)
					json.push_back('0' + v);
				else
					json.push_back('a' + (v - 10));
			}
			for (int i = 0; i < 4; i++)
			{
				uint32_t v = (w2 >> (12 - i * 4)) & 15;
				if (v < 10)
					json.push_back('0' + v);
				else
					json.push_back('a' + (v - 10));
			}
		}

		reader.next();
	}

	json.push_back('"');
}

void JsonValueImpl::write_number(const JsonValue &value, std::string &json)
{
	if (static_cast<double>(static_cast<int>(value.to_number())) == value.to_number())
	{
		json += std::to_string((int)value.to_number());
	}
	else
	{
		json += std::to_string(value.to_number());
	}
}

JsonValue JsonValueImpl::read(const std::string &json, size_t &pos)
{
	read_whitespace(json, pos);

	if (pos == json.length())
		throw std::runtime_error("Unexpected end of JSON data");

	switch (json[pos])
	{
	case '{':
		return read_object(json, pos);
	case '[':
		return read_array(json, pos);
	case '"':
		return JsonValue::string(read_string(json, pos));
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		return read_number(json, pos);
	case 'f':
	case 't':
		return read_boolean(json, pos);
	case 'n':
		return read_null(json, pos);
	default:
		throw std::runtime_error("Unexpected character in JSON data");
	}
}

JsonValue JsonValueImpl::read_object(const std::string &json, size_t &pos)
{
	JsonValue result = JsonValue::object();

	pos++;

	read_whitespace(json, pos);

	if (pos == json.length())
		throw std::runtime_error("Unexpected end of JSON data");

	while (pos != json.length() && json[pos] != '}')
	{
		std::string key = read_string(json, pos);

		read_whitespace(json, pos);

		if (pos == json.length())
			throw std::runtime_error("Unexpected end of JSON data");
		else if (json[pos] != ':')
			throw std::runtime_error("Unexpected character in JSON data");
		pos++;

		read_whitespace(json, pos);

		result.prop(key) = read(json, pos);

		read_whitespace(json, pos);

		if (pos == json.length())
		{
			throw std::runtime_error("Unexpected end of JSON data");
		}
		else if (json[pos] == '}')
		{
			break;
		}
		else if (json[pos] == ',')
		{
			pos++;
			read_whitespace(json, pos);
		}
		else
		{
			throw std::runtime_error("Unexpected character in JSON data");
		}
	}
	pos++;

	return result;
}

JsonValue JsonValueImpl::read_array(const std::string &json, size_t &pos)
{
	JsonValue result = JsonValue::array();

	pos++;

	read_whitespace(json, pos);

	if (pos == json.length())
		throw std::runtime_error("Unexpected end of JSON data");

	while (json[pos] != ']')
	{
		read_whitespace(json, pos);
		result.items().push_back(read(json, pos));
		read_whitespace(json, pos);

		if (pos == json.length())
		{
			throw std::runtime_error("Unexpected end of JSON data");
		}
		else if (json[pos] == ']')
		{
			break;
		}
		else if (json[pos] == ',')
		{
			pos++;
			read_whitespace(json, pos);
		}
		else
		{
			throw std::runtime_error("Unexpected character in JSON data");
		}
	}
	pos++;

	return result;
}

std::string JsonValueImpl::read_string(const std::string &json, size_t &pos)
{
	pos++;
	if (pos == json.length())
		throw std::runtime_error("Unexpected end of JSON data");

	std::string result;
	while (true)
	{
		if (pos == json.length())
		{
			throw std::runtime_error("Unexpected end of JSON data");
		}
		else if (json[pos] == '"')
		{
			break;
		}
		else if (json[pos] == '\\')
		{
			pos++;
			if (pos == json.length())
				throw std::runtime_error("Unexpected end of JSON data");

			unsigned codepoint;
			switch (json[pos])
			{
			case '"':
				result.push_back('"');
				break;
			case '\\':
				result.push_back('\\');
				break;
			case '/':
				result.push_back('/');
				break;
			case 'b':
				result.push_back('\b');
				break;
			case 'f':
				result.push_back('\f');
				break;
			case 'n':
				result.push_back('\n');
				break;
			case 'r':
				result.push_back('\r');
				break;
			case 't':
				result.push_back('\t');
				break;
			case 'u':
				if (pos + 5 > json.length())
					throw std::runtime_error("Unexpected end of JSON data");

				codepoint = 0;
				for (int i = 0; i < 4; i++)
				{
					char c = json[pos + 1 + i];
					if (c >= '0' && c <= '9')
					{
						codepoint <<= 4;
						codepoint |= c - '0';
					}
					else if (c >= 'a' && c <= 'f')
					{
						codepoint <<= 4;
						codepoint |= c - 'a' + 10;
					}
					else if (c >= 'A' && c <= 'F')
					{
						codepoint <<= 4;
						codepoint |= c - 'A' + 10;
					}
					else
					{
						throw std::runtime_error("Invalid unicode escape");
					}
				}
				result += from_utf32(codepoint);
				pos += 4;
				break;
			}
			pos++;
		}
		else
		{
			result.push_back(json[pos]);
			pos++;
		}
	}

	pos++;

	return result;
}

JsonValue JsonValueImpl::read_number(const std::string &json, size_t &pos)
{
	size_t start_pos = pos;
	if (json[pos] == '-')
		pos++;
	while (pos < json.length() && json[pos] >= '0' && json[pos] <= '9')
		pos++;
	if (pos != json.length() && json[pos] == '.')
		pos++;
	while (pos < json.length() && json[pos] >= '0' && json[pos] <= '9')
		pos++;
	if (pos != json.length() && (json[pos] == 'e' || json[pos] == 'E'))
	{
		pos++;
		if (pos != json.length() && (json[pos] == '+' || json[pos] == '-'))
			pos++;
		while (pos < json.length() && json[pos] >= '0' && json[pos] <= '9')
			pos++;
	}
	size_t end_pos = pos;

	std::string number_string = json.substr(start_pos, end_pos - start_pos);
	if (number_string.empty())
		throw std::runtime_error("Unexpected character in JSON data");

	double result = std::atof(number_string.c_str());
	return JsonValue::number(result);
}

JsonValue JsonValueImpl::read_boolean(const std::string &json, size_t &pos)
{
	if (json[pos] == 't')
	{
		if (pos + 4 > json.length() || memcmp(&json[pos], "true", 4) != 0)
			throw std::runtime_error("Unexpected character in JSON data");
		pos += 4;
		return JsonValue::boolean(true);
	}
	else
	{
		if (pos + 5 > json.length() || memcmp(&json[pos], "false", 5) != 0)
			throw std::runtime_error("Unexpected character in JSON data");
		pos += 5;
		return JsonValue::boolean(false);
	}
}

JsonValue JsonValueImpl::read_null(const std::string &json, size_t &pos)
{
	if (pos + 4 > json.length() || memcmp(&json[pos], "null", 4) != 0)
		throw std::runtime_error("Unexpected character in JSON data");
	pos += 4;
	return JsonValue::null();
}

void JsonValueImpl::read_whitespace(const std::string &json, size_t &pos)
{
	while (pos != json.length() && (json[pos] == ' ' || json[pos] == '\r' || json[pos] == '\n' || json[pos] == '\t' || json[pos] == '\f'))
		pos++;
}

std::string JsonValueImpl::from_utf32(unsigned int value)
{
	char text[8];

	if ((value < 0x80) && (value > 0))
	{
		text[0] = (char)value;
		text[1] = 0;
	}
	else if (value < 0x800)
	{
		text[0] = (char)(0xc0 | (value >> 6));
		text[1] = (char)(0x80 | (value & 0x3f));
		text[2] = 0;
	}
	else if (value < 0x10000)
	{
		text[0] = (char)(0xe0 | (value >> 12));
		text[1] = (char)(0x80 | ((value >> 6) & 0x3f));
		text[2] = (char)(0x80 | (value & 0x3f));
		text[3] = 0;

	}
	else if (value < 0x200000)
	{
		text[0] = (char)(0xf0 | (value >> 18));
		text[1] = (char)(0x80 | ((value >> 12) & 0x3f));
		text[2] = (char)(0x80 | ((value >> 6) & 0x3f));
		text[3] = (char)(0x80 | (value & 0x3f));
		text[4] = 0;

	}
	else if (value < 0x4000000)
	{
		text[0] = (char)(0xf8 | (value >> 24));
		text[1] = (char)(0x80 | ((value >> 18) & 0x3f));
		text[2] = (char)(0x80 | ((value >> 12) & 0x3f));
		text[3] = (char)(0x80 | ((value >> 6) & 0x3f));
		text[4] = (char)(0x80 | (value & 0x3f));
		text[5] = 0;

	}
	else if (value < 0x80000000)
	{
		text[0] = (char)(0xfc | (value >> 30));
		text[1] = (char)(0x80 | ((value >> 24) & 0x3f));
		text[2] = (char)(0x80 | ((value >> 18) & 0x3f));
		text[3] = (char)(0x80 | ((value >> 12) & 0x3f));
		text[4] = (char)(0x80 | ((value >> 6) & 0x3f));
		text[5] = (char)(0x80 | (value & 0x3f));
		text[6] = 0;
	}
	else
	{
		text[0] = 0;	// Invalid wchar value
	}
	return text;
}
