#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// change this to match your SD shield or module;
//     Arduino Ethernet shield: pin 4
//     Adafruit SD shields and modules: pin 10
//     Sparkfun SD shield: pin 8
const int chipSelect = 4;

// temporary display array variable
static uint8_t tempFrameMem[1024];


/*  */
unsigned int hexToDec(String hexString)
{
	unsigned int decValue = 0;
	int nextInt;

	for (int i = 0; i < hexString.length(); i++)
	{
		// 
		nextInt = int(hexString.charAt(i));
		if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
		if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
		if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
		nextInt = constrain(nextInt, 0, 15);

		// 
		decValue = (decValue * 16) + nextInt;
	}

	return decValue;
}

/* Get BMP data from file in SD card */
uint16_t GetBMPData(uint8_t *dstArr, uint16_t nCount, String fileName)
{
	//
	File myFile;
	// 
	uint16_t nReadCount = 0;

	// Maximum count of array.
	uint16_t nMax = nCount;
	// Open file in SD card.
	myFile = SD.open(fileName);

	if (!myFile)
	{ // If file does not exist or can't open file.
		Serial.print(" File open error, ");
		Serial.println(fileName);
		return 0;
	}

	// Read buffer from file and store data to buffe
	while (nCount > 0 && myFile.available())
	{ // 
		String temp = myFile.readStringUntil(',');
		
		// end on file
		if (temp.indexOf("};") != -1)
			temp = temp.substring(0, temp.indexOf("\r\n"));
		// 
		uint8_t nVal = hexToDec(temp);
		dstArr[nMax - nCount] = nVal;
		nCount--;
		nReadCount++;
	}
	myFile.close();
	return nReadCount;
}

bool convertDir(String HexDir, String BinDir)
{
	Serial.print("Source directory is ");
	Serial.println(HexDir);

	Serial.print("Destination directory is ");
	Serial.println(BinDir);

	File hexDir = SD.open(HexDir);
	if (!hexDir)
	{
		Serial.println("There is no hex directory in SD.");
		return false;
	}
	File binDir = SD.open(BinDir);
	if (!binDir)
	{
		Serial.println("There is no bin directory in SD, I will create directory");
		binDir.close();
		if (!SD.mkdir(BinDir))
		{
			Serial.println("I can't create directory!");
			return false;
		}
	}
	
	while (true)
	{

		File entry = hexDir.openNextFile();
		if (!entry) {
			// no more files
			break;
		}

		String fileName = entry.name();
		if (!entry.isDirectory())
		{
			uint16_t nReadCount = GetBMPData(tempFrameMem, 1024, (HexDir + "/") + fileName);
			Serial.print((HexDir + "/") + fileName);
			Serial.print(", PixelCount-");
			Serial.print(nReadCount);
			Serial.print(", ");

			if (nReadCount)
			{
				String binFileName = fileName.substring(0, fileName.length() - 4);
				binFileName = (BinDir + "/") + binFileName;
				binFileName += ".bin";
				Serial.println(binFileName);
				File binFile = SD.open(binFileName, FILE_WRITE);
				binFile.write(tempFrameMem, 1024);
				binFile.close();
			}
			Serial.println();
		}
		entry.close();
	}
	return true;
}

void setup()
{
	// set the baud rate of serial port as 9600
	Serial.begin(9600);

	delay(2000);

	Serial.print("Initializing SD card...");
	pinMode(SS, OUTPUT);

	if (!SD.begin(chipSelect)) {
		Serial.println("initialization failed!");
		return;
	}

	Serial.println("initialization done.");

	convertDir("/hex/1", "/bin/1");
	convertDir("/hex/2", "/bin/2");
	convertDir("/hex/3", "/bin/3");
	Serial.println("Converting is done.");

}



void loop()
{

}