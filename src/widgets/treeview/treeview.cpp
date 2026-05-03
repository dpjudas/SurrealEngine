#include "widgets/treeview/treeview.h"
#include "widgets/scrollbar/scrollbar.h"

TreeView::TreeView(Widget* parent) : Widget(parent)
{
    SetStretching(true);
    SetStyleClass("listview");

    m_Scrollbar = new Scrollbar(this);
    m_Scrollbar->FuncScroll = [this]() { OnScrollbarScroll(); };

    m_Header = new TreeViewHeader(this);
    m_Header->SetVisible(false);

    m_Body = new TreeViewBody(this);
    m_RootNode = new TreeNode();
    m_RootNode->SetChildrenVisible(true); // Root Node's children must always be visible
}

void TreeView::ClearColumns()
{
    if (m_Header->GetColumnCount() != 0)
    {
        m_Header->Clear();
        m_Header->SetVisible(false);
        OnGeometryChanged();
        Update();
    }
}

void TreeView::SetColumn(int index, const std::string& text, double width)
{
    m_Header->SetColumn(index, text, width);
    OnGeometryChanged();
    Update();
}

void TreeView::ShowHeader(bool value)
{
    if (m_Header->IsVisible() != value)
    {
        m_Header->SetVisible(value);
        OnGeometryChanged();
        Update();
    }
}

void TreeView::AddItem(std::initializer_list<std::string> columns, int index, bool asChildOf)
{
    auto item = std::make_unique<TreeNode>(columns);

    if (index < 0)
        m_RootNode->AddChildNode(item);
    else if (asChildOf)
        m_VisibleNodes[index]->AddChildNode(item);
    else
        m_VisibleNodes[index]->GetParentNode()->AddChildNode(item);

    RebuildVisibleNodes();
    m_Scrollbar->SetRanges(m_Body->GetHeight(), m_VisibleNodes.size() * m_Body->GetItemHeight());
    Update();
}

void TreeView::AddItem(const std::string& text, int index, int column, bool asChildOf)
{
    auto item = std::make_unique<TreeNode>();
    item->m_Columns.resize(column + 1);
    item->m_Columns[column] = text;

    if (index < 0)
        m_RootNode->AddChildNode(item);
    else if (asChildOf)
        m_VisibleNodes[index]->AddChildNode(item);
    else
        m_VisibleNodes[index]->GetParentNode()->AddChildNode(item);

    RebuildVisibleNodes();
    m_Scrollbar->SetRanges(m_Body->GetHeight(), m_VisibleNodes.size() * m_Body->GetItemHeight());
    Update();
}

int TreeView::GetColumnCount() const
{
    return m_Header->GetColumnCount();
}

double TreeView::GetColumnWidth(const int index) const
{
    if (index >= 0 && index < m_Header->GetColumnCount())
        return m_Header->GetColumnWidth(index);
    return 0.0;
}

const std::string& TreeView::GetColumnText(const int index) const
{
    if (index >= 0 && index < m_Header->GetColumnCount())
        return m_Header->GetColumnText(index);
    static std::string empty;
    return empty;
}

void TreeView::UpdateItem(const std::string& text, int index, int column)
{
    if (index < 0 || (size_t)index >= m_VisibleNodes.size() || column < 0)
        return;

    TreeNode* item = m_VisibleNodes[index];
    item->m_Columns.resize(std::max((size_t)column + 1, item->m_Columns.size()));
    item->m_Columns[column] = text;

    Update();
}

void TreeView::RemoveItem(int index)
{
    if (m_VisibleNodes.empty() || index >= (int)m_VisibleNodes.size())
        return;

    if (index < 0)
        index = static_cast<int>(m_VisibleNodes.size()) - 1;

    if (m_SelectedItem == index)
        SetSelectedItem(0);

    if (m_VisibleNodes[index]->GetParentNode())
        m_VisibleNodes[index]->GetParentNode()->RemoveChildNode(m_VisibleNodes[index]);

    m_VisibleNodes.erase(m_VisibleNodes.begin() + index);
    m_Scrollbar->SetRanges(m_Body->GetHeight(), m_VisibleNodes.size() * m_Body->GetItemHeight());
    Update();
}

std::unique_ptr<TreeNode>& TreeView::GetChildItem(const int index) const
{
    return m_RootNode->GetChildNode(index);
}

size_t TreeView::GetItemCount() const
{
    return m_RootNode->GetVisibleItemCount();
}

void TreeView::SetSelectedItem(int index, bool notify)
{
    if (m_SelectedItem != index && index >= 0 && index < (int)m_VisibleNodes.size())
    {
        m_SelectedItem = index;
        Update();
        if (notify && OnChanged)
            OnChanged(m_SelectedItem);
    }
}

void TreeView::ScrollToItem(int index)
{
    double itemHeight = m_Body->GetItemHeight();
    double y = itemHeight * index;
    if (y < m_Scrollbar->GetPosition())
    {
        m_Scrollbar->SetPosition(y);
    }
    else if (y + itemHeight > m_Scrollbar->GetPosition() + GetHeight())
    {
        m_Scrollbar->SetPosition(std::max(y + itemHeight - GetHeight(), 0.0));
    }
}

double TreeView::GetPreferredWidth()
{
    double total = 0.0;

    if (m_Header->GetColumnCount() != 0)
    {
        for (int i = 0, count = m_Header->GetColumnCount(); i < count; i++)
        {
            total += m_Header->GetColumnWidth(i);
        }
    }
    else
    {
        auto canvas = GetCanvas();
        auto font = GetFont();
        for (const auto& row : m_VisibleNodes)
        {
            double wRow = 0.0;
            for (const auto& cell : row->m_Columns)
            {
                wRow += canvas->measureText(font, cell).width;
            }
            total = std::max(wRow, total);
        }
    }

    return total + 10.0*2 + m_Scrollbar->GetPreferredWidth();
}

double TreeView::GetPreferredHeight()
{
    return m_VisibleNodes.size() * 20.0 + 10.0 * 2; // Items plus top/bottom padding
}

double TreeView::GetMinimumHeight()
{
    return 20.0 + 10.0 * 2; // One item plus top/bottom padding
}

void TreeView::Activate()
{
    if (m_SelectedItem >= 0 && m_VisibleNodes[m_SelectedItem]->HasChildNodes())
    {
        m_VisibleNodes[m_SelectedItem]->ToggleChildrenVisible();
        RebuildVisibleNodes();
    }
    else if (OnActivated)
        OnActivated();
}

void TreeView::OnGeometryChanged()
{
    double w = GetWidth();
    double h = GetHeight();
    double hh = GetHeaderHeight();
    double sw = m_Scrollbar->GetPreferredWidth();
    m_Header->SetFrameGeometry(Rect::xywh(0.0, 0.0, w - sw, hh));
    m_Body->SetFrameGeometry(Rect::xywh(0.0, hh, w - sw, h - hh));
    m_Scrollbar->SetFrameGeometry(Rect::xywh(w - sw, 0.0, sw, h));
    m_Scrollbar->SetRanges(m_Body->GetHeight(), m_VisibleNodes.size() * m_Body->GetItemHeight());
}

void TreeView::OnKeyDown(InputKey key)
{
    if (key == InputKey::Down)
    {
        if (m_SelectedItem + 1 < (int)m_VisibleNodes.size())
        {
            SetSelectedItem(m_SelectedItem + 1);
        }
        ScrollToItem(m_SelectedItem);
    }
    else if (key == InputKey::Up)
    {
        if (m_SelectedItem > 0)
        {
            SetSelectedItem(m_SelectedItem - 1);
        }
        ScrollToItem(m_SelectedItem);
    }
    else if (key == InputKey::Enter)
    {
        Activate();
    }
    else if (key == InputKey::Home)
    {
        if (m_SelectedItem > 0)
        {
            SetSelectedItem(0);
        }
        ScrollToItem(m_SelectedItem);
    }
    else if (key == InputKey::End)
    {
        if (m_SelectedItem + 1 < (int)m_VisibleNodes.size())
        {
            SetSelectedItem((int)m_VisibleNodes.size() - 1);
        }
        ScrollToItem(m_SelectedItem);
    }
    else if (key == InputKey::PageUp)
    {
        double h = GetHeight();
        if (h <= 0.0 || m_VisibleNodes.empty())
            return;
        int itemsPerPage = (int)std::max(std::round(h / m_Body->GetItemHeight()), 1.0);
        int nextItem = std::max(m_SelectedItem - itemsPerPage, 0);
        if (nextItem != m_SelectedItem)
        {
            SetSelectedItem(nextItem);
            ScrollToItem(m_SelectedItem);
        }
    }
    else if (key == InputKey::PageDown)
    {
        double h = GetHeight();
        if (h <= 0.0 || m_VisibleNodes.empty())
            return;
        int itemsPerPage = (int)std::max(std::round(h / m_Body->GetItemHeight()), 1.0);
        int prevItem = std::min(m_SelectedItem + itemsPerPage, (int)m_VisibleNodes.size() - 1);
        if (prevItem != m_SelectedItem)
        {
            SetSelectedItem(prevItem);
            ScrollToItem(m_SelectedItem);
        }
    }
}

