#ifndef _DATAWRITER_h
#define _DATAWRITER_h

#include "arduino.h"
#include <FS.h>
#include <SD.h>

class DataWriter
{
    protected:
        
    public:
        DataWriter();
        bool cardMount();
        uint64_t getCardSpaceMB();
        uint64_t getUsedSpaceMB();
        uint64_t getCardFreeSpaceMB();
        File createDataFile();
        void appendDataToDataFile(File file, const char* message);
};

#endif /* _DATAWRITER_h */