#pragma once

#include "UI/Core/View.h"
#include "UI/Controls/TextLabel/TextLabel.h"
#include "UI/Controls/ImageBox/ImageBox.h"
#include "ListViewItemView.h"
#include <list>
#include <memory>

class View;
class ListView;
class ListViewItemView;

class ListViewItem
{
public:
	ListViewItem();
	virtual ~ListViewItem();

	ListViewItem* add(std::unique_ptr<ListViewItem> item);
	ListViewItem* insertBefore(std::unique_ptr<ListViewItem> item, ListViewItem* sibling);
	std::unique_ptr<ListViewItem> remove();
	void removeAllChildren();

	void open();
	void close();
	bool isOpen() const { return openFlag; }

	const std::string& id() const { return itemId; }
	void setId(std::string id) { itemId = std::move(id); }

	const ListViewItem* root() const;
	ListViewItem* parent() const { return parentObj; }
	ListViewItem* prevSibling() const { return prevSiblingObj; }
	ListViewItem* nextSibling() const { return nextSiblingObj; }
	ListViewItem* firstChild() const { return firstChildObj; }
	ListViewItem* lastChild() const { return lastChildObj; }

	ListViewItem* prevOpenItem() const;
	ListViewItem* nextOpenItem() const;

	ListViewItemView* itemview() const { return view; }

	virtual ListView* listview() const { const ListViewItem* r = root(); return (r != this) ? r->listview() : nullptr; }

	void updateColumn(size_t index);

	void sort();

protected:
	virtual std::string sortCompareString() { return id(); }
	virtual void updateColumnView(size_t index) = 0;

private:
	ListViewItem* parentObj = nullptr;
	ListViewItem* prevSiblingObj = nullptr;
	ListViewItem* nextSiblingObj = nullptr;
	ListViewItem* firstChildObj = nullptr;
	ListViewItem* lastChildObj = nullptr;

	std::string itemId;
	ListViewItemView* view = nullptr;
	bool openFlag = true;

	friend class ListView;
};

class RootListViewItem : public ListViewItem
{
public:
	RootListViewItem(ListView* listview) : listviewObj(listview) { }

	ListView* listview() const override { return listviewObj; }

protected:
	std::string sortCompareString() override { return {}; }
	void updateColumnView(size_t index) override { }

private:
	ListView* listviewObj = nullptr;
};

class TextListViewItem : public ListViewItem
{
public:
	TextListViewItem() { }
	TextListViewItem(std::string id) { setId(id); }
	TextListViewItem(std::string id, std::vector<std::string> columns) : columns(std::move(columns)) { setId(std::move(id)); }
	TextListViewItem(std::string id, std::string icon, std::vector<std::string> columns) : columns(std::move(columns)) { setIcon(0, std::move(icon)); setId(std::move(id)); }
	TextListViewItem(std::vector<std::string> columns) : columns(std::move(columns)) { }

	void setIcon(size_t index, std::string src)
	{
		if (index >= icons.size())
			icons.resize(index + 1);
		icons[index] = std::move(src);
		updateColumn(index);
	}

	void setText(size_t index, std::string text)
	{
		if (index >= columns.size())
			columns.resize(index + 1);
		columns[index] = text;
		updateColumn(index);
	}

	std::string getIcon(size_t index)
	{
		if (index < icons.size())
			return icons[index];
		else
			return {};
	}

	std::string getText(size_t index)
	{
		if (index < columns.size())
			return columns[index];
		else
			return {};
	}

protected:
	void updateColumnView(size_t index) override
	{
		itemview()->setColumnView(index, createTextColumn(getIcon(index), getText(index)));
	}

	std::string sortCompareString() override
	{
		if (!columns.empty())
			return columns.front();
		else
			return {};
	}

private:
	View* createTextColumn(const std::string& icon, const std::string& text)
	{
		if (!icon.empty())
		{
			auto view = new HBoxView(itemview());
			auto image = new ImageBox(view);
			image->setSrc(icon);
			image->addClass("listviewitemicon");
			auto label = new TextLabel(view);
			label->setText(text);
			return view;
		}
		else
		{
			auto view = new TextLabel(itemview());
			view->setText(text);
			return view;
		}
	}

	std::vector<std::string> icons;
	std::vector<std::string> columns;
};

class HeadlineListViewItem : public ListViewItem
{
public:
	HeadlineListViewItem(std::string headline) : headline(std::move(headline)) { }
	HeadlineListViewItem(std::string id, std::string headline) : headline(std::move(headline)) { setId(std::move(id)); }
	HeadlineListViewItem(std::string id, std::string icon, std::string headline) : icon(std::move(icon)), headline(std::move(headline)) { setId(std::move(id)); }

protected:
	void updateColumnView(size_t index) override
	{
		if (index == 0)
		{
			itemview()->setColumnView(index, createHeadlineColumn(icon, headline));
		}
	}

	std::string sortCompareString() override
	{
		return headline;
	}

private:
	View* createHeadlineColumn(const std::string& icon, const std::string& text)
	{
		if (!icon.empty())
		{
			auto view = new HBoxView(itemview());
			view->addClass("listviewheadline");
			auto image = new ImageBox(view);
			image->setSrc(icon);
			image->addClass("listviewitemicon");
			auto label = new TextLabel(view);
			label->setText(text);
			return view;
		}
		else
		{
			auto view = new TextLabel(itemview());
			view->setText(text);
			view->addClass("listviewheadline");
			return view;
		}
	}

	std::string icon;
	std::string headline;
};
