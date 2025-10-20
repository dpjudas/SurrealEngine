#include "cocoa_save_file_dialog.h"
#import "AppKitWrapper.h"
#import <UniformTypeIdentifiers/UTType.h>

CocoaSaveFileDialog::CocoaSaveFileDialog(DisplayWindow* owner)
{
    panel = (__bridge void*)[NSSavePanel savePanel];
}

void CocoaSaveFileDialog::AddFilter(const std::string &filter_description, const std::string &filter_extension)
{
    NSArray* fileTypeStrings = [[NSString stringWithUTF8String:filter_extension.c_str()] componentsSeparatedByString:@";"];
    NSMutableArray<UTType*>* utTypes = [NSMutableArray array];
    for (NSString* typeString in fileTypeStrings) {
        UTType* utType = [UTType typeWithFilenameExtension:typeString];
        if (utType) {
            [utTypes addObject:utType];
        }
    }
    if ([utTypes count] > 0) {
        [((__bridge NSSavePanel*)panel) setAllowedContentTypes:utTypes];
    } else {
        // Fallback if no valid UTTypes could be created
        [((__bridge NSSavePanel*)panel) setAllowedFileTypes:fileTypeStrings];
    }
}

void CocoaSaveFileDialog::SetInitialDirectory(const std::string& path)
{
    [((__bridge NSSavePanel*)panel) setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:path.c_str()]]];
}



void CocoaSaveFileDialog::SetDefaultExtension(const std::string& extension)
{
    NSString* extensionString = [NSString stringWithUTF8String:extension.c_str()];
    UTType* utType = [UTType typeWithFilenameExtension:extensionString];
    if (utType) {
        [((__bridge NSSavePanel*)panel) setAllowedContentTypes:@[utType]];
    } else {
        // Fallback for unknown extensions or older macOS versions if UTType is not available
        [((__bridge NSSavePanel*)panel) setAllowedFileTypes:@[extensionString]];
    }
}

bool CocoaSaveFileDialog::Show()
{
    if ([((__bridge NSSavePanel*)panel) runModal] == NSModalResponseOK)
    {
        _filename = [[[(__bridge NSSavePanel*)panel URL] path] UTF8String];
        return true;
    }
    return false;
}

std::string CocoaSaveFileDialog::Filename() const
{
    return _filename;
}

void CocoaSaveFileDialog::SetFilename(const std::string &filename)
{
    [((__bridge NSSavePanel*)panel) setNameFieldStringValue:[NSString stringWithUTF8String:filename.c_str()]];
}

void CocoaSaveFileDialog::ClearFilters()
{
    [((__bridge NSSavePanel*)panel) setAllowedContentTypes:@[]];
}

void CocoaSaveFileDialog::SetFilterIndex(int filter_index)
{
    // NSSavePanel does not have a direct equivalent for setting a filter by index.
    // Filters are set via setAllowedFileTypes.
    // For now, this will be a no-op.
}

void CocoaSaveFileDialog::SetTitle(const std::string &title)
{
    [((__bridge NSSavePanel*)panel) setTitle:[NSString stringWithUTF8String:title.c_str()]];
}