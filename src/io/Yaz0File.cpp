#include "io/Yaz0File.h"

#include "filetypes/Yaz0.h"
#include "io/ExternalFile.h"

Yaz0File::Yaz0File(QString filePath)
        : MemoryFile(), m_backend(ExternalFile(filePath))
{
    m_contents = Yaz0::decompress(m_backend.getContents());
}

void Yaz0File::save()
{
    QByteArray compBuffer = Yaz0::compress(m_contents);


    close();
}
