#include "DataWriter.h"

DataWriter::DataWriter()
{

}

bool DataWriter::cardMount()
{
    if (!SD.begin()) 
    {
        Serial.println("Card Mount Failed");
        return false;
    }

    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) 
    {
        Serial.println("No SD card attached");
        return false;
    }

    return true;
}

uint64_t DataWriter::getCardSpaceMB()
{
    return SD.cardSize() / (1024 * 1024);
}

uint64_t DataWriter::getUsedSpaceMB()
{
    return SD.usedBytes() / (1024 * 1024);
}

uint64_t DataWriter::getCardFreeSpaceMB()
{
    return ((SD.totalBytes() - SD.usedBytes()) / (1024 * 1024));
}

File DataWriter::createDataFile()
{
    //check directory
    const char* directory = "/data";
    if(!SD.exists(directory))
    {
        Serial.printf("Create direcory \"%s\"\r\n", directory);
        if(!SD.mkdir(directory))
        {
            Serial.printf("Direcory not created \"%s\"\r\n", directory);
        }
    }

    int countFile = 0;
    File data = SD.open(directory);
    while(data.openNextFile() != NULL)
    {
        countFile++;
    }

    String fileName = "/data_" + String(countFile) + ".txt";
    String filePath = String(directory) + fileName;
    Serial.printf("Date file creating \"%s\"\r\n", filePath.c_str());

    File file = SD.open(filePath, FILE_WRITE);

    Serial.printf("Date file created \"%s\"\r\n", filePath.c_str());
    return file;
}

void DataWriter::appendDataToDataFile(File file, const char* message)
{
    if (!file) 
    {
        Serial.println("Failed to open file for appending");
        return;
    }
    if (file.println(message)) 
    {
        Serial.println("Message appended");
    }
    else 
    {
        Serial.println("Append failed");
    }
}



