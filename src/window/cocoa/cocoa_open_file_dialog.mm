#include "cocoa_open_file_dialog.h"
#import "AppKitWrapper.h"
#import <UniformTypeIdentifiers/UTType.h>


CocoaOpenFileDialog::CocoaOpenFileDialog(DisplayWindow* owner)
{
    panel = (__bridge void*)[NSOpenPanel openPanel];
}

void CocoaOpenFileDialog::SetMultiSelect(bool multiselect)
{
    [((__bridge NSOpenPanel*)panel) setAllowsMultipleSelection:multiselect];
}

void CocoaOpenFileDialog::AddFilter(const std::string &filter_description, const std::string &filter_extension)
{
    _filters.push_back({filter_description, filter_extension});
}

void CocoaOpenFileDialog::SetInitialDirectory(const std::string& path)
{
    [((__bridge NSOpenPanel*)panel) setDirectoryURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:path.c_str()]]];
}

void CocoaOpenFileDialog::SetFilename(const std::string &filename)
{
    [((__bridge NSOpenPanel*)panel) setNameFieldStringValue:[NSString stringWithUTF8String:filename.c_str()]];
}

void CocoaOpenFileDialog::SetDefaultExtension(const std::string& extension)
{
    if (@available(macOS 11.0, *)) {
        NSString* extensionString = [NSString stringWithUTF8String:extension.c_str()];
        UTType* utType = [UTType typeWithFilenameExtension:extensionString];
        if (utType) {
            [((__bridge NSOpenPanel*)panel) setAllowedContentTypes:@[utType]];
        } else {
            // Fallback for unknown extensions
            [((__bridge NSOpenPanel*)panel) setAllowedContentTypes:@[]];
        }
    } else {
        NSString* extensionString = [NSString stringWithUTF8String:extension.c_str()];
        [((__bridge NSOpenPanel*)panel) setAllowedFileTypes:@[extensionString]];
    }
}

void CocoaOpenFileDialog::ClearFilters()
{
    _filters.clear();
    _filterIndex = 0;
}

void CocoaOpenFileDialog::SetFilterIndex(int filter_index)
{
    if (filter_index >= 0 && filter_index < _filters.size())
    {
        _filterIndex = filter_index;
    }
    else
    {
        _filterIndex = 0; // Default to the first filter or no filter
    }
}

void CocoaOpenFileDialog::SetTitle(const std::string &title)
{
    [((__bridge NSOpenPanel*)panel) setTitle:[NSString stringWithUTF8String:title.c_str()]];
}

bool CocoaOpenFileDialog::Show()
{
    if (@available(macOS 11.0, *)) {
        if (!_filters.empty())
        {
            NSArray* fileTypeStrings = [[NSString stringWithUTF8String:_filters[_filterIndex].second.c_str()] componentsSeparatedByString:@";"];
            NSMutableArray<UTType*>* utTypes = [NSMutableArray array];
            for (NSString* typeString in fileTypeStrings) {
                UTType* utType = [UTType typeWithFilenameExtension:typeString];
                if (utType) {
                    [utTypes addObject:utType];
                }
            }
            if ([utTypes count] > 0) {
                [((__bridge NSOpenPanel*)panel) setAllowedContentTypes:utTypes];
            } else {
                // Fallback if no valid UTTypes could be created
                [((__bridge NSOpenPanel*)panel) setAllowedContentTypes:@[]];
            }
        }
        else
        {
            [((__bridge NSOpenPanel*)panel) setAllowedContentTypes:@[]]; // No filters
        }
    } else {
        if (!_filters.empty())
        {
            NSArray* fileTypeStrings = [[NSString stringWithUTF8String:_filters[_filterIndex].second.c_str()] componentsSeparatedByString:@";"];
            [((__bridge NSOpenPanel*)panel) setAllowedFileTypes:fileTypeStrings];
        }
        else
        {
            [((__bridge NSOpenPanel*)panel) setAllowedFileTypes:@[]]; // No filters
        }
    }

    if ([((__bridge NSOpenPanel*)panel) runModal] == NSModalResponseOK)
    {
        _filenames.clear();

        for (NSURL* url in [((__bridge NSOpenPanel*)panel) URLs])
        {
            _filenames.push_back([[url path] UTF8String]);
        }
        return true;
    }
    return false;
}

std::string CocoaOpenFileDialog::Filename() const
{
    if (_filenames.empty())
    {
        return "";
    }
    return _filenames[0];
}

std::vector<std::string> CocoaOpenFileDialog::Filenames() const
{
    return _filenames;
}