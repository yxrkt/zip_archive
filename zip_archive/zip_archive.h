#pragma once

#include <iosfwd>
#include <string>

typedef void* unzFile;

class zip_archive
{
public:
    zip_archive(std::istream& stream);
    zip_archive(const zip_archive&) = delete;
    zip_archive(zip_archive&&) = delete;
    ~zip_archive();

    void reset();
    bool seek(const char* file_name);
    void read(std::string& file_name, std::stringstream& file_data);
    bool eof() const;

private:
    std::istream& m_stream;
    unzFile m_fileHandle = nullptr;
    bool m_eof = false;
};