
#include "systemdialogs/open_folder_dialog.h"
#include "window/window.h"

std::unique_ptr<OpenFolderDialog> OpenFolderDialog::Create(Widget* owner)
{
	return DisplayBackend::Get()->CreateOpenFolderDialog(owner);
}