void TreeView::OnScrollbarScroll()
{
    Update();
}

double TreeView::GetHeaderHeight() const
{
    if (m_Header->IsVisible())
        return m_Header->GetPreferredHeight() + m_Header->GetNoncontentTop() + m_Header->GetNoncontentBottom();

    return 0.0;
}

void TreeView::RebuildVisibleNodes()
{
    m_VisibleNodes = m_RootNode->GetVisibleChildNodes();
    Update();
}

///////////////////////////////////////////////////

TreeViewBody::TreeViewBody(TreeView* parent) : Widget(parent), m_TreeView(parent)
{
	SetStyleClass("listview-body");
}

double TreeViewBody::GetItemHeight()
{
	return 20.0;
}

void TreeViewBody::OnKeyDown(InputKey key)
{
	m_TreeView->OnKeyDown(key);
}

void TreeViewBody::OnPaint(Canvas* canvas)
{
	double x = 2.0;
	double w = GetWidth();
	double itemHeight = GetItemHeight();

	Colorf textColor = GetStyleColor("color");
	Colorf selectionColor = GetStyleColor("selection-color");
	auto font = GetFont();

	double y = -m_TreeView->m_Scrollbar->GetPosition();

	int itemIndex = 0;
	for (const auto& item : m_TreeView->m_VisibleNodes)
	{
		double itemY = y;
	    double notchPadding = 15.0; // +/- "icon"
	    double leftPadding = item->GetDepth() * 4.0;
	    bool hasChildren = item->HasChildNodes();
	    bool childrenVisible = item->GetChildrenVisible();
		if (itemY + itemHeight >= 0.0 && itemY < GetHeight())
		{
			if (itemIndex == m_TreeView->m_SelectedItem)
			{
				canvas->fillRect(Rect::xywh(x - 2.0, itemY, w, itemHeight), selectionColor);
			}
			double cx = x;
			int colCount = std::min(item->m_Columns.size(), m_TreeView->m_Header->GetColumnCount());
			for (int colIndex = 0; colIndex < colCount; ++colIndex)
			{
				double colwidth = m_TreeView->m_Header->GetColumnWidth(colIndex);
				if (colIndex + 1 == m_TreeView->m_Header->GetColumnCount())
					colwidth = std::max(w - cx, 0.0);
				canvas->pushClip(Rect::xywh(cx, itemY, std::max(colwidth - 5.0, 0.0), itemHeight));
			    if (colIndex == 0)
			    {
			        if (hasChildren)
			            canvas->drawText(font, Point(cx, y + 15.0), childrenVisible ? "-" : "+", textColor);
			        canvas->drawText(font, Point(cx + notchPadding + leftPadding, y + 15.0), item->m_Columns[colIndex], textColor);
			    }
			    else
				    canvas->drawText(font, Point(cx, y + 15.0), item->m_Columns[colIndex], textColor);
				canvas->popClip();
				cx += colwidth;
			}
		}
		y += itemHeight;
		itemIndex++;
	}
}

bool TreeViewBody::OnMouseDown(const Point& pos, InputKey key)
{
	SetFocus();

	if (key == InputKey::LeftMouse)
	{
		int index = (int)((pos.y + m_TreeView->m_Scrollbar->GetPosition()) / GetItemHeight());
		if (index >= 0 && (size_t)index < m_TreeView->m_VisibleNodes.size())
		{
			m_TreeView->ScrollToItem(index);
			m_TreeView->SetSelectedItem(index);
		    // Expand item if the "notch" is pressed
		    const auto currentItem = m_TreeView->m_VisibleNodes[index];
		    if (currentItem->HasChildNodes() && pos.x > currentItem->GetDepth() * 4.0 && pos.x <= currentItem->GetDepth() * 4.0 + 15.0)
		        m_TreeView->Activate();
		}
	}
	return true;
}

