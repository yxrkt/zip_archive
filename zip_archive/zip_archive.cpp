#include "zip_archive.h"

#include "minizip\unzip.h"

#include <cassert>
#include <istream>
#include <sstream>

zip_archive::zip_archive(std::istream& stream)
    : m_stream(stream)
{
    zlib_filefunc64_def zlib_calbacks;

    zlib_calbacks.opaque = this;

    zlib_calbacks.zopen64_file = [](voidpf opaque, const void* /*filename*/, int /*mode*/) -> voidpf
    {
        auto self = static_cast<zip_archive*>(opaque);
        return &self->m_stream;
    };

    zlib_calbacks.zread_file = [](voidpf /*opaque*/, voidpf stream, void* buf, uLong size)
    {
        auto istream = static_cast<std::istream*>(stream);
        istream->read(static_cast<char*>(buf), size);
        return (uLong)istream->gcount();
    };

    zlib_calbacks.zwrite_file = [](voidpf /*opaque*/, voidpf /*stream*/, const void* /*buf*/, uLong /*size*/)
    {
        assert(false && "read only");
        return 0ul;
    };

    zlib_calbacks.ztell64_file = [](voidpf /*opaque*/, voidpf stream) -> ZPOS64_T
    {
        auto istream = static_cast<std::istream*>(stream);
        return istream->tellg();
    };

    zlib_calbacks.zseek64_file = [](voidpf /*opaque*/, voidpf stream, ZPOS64_T offset, int origin) -> long
    {
        auto istream = static_cast<std::istream*>(stream);

        switch (origin)
        {
        case ZLIB_FILEFUNC_SEEK_CUR:
            istream->seekg(offset, std::ios::cur);
            return 0;
        case ZLIB_FILEFUNC_SEEK_END:
            istream->seekg(offset, std::ios::end);
            return 0;
        case ZLIB_FILEFUNC_SEEK_SET:
            istream->seekg(offset, std::ios::beg);
            return 0;
        }

        return -1;
    };

    zlib_calbacks.zclose_file = [](voidpf /*opaque*/, voidpf /*stream*/)
    {
        return 0;
    };

    zlib_calbacks.zerror_file = [](voidpf /*opaque*/, voidpf /*stream*/)
    {
        return 0;
    };

    m_fileHandle = unzOpen2_64(nullptr, &zlib_calbacks);
}

zip_archive::~zip_archive()
{
    unzClose(m_fileHandle);
}

void zip_archive::reset()
{
    unzGoToFirstFile(m_fileHandle);
    m_eof = false;
}

bool zip_archive::seek(const char* file_name)
{
    if (unzLocateFile(m_fileHandle, file_name, 1 /*case-sensitive*/) == UNZ_OK)
    {
        m_eof = false;
        return true;
    }

    return false;
}

void zip_archive::read(std::string& name, std::stringstream& data)
{
    size_t file_size = 0;
    do
    {
        if (unzOpenCurrentFile(m_fileHandle) != UNZ_OK)
        {
            break;
        }

        unz_file_info64 file_info;
        char name_buffer[512];
        if (unzGetCurrentFileInfo64(m_fileHandle, &file_info, name_buffer, sizeof(name_buffer), nullptr, 0, nullptr, 0) != UNZ_OK)
        {
            break;
        }

        file_size = file_info.uncompressed_size;
        if (file_size > 0)
        {
            std::string bytes(file_size, 0);
            unzReadCurrentFile(m_fileHandle, &bytes[0], (uLong)file_size);

            name.assign(name_buffer);
            data << bytes;
        }

        unzCloseCurrentFile(m_fileHandle);

        if (unzGoToNextFile(m_fileHandle) == UNZ_END_OF_LIST_OF_FILE)
        {
            m_eof = true;
            break;
        }
    } while (file_size == 0);
}

bool zip_archive::eof() const
{
    return m_eof;
}
