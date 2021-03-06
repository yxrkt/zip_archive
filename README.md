## Overview

C++ library for reading files from a zip archive in memory.

## API
**zip_archive(std::istream& stream)**

*Constructs the zip_archive and moves to the first file.*

**void reset()**

*Resets the position to the first file.*

**bool seek(const char\* file_name)**

*Seeks to the file specified (full path)*

**read(std::string& file_name, std::stringstream& file_data)**

*Reads the name and contents of the current file and advances to the next file.*

**eof()**

*True once the last file in the archive has been reached.*

## Examples
```cpp
// Find a file and read its contents
void read_contents(std::istream& stream)
{
    zip_archive archive(stream);

    if (archive.seek("root/src/index.html"))
    {
        std::string name;
        std::stringstream content;
        archive.read(name, content);

        std::cout << content.str() << std::endl;
    }
}

// Iterate through files
void iterate_flies(std::istream& stream)
{
    zip_archive archive(stream);

    while (!archive.eof())
    {
        std::string name;
        std::string content;
        archive.read(name, content);
        std::cout << name << std::endl;
    }
}
```
