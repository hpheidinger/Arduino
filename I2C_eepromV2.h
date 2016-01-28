#ifndef I2C_EEPROM_H
#define I2C_EEPROM_H
//
//    FILE: I2C_eeprom.h
//  AUTHOR: Rob Tillaart
// PURPOSE: I2C_eeprom library for Arduino with EEPROM 24LC256 et al.
// VERSION: 1.2.03
// HISTORY: See I2C_eeprom.cpp
//     URL: http://arduino.cc/playground/Main/LibraryForI2CEEPROM
//
// Released to the public domain
//

#include <Wire.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#include "Wstring.h"
#include "Wiring.h"
#endif

#define I2C_EEPROM_VERSION "2.0.0b"

// TWI buffer needs max 2 bytes for eeprom address
// 1 byte for eeprom register address is available in txbuffer
#define I2C_TWIBUFFERSIZE	30

// to break blocking read/write after n millis()
#define I2C_EEPROM_TIMEOUT	1000


class I2C_eeprom {
//-------------------------------------
//	Public space
//-------------------------------------
public:

    //
    // Constructor 	... Use for ATMELs 24xx and compatibles
    //
    /**
     * Initializes the EEPROM for the given device address.
     *
     * It will try to guess page size and address word size based on the size of the device.
     * 
     * @param deviceAddress Byte address of the device.
     * @param deviceSize    Max size in bytes of the device (divide your device size in Kbits by 8)
     */
    I2C_eeprom(const uint8_t deviceAddress, const unsigned int deviceSize);


    //
    // Prototypes
    //
    uint8_t	get_deviceAddress(void);
    int		get_deviceSize(void);		// resembles removed/old: 'determinesize'; see also get_Kbytes()
    int		get_pages(void);
    int		get_pageSize(void);
    int		get_Kbytes(void);
    int		get_addrBits(void);
    int		get_addrWords(void);
    int		get_speed(void);

    void	begin(int);			// Must supply a speed in Khz ... defaults to save 100[Khz]

    char*	status(void);

    int 	setBlock(	const uint16_t	memoryAddress,
				const uint8_t	value,
				const uint16_t	length);

    int		writeByte(	const uint16_t	memoryAddress,
				const uint8_t	value);

    int		writeBlock(	const uint16_t	memoryAddress,
				const uint8_t*	buffer,
				const uint16_t	length);

    uint8_t	readByte(	const uint16_t	memoryAddress);

    uint16_t	readBlock(	const uint16_t	memoryAddress,
				      uint8_t*	buffer,
				const uint16_t	length);


//-------------------------------------
//	Private
//-------------------------------------
private:
    uint8_t	_deviceAddress;
    uint16_t	_deviceSize;
    uint16_t	_Kbytes;
    uint16_t	_pages;	
    uint8_t	_pageSize;
    uint16_t	_addrBits;
    uint16_t	_addrWords;
    uint16_t	_speed=0;	// Sanity condition '0' for begin() not called
    uint32_t 	_lastWrite;     // for waitEEReady
    char	_statbuf[128];

    // for some smaller chips that use one-word addresses
    //bool _isAddressSizeTwoWords;
    bool	_TwoWordAddr;	// What about C1024 ??


    //
    // Prototypes
    //
    /**
     * Begins wire transmission and selects the given address to write/read.
     * 
     * @param memoryAddress Address to write/read
     */
    void	_beginTransmission(const uint16_t memoryAddress);

    int		_pageBlock(	const uint16_t	memoryAddress,
				const uint8_t*	buffer,
				const uint16_t	length,
				const bool	incrBuffer);

    int		_WriteBlock(	const uint16_t	memoryAddress,
				const uint8_t*	buffer,
				const uint8_t	length);

    uint8_t	_ReadBlock(	const uint16_t	memoryAddress,
				      uint8_t*	buffer,
				const uint8_t	length);

    void	waitEEReady();
};
#endif
