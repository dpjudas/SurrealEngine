
#include "core/theme.h"
#include "core/widget.h"
#include "core/canvas.h"
#include "core/font.h"
#include "theme_stylesheet_tokenizer.h"
#include "theme_stylesheet_document.h"
#include "theme_default_stylesheet.h"
#include "border_image_renderer.h"
#include <stdexcept>

WidgetStyle::WidgetStyle(WidgetStyle* parentStyle) : ParentStyle(parentStyle)
{
}

void WidgetStyle::SetBool(const std::string& state, const std::string& propertyName, bool value)
{
	StyleProperties[state][propertyName] = value;
}

void WidgetStyle::SetInt(const std::string& state, const std::string& propertyName, int value)
{
	StyleProperties[state][propertyName] = value;
}

void WidgetStyle::SetDouble(const std::string& state, const std::string& propertyName, double value)
{
	StyleProperties[state][propertyName] = value;
}

void WidgetStyle::SetString(const std::string& state, const std::string& propertyName, const std::string& value)
{
	StyleProperties[state][propertyName] = value;
}

void WidgetStyle::SetColor(const std::string& state, const std::string& propertyName, const Colorf& value)
{
	StyleProperties[state][propertyName] = value;
}

void WidgetStyle::SetImage(const std::string& state, const std::string& propertyName, const std::shared_ptr<Image>& value)
{
	StyleProperties[state][propertyName] = value;
}

const WidgetStyle::PropertyVariant* WidgetStyle::FindProperty(const std::string& state, const std::string& propertyName) const
{
	const WidgetStyle* style = this;
	do
	{
		// Look for property in the specific state
		auto stateIt = style->StyleProperties.find(state);
		if (stateIt != style->StyleProperties.end())
		{
			auto it = stateIt->second.find(propertyName);
			if (it != stateIt->second.end())
				return &it->second;
		}

		// Fall back to the widget main style
		if (state != std::string())
		{
			stateIt = style->StyleProperties.find(std::string());
			if (stateIt != style->StyleProperties.end())
			{
				auto it = stateIt->second.find(propertyName);
				if (it != stateIt->second.end())
					return &it->second;
			}
		}

		style = style->ParentStyle;
	} while (style);
	return nullptr;
}

std::shared_ptr<Font> WidgetStyle::GetFont(const std::string& state)
{
	auto& font = Fonts[state];
	if (!font)
	{
		std::string fontName = GetString(state, "font-family");
		double size = GetDouble(state, "font-size");
		font = Font::Create(fontName, size);
	}
	return font;
}

bool WidgetStyle::GetBool(const std::string& state, const std::string& propertyName) const
{
	const PropertyVariant* prop = FindProperty(state, propertyName);
	return prop ? std::get<bool>(*prop) : false;
}

int WidgetStyle::GetInt(const std::string& state, const std::string& propertyName) const
{
	const PropertyVariant* prop = FindProperty(state, propertyName);
	return prop ? std::get<int>(*prop) : 0;
}

double WidgetStyle::GetDouble(const std::string& state, const std::string& propertyName) const
{
	const PropertyVariant* prop = FindProperty(state, propertyName);
	return prop ? std::get<double>(*prop) : 0.0;
}

std::string WidgetStyle::GetString(const std::string& state, const std::string& propertyName) const
{
	const PropertyVariant* prop = FindProperty(state, propertyName);
	return prop ? std::get<std::string>(*prop) : std::string();
}

Colorf WidgetStyle::GetColor(const std::string& state, const std::string& propertyName) const
{
	const PropertyVariant* prop = FindProperty(state, propertyName);
	return prop ? std::get<Colorf>(*prop) : Colorf::transparent();
}

std::shared_ptr<Image> WidgetStyle::GetImage(const std::string& state, const std::string& propertyName) const
{
	const PropertyVariant* prop = FindProperty(state, propertyName);
	return prop ? std::get<std::shared_ptr<Image>>(*prop) : std::shared_ptr<Image>();
}

/////////////////////////////////////////////////////////////////////////////

