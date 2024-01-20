#include "file_picker_cocoa.h"

#import <Cocoa/Cocoa.h>

bool CocoaFilePicker::show() {
    NSSavePanel* panel;
    if (mode == FilePicker::Mode::OPEN)
        panel = [NSOpenPanel openPanel];
    else
    panel = [NSSavePanel savePanel];
    panel.title = [NSString stringWithUTF8String: title.c_str()];
    NSMutableArray* types = [NSMutableArray array];
    for (std::string const& pattern : patterns) {
        if (pattern.size() < 2 || pattern[0] != '*' || pattern[1] != '.')
            continue;
        NSString* patternNS = [NSString stringWithUTF8String: &pattern[2]];
        [types addObject: patternNS];
    }
    if ([types count] > 0) {
        panel.allowedFileTypes = types;
        panel.allowsOtherFileTypes = true;
    }
    if ([panel runModal] == NSModalResponseOK) {
        if (![[panel URL] isFileURL])
            return false;
        NSString* dirPathNS = [[panel URL] path];
        pickedFile = std::string([dirPathNS UTF8String], [dirPathNS lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
        return true;
    }
    return false;
}
