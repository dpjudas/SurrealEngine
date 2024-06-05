
#include "systemdialogs/save_file_dialog.h"
#include "window/window.h"

std::unique_ptr<SaveFileDialog> SaveFileDialog::Create(Widget* owner)
{
	return DisplayBackend::Get()->CreateSaveFileDialog(owner);
}
