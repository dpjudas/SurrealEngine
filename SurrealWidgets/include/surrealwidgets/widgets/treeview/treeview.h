#pragma once

#include "../../core/widget.h"
#include <vector>

class TreeViewHeader;
class TreeViewBody;
class TreeNode;
class Scrollbar;

class TreeView : public Widget
{
public:
    TreeView(Widget* parent = nullptr);

    void ClearColumns();
    void SetColumn(int index, const std::string& text, double width);
    void ShowHeader(bool value);

    int GetColumnCount() const;
    double GetColumnWidth(int index) const;
    const std::string& GetColumnText(int index) const;

    void AddItem(std::initializer_list<std::string> columns, int index = -1, bool asChildOf = false);
    void AddItem(const std::string& text, int index = -1, int column = 0, bool asChildOf = false);
    void UpdateItem(const std::string& text, int index, int column = 0);
    void RemoveItem(int index = -1);

    std::unique_ptr<TreeNode>& GetChildItem(int index) const;
    size_t GetItemCount() const;

    int GetSelectedItem() const { return m_SelectedItem; }
    void SetSelectedItem(int index, bool notify = true);

    void ScrollToItem(int index);

    double GetPreferredWidth() override;
    double GetPreferredHeight() override;
    double GetMinimumHeight();

    void SetRootNode(TreeNode* newNode) { m_RootNode = newNode; }

    void Activate();

    std::function<void(int)> OnChanged;
    std::function<void()> OnActivated;

protected:
    void OnGeometryChanged() override;
    void OnKeyDown(InputKey key) override;
    void OnScrollbarScroll();

private:
    double GetHeaderHeight() const;
    void RebuildVisibleNodes();

    TreeViewHeader* m_Header = nullptr;
    TreeViewBody* m_Body = nullptr;
    Scrollbar* m_Scrollbar = nullptr;
    TreeNode* m_RootNode = nullptr;
    int m_SelectedItem = 0;
    std::vector<TreeNode*> m_VisibleNodes; // Flat representation of the currently visible nodes

    friend class TreeViewBody;
};

class TreeViewBody : public Widget
{
public:
    TreeViewBody(TreeView* parent);

    static double GetItemHeight();

private:
    void OnPaint(Canvas* canvas) override;
    bool OnMouseDown(const Point& pos, InputKey key) override;
    bool OnMouseDoubleclick(const Point& pos, InputKey key) override;
    bool OnMouseWheel(const Point& pos, InputKey key) override;
    void OnKeyDown(InputKey key) override;

    TreeView* m_TreeView = nullptr;
};

class TreeViewHeader : public Widget
{
public:
    TreeViewHeader(TreeView* parent);

    void OnPaint(Canvas* canvas) override;

    void Clear();
    void SetColumn(int index, const std::string& text, double width);

    size_t GetColumnCount() const { return m_Columns.size(); }
    double GetColumnWidth(const int index) const { return m_Columns[index].width; }
    const std::string& GetColumnText(const int index) const { return m_Columns[index].text; }

    double GetPreferredHeight() override;

private:
    struct Column
    {
        std::string text;
        double width = 0.0;
    };

    std::vector<Column> m_Columns;
};

class TreeNode
{
public:
    TreeNode(std::initializer_list<std::string> columns = {}, TreeNode* parentNode = nullptr);
    void AddChildNode(std::unique_ptr<TreeNode>& node);
    void AddChildNode(std::initializer_list<std::string> columns = {});
    void RemoveChildNode(const TreeNode* node);
    std::unique_ptr<TreeNode>& GetChildNode(const int index) { return m_ChildNodes[index]; }
    std::vector<std::unique_ptr<TreeNode>>& GetChildNodes() { return m_ChildNodes; }
    std::vector<TreeNode*> GetVisibleChildNodes();
    uint32_t GetDepth() const { return m_Depth; }
    bool HasChildNodes() const { return !m_ChildNodes.empty(); }
    bool GetChildrenVisible() const { return m_ChildrenVisible; }
    void SetChildrenVisible(const bool value) { m_ChildrenVisible = value; }
    void ToggleChildrenVisible() { m_ChildrenVisible = !m_ChildrenVisible; }
    TreeNode* GetParentNode() const { return m_ParentNode; }
    size_t GetVisibleItemCount() const;
    size_t GetChildrenCount() const { return m_ChildNodes.size(); }

    std::vector<std::string> m_Columns;
private:
    std::vector<std::unique_ptr<TreeNode>> m_ChildNodes;
    TreeNode* m_ParentNode = nullptr;
    int32_t m_Depth = -1;
    bool m_ChildrenVisible = false;
};