#pragma once

#include <file_picker.h>

class CocoaFilePicker : public FilePicker {

private:
    std::string title;
    Mode mode;
    std::vector<std::string> patterns;
    std::string pickedFile;

public:
    void setTitle(std::string const& title) override {
        this->title = title;
    }

    void setMode(Mode mode) override {
        this->mode = mode;
    }

    void setFileNameFilters(std::vector<std::string> const& patterns) override {
        this->patterns = patterns;
    }

    bool show() override;

    std::string getPickedFile() const override {
        return pickedFile;
    }

};