void BasicWidgetStyle::Paint(Widget* widget, Canvas* canvas, Size size)
{
	Colorf bgcolor = widget->GetStyleColor("background-color");
	if (bgcolor.a > 0.0f)
		canvas->fillRect(Rect::xywh(0.0, 0.0, size.width, size.height), bgcolor);

	Colorf borderleft = widget->GetStyleColor("border-left-color");
	Colorf bordertop = widget->GetStyleColor("border-top-color");
	Colorf borderright = widget->GetStyleColor("border-right-color");
	Colorf borderbottom = widget->GetStyleColor("border-bottom-color");

	double borderwidth = widget->GridFitSize(1.0);

	if (bordertop.a > 0.0f)
		canvas->fillRect(Rect::xywh(0.0, 0.0, size.width, borderwidth), bordertop);
	if (borderbottom.a > 0.0f)
		canvas->fillRect(Rect::xywh(0.0, size.height - borderwidth, size.width, borderwidth), borderbottom);
	if (borderleft.a > 0.0f)
		canvas->fillRect(Rect::xywh(0.0, 0.0, borderwidth, size.height), borderleft);
	if (borderright.a > 0.0f)
		canvas->fillRect(Rect::xywh(size.width - borderwidth, 0.0, borderwidth, size.height), borderright);

	auto image = widget->GetStyleImage("border-image-source");
	if (image)
	{
		BorderGeometry geo;
		geo.box = Rect::xywh(0.0, 0.0, size.width, size.height);
		geo.border.left = widget->GetStyleDouble("border-left-width");
		geo.border.right = widget->GetStyleDouble("border-right-width");
		geo.border.top = widget->GetStyleDouble("border-top-width");
		geo.border.bottom = widget->GetStyleDouble("border-bottom-width");

		BorderImage style;
		style.source = image;
		style.width.left = widget->GetStyleDouble("border-left-image-width");
		style.width.top = widget->GetStyleDouble("border-top-image-width");
		style.width.right = widget->GetStyleDouble("border-right-image-width");
		style.width.bottom = widget->GetStyleDouble("border-bottom-image-width");
		style.slice.left = BorderImageValue(widget->GetStyleDouble("border-left-image-slice"), BorderImageValueType::number);
		style.slice.top = BorderImageValue(widget->GetStyleDouble("border-top-image-slice"), BorderImageValueType::number);
		style.slice.right = BorderImageValue(widget->GetStyleDouble("border-right-image-slice"), BorderImageValueType::number);
		style.slice.bottom = BorderImageValue(widget->GetStyleDouble("border-bottom-image-slice"), BorderImageValueType::number);
		style.slice.center = widget->GetStyleBool("border-center-image-slice");

		BorderImageRenderer::render(canvas, geo, style);
	}
}

/////////////////////////////////////////////////////////////////////////////

static std::unique_ptr<WidgetTheme> CurrentTheme;

WidgetStyle* WidgetTheme::RegisterStyle(std::unique_ptr<WidgetStyle> widgetStyle, const std::string& widgetClass)
{
	auto& style = Styles[widgetClass];
	style = std::move(widgetStyle);
	return style.get();
}

WidgetStyle* WidgetTheme::GetStyle(const std::string& widgetClass)
{
	auto it = Styles.find(widgetClass);
	if (it != Styles.end())
		return it->second.get();
	if (widgetClass != "widget")
		return GetStyle("widget");
	return nullptr;
}

void WidgetTheme::SetTheme(std::unique_ptr<WidgetTheme> theme)
{
	CurrentTheme = std::move(theme);
}

WidgetTheme* WidgetTheme::GetTheme()
{
	return CurrentTheme.get();
}

/////////////////////////////////////////////////////////////////////////////

DarkWidgetTheme::DarkWidgetTheme() : StylesheetTheme(theme_default_stylesheet, "dark")
{
}

/////////////////////////////////////////////////////////////////////////////

LightWidgetTheme::LightWidgetTheme() : StylesheetTheme(theme_default_stylesheet, "light")
{
}

/////////////////////////////////////////////////////////////////////////////

StylesheetTheme::StylesheetTheme(const std::string& stylesheet, const std::string& usedThemeClass)
{
	ThemeStylesheetDocument doc(stylesheet);

	std::unordered_map<std::string, WidgetStyle*> styles;
	std::unordered_map<std::string, std::vector<ThemeStylesheetToken>> variables;
	auto widget = RegisterStyle(std::make_unique<BasicWidgetStyle>(), "widget");
	styles["widget"] = widget;

	for (const ThemeSortedSelector& s : doc.selectors)
	{
		if (!s.selector->theme.empty() && s.selector->theme != usedThemeClass)
			continue; // Not this theme

		if (s.selector->widget == "*")
		{
			// Only support variable declarations in universal selectors for now
			for (const ThemeStyleProperty& prop : s.ruleset->properties)
			{
				if (prop.name.size() > 2 && prop.name.substr(0, 2) == "--")
				{
					variables[prop.name] = prop.value;
				}
			}
		}
		else
		{
			std::string widgetName = s.selector->widget;
			std::string partName = s.selector->state;

			WidgetStyle* style = styles[widgetName];
			if (!style)
			{
				style = RegisterStyle(std::make_unique<BasicWidgetStyle>(widget), widgetName);
				styles[widgetName] = style;
			}

			for (const ThemeStyleProperty& prop : s.ruleset->properties)
			{
				if (prop.name.size() > 2 && prop.name.substr(0, 2) == "--")
					continue; // skip variable declarations

				const std::string& name = prop.name;

				// Grab property value tokens
				std::vector<ThemeStylesheetToken> origtokens = prop.value;

				// Insert variables (to do: move to function and clean this up!)
				std::vector<ThemeStylesheetToken> tokens;
				for (auto it = origtokens.begin(); it != origtokens.end(); ++it)
				{
					const ThemeStylesheetToken& t = *it;
					if (t.type == ThemeStylesheetTokenType::function && t.value == "var")
					{
						++it;
						if (it == origtokens.end() || (*it).type != ThemeStylesheetTokenType::delim || (*it).value != "-")
						{
							tokens.clear();
							break;
						}

						++it;
						if (it == origtokens.end() || (*it).type != ThemeStylesheetTokenType::ident)
						{
							tokens.clear();
							break;
						}
						const auto& variable = variables["-" + (*it).value];
						tokens.insert(tokens.end(), variable.begin(), variable.end());
						++it;
						if (it == origtokens.end() || (*it).type != ThemeStylesheetTokenType::bracket_end)
						{
							tokens.clear();
							break;
						}
					}
					else
					{
						tokens.push_back(t);
					}
				}

				if (tokens.empty())
					continue;

				// Parse the tokens:
				// To do: maybe use property parsers like ClanLib and UICore does?
				// That would allow for short form properties

				if (tokens[0].type == ThemeStylesheetTokenType::ident && (tokens[0].value == "true" || tokens[0].value == "false"))
				{
					if (tokens[0].value == "true")
					{
						style->SetBool(partName, name, true);
					}
					else if (tokens[0].value == "false")
					{
						style->SetBool(partName, name, false);
					}
				}
				else if (tokens[0].type == ThemeStylesheetTokenType::number)
				{
					style->SetDouble(partName, name, std::atof(tokens[0].value.c_str()));
				}
				else if (tokens.size() == 2 && tokens[0].type == ThemeStylesheetTokenType::delim && tokens[0].value == "-" && tokens[1].type == ThemeStylesheetTokenType::number)
				{
					style->SetDouble(partName, name, -std::atof(tokens[1].value.c_str()));
				}
				else if (tokens[0].type == ThemeStylesheetTokenType::string)
				{
					style->SetString(partName, name, tokens[0].value);
				}
				else if (tokens[0].type == ThemeStylesheetTokenType::uri)
				{
					style->SetImage(partName, name, Image::LoadResource(tokens[0].value));
				}
				else
				{
					size_t pos = 0;
					Colorf color;
					if (ThemeStylesheetDocument::parse_color(tokens, pos, color))
					{
						style->SetColor(partName, name, color);
					}
				}
			}
		}
	}
}