bool TreeViewBody::OnMouseDoubleclick(const Point& pos, InputKey key)
{
	if (key == InputKey::LeftMouse)
	{
	    int index = (int)((pos.y + m_TreeView->m_Scrollbar->GetPosition()) / GetItemHeight());
	    if (index == m_TreeView->m_SelectedItem)
	        m_TreeView->Activate();
	}
	return true;
}

bool TreeViewBody::OnMouseWheel(const Point& pos, InputKey key)
{
	if (key == InputKey::MouseWheelUp)
	{
		m_TreeView->m_Scrollbar->SetPosition(std::max(m_TreeView->m_Scrollbar->GetPosition() - GetItemHeight(), 0.0));
	}
	else if (key == InputKey::MouseWheelDown)
	{
		m_TreeView->m_Scrollbar->SetPosition(std::min(m_TreeView->m_Scrollbar->GetPosition() + GetItemHeight(), m_TreeView->m_Scrollbar->GetMax()));
	}
	return true;
}

///////////////////////////////////////////////////

TreeViewHeader::TreeViewHeader(TreeView* parent) : Widget(parent)
{
    SetStyleClass("listview-header");
    Clear();
}

void TreeViewHeader::Clear()
{
    m_Columns.clear();
    m_Columns.push_back({});
}

void TreeViewHeader::SetColumn(int index, const std::string& text, double width)
{
    if (index < 0)
        return;
    if (m_Columns.size() <= (size_t)index)
        m_Columns.resize(index + 1);

    Column col;
    col.text = text;
    col.width = width;
    m_Columns[index] = std::move(col);
}

void TreeViewHeader::OnPaint(Canvas* canvas)
{
    Colorf textColor = GetStyleColor("color");
    auto font = GetFont();
    double cx = 0.0;
    for (size_t idx = 0; idx < m_Columns.size(); idx++)
    {
        canvas->drawText(font, Point(cx, 15.0), m_Columns[idx].text, textColor);
        cx += m_Columns[idx].width;
    }
}

double TreeViewHeader::GetPreferredHeight()
{
    return 20.0;
}

///////////////////////////////////////////////////

TreeNode::TreeNode(const std::initializer_list<std::string> columns, TreeNode* parentNode)
    : m_Columns(columns), m_ParentNode(parentNode)
{
    if (m_ParentNode)
        m_Depth = m_ParentNode->m_Depth + 1;
}

void TreeNode::AddChildNode(std::unique_ptr<TreeNode>& node)
{
    node->m_ParentNode = this;
    node->m_Depth = m_Depth + 1;
    m_ChildNodes.push_back(std::move(node));
}

void TreeNode::AddChildNode(std::initializer_list<std::string> columns)
{
    auto newNode = std::make_unique<TreeNode>(columns, this);
    m_ChildNodes.push_back(std::move(newNode));
}

void TreeNode::RemoveChildNode(const TreeNode* node)
{
    for (auto it = m_ChildNodes.cbegin() ; it != m_ChildNodes.cend() ; ++it)
    {
        if (it->get() == node)
            m_ChildNodes.erase(it);
    }
}

size_t TreeNode::GetVisibleItemCount() const
{
    size_t result = m_ParentNode ? 1 : 0; // Count yourself if you're not the topmost node (meaning you should be always invisible)

    if (m_ChildrenVisible)
    {
        for (const auto& it : m_ChildNodes)
            result += it->GetVisibleItemCount();
    }

    return result;
}

std::vector<TreeNode*> TreeNode::GetVisibleChildNodes()
{
    std::vector<TreeNode*> result;

    if (m_ParentNode != nullptr)
        result.push_back(this);

    if (m_ChildrenVisible)
    {
        for (const auto& child: m_ChildNodes)
        {
            auto child_items = child->GetVisibleChildNodes();

            for (const auto it : child_items)
                result.push_back(it);
        }
    }

    return result;
}
