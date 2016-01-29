//
//    FILE:	I2C_eepromV2.cpp
//  AUTHOR:	Rob Tillaart (original author)
//  AUTHOR:	H.P. Heidinger for release 2.0.0b
// VERSION:	2.0.0b
// PURPOSE: 	I2C_eeprom library for Arduino with EEPROM 24xx01..512;
//		24xx1024 not yet supported
// ----------------------------------------------------------------------------------------
// HISTORY:
// 0.1.00 - 2011-01-21 initial version
// 0.1.01 - 2011-02-07 added setBlock function
// 0.2.00 - 2011-02-11 fixed 64 bit boundary bug
// 0.2.01 - 2011-08-13 _readBlock made more robust + return value
// 1.0.00 - 2013-06-09 support for Arduino 1.0.x
// 1.0.01 - 2013-11-01 fixed writeBlock bug, refactor
// 1.0.02 - 2013-11-03 optimize internal buffers, refactor
// 1.0.03 - 2013-11-03 refactor 5 millis() write-latency
// 1.0.04 - 2013-11-03 fix bug in readBlock, moved waitEEReady()
//                     -> more efficient.
// 1.0.05 - 2013-11-06 improved waitEEReady(),
//                     added determineSize()
// 1.1.00 - 2013-11-13 added begin() function (Note breaking interface)
//                     use faster block Wire.write()
//                     int casting removed
// 1.2.00 - 2014-05-21 Added support for Arduino DUE ( thanks to Tyler F.)
// 1.2.01 - 2014-05-21 Refactoring
// 1.2.02 - 2015-03-06 stricter interface
// 1.2.03 - 2015-05-15 bugfix in _pageBlock & example (thanks ifreislich )
//
// --------- Changes by hph (H.P. Heidinger, Essen/Germany, hph(at)comserve-it-services.de) -
// 2.0.0b - 2016-01-26  [ --------[ Main changes ] -------------------------------------------
// 			- Changed coding style from 'PASCALish/JAVAish' to clean C-style
// 			- Removed constructor #1 which used hard-coded page size for all PROMs
// 			  supplyed by a '#define'
//			- Removed constructor #1, since it relied on assumptions
// 			- Changed device determination from construtor #2
// 			  Now the 'type code' of the PROM is passed; i.e. 24c64 passes '64'
// 			  as second argument ... rest is done by a "rope ladder" ...
// 			- Changed begin() function to supply a speed value
// 			  Sets TWBR accordingly by another "rope ladder" ...
// 			  Falls back to 100Khz (TWBR=72) on illegal values.
// 			  Value is one of 50,100,200,250,400,500,800,888 or 1000 [Khz]
// 			  ... see begin() function for details 
// 			- Removed 'determinesize', cause it fiddeled the PROM with writes
// 			- Introduced a status() function that return the guts after instantiation
//
// 			  char* msg;
//
// 				msg = <instance>.status();
// 				Serial.println(msg);
//
//				... or simply: Serial.println( <instance>.status() );
//
// 			- Introduced get_XXXX functions that gets the guts separately:
//
// 				uint8_t  I2C_eeprom::get_deviceAddress()   { return _deviceAddress; }
// 				int      I2C_eeprom::get_deviceSize()      { return _deviceSize;    }
// 				int      I2C_eeprom::get_pages()           { return _pages;         }
// 				int      I2C_eeprom::get_pageSize()        { return _pageSize;      }
// 				int      I2C_eeprom::get_Kbytes()          { return _Kbytes;        }
// 				int      I2C_eeprom::get_addrBits()        { return _addrBits;      }
// 				int      I2C_eeprom::get_addrWords()       { return _addrWords;     }
//
// 				(See examples in the test INO: I2C_eeprom-dump.ino ==> mystatus()) 
// 			  
// 			  These two save alot of hard-coded '#defines ' and are deterministic ...
// 				
// 			- Lots of cleanups and code optimizations
//			- Checks were made with a 24C64 [ST]; no others available ... unfortunately
//			- Bugs?? Find them and keep 'em in a warm place ...
//
//
// --------------------------------------------------------------------------------------------
// Released to the public domain
// --------------------------------------------------------------------------------------------


#include <I2C_eepromV2.h>

#if defined(ARDUINO) && ARDUINO >= 100
    #define WIRE_WRITE Wire.write
    #define WIRE_READ  Wire.read
#else
    #define WIRE_WRITE Wire.send
    #define WIRE_READ  Wire.receive
#endif


//
// Definitions ... local
//
#define I2C_WRITEDELAY  5000	// uSecs to wait between writes


//
// Constructor ...
//
// deviceAddress is the address on the I2C bus
//
// DEVtype as the integer following the type 'xx' in "24xx"
// Examples:	For a 24C01	pass 1
//	 	For a 24C04 	pass 4
//	 	For a 24C64 	pass 64
//		For a 24LC128 	pass 128
//		... a.s.o.
//
I2C_eeprom::I2C_eeprom(const uint8_t deviceAddress, const unsigned int DEVtype) {

	this->_deviceAddress	= deviceAddress;
	this->_deviceSize	= DEVtype;
	this->_Kbytes		= DEVtype/8;

	//
	// Setup for specific PROM ... determined by it's type
	//
	// 24xx01..512 	...	are supported
	// 24xx1024 	...	needs code not yet implemented /hph Jan-2016
	//
	switch (DEVtype) {  		// see also ATMEL's information
		// -------------------- PS 8 -- 1 address word ----- 
		case 1		:	this->_pages            = 16;
					this->_pageSize		= 8;
					this->_addrBits		= 7;
					this->_addrWords        = 1;
					break;

		case 2		:	this->_pages            = 32;
					this->_pageSize         = 8;
                                        this->_addrBits		= 8;
					this->_addrWords        = 1;
					break;

		// -------------------- PS 16 & 2 address words ----- 
		case 4		:	this->_pages            = 32;
					this->_pageSize         = 16;
					this->_addrBits		= 9;
					this->_addrWords        = 2;
					break;

		case 8		:	this->_pages            = 64;
                                        this->_pageSize         = 16;
                                        this->_addrBits		= 10;
					this->_addrWords        = 2;
					break;

		case 16		:	this->_pages		= 128;
					this->_pageSize         = 16;
					this->_addrBits		= 11;
					this->_addrWords        = 2;
					break;
	
		// -------------------- PS 32 ------------------------ 
		case 32		:	this->_pages            = 256;
					this->_pageSize         = 32;
					this->_addrBits		= 12;
					this->_addrWords        = 2;
					break;

		case 64		:	this->_pages            = 256;
					this->_pageSize		= 32;
					this->_addrBits		= 13;
					this->_addrWords        = 2;
					break;

		// -------------------- PS 64 ------------------------ 
		case 128	:	this->_pages		= 256;
					this->_pageSize         = 64;
					this->_addrBits		= 14;
					this->_addrWords        = 2;
					break;

		case 256	:	this->_pages		= 512;
					this->_pageSize         = 64;
					this->_addrBits		= 15;
					this->_addrWords        = 2;
					break;

		// -------------------- PS 128 ----------------------- 
		case 512	:	this->_pages		= 512;
					this->_pageSize         = 128;
					this->_addrBits		= 16;
					this->_addrWords        = 2;
					break;

		// -------------------- PS 256 ----------------------- 
		// This is still a curious thing	
		// case 1024	:	this->_pages		= 512;
		//			this->_pageSize         = 256;
		//			this->_addrBits		= 17;	// 17 bit addressing ??
		//			this->_addrWords        = 3;	// Actually this is "three-word" 
		//			break;

		default		:	this->_pages            = 0;
					this->_pageSize		= 0;
					this->_addrBits		= 0;
					this->_addrWords        = 0;
					break;	
	}
}



//
// Introducer
//
void I2C_eeprom::begin(int speed) {
    Wire.begin();
    _lastWrite = 0;

// TWBR is not available on Arduino Due
#ifdef TWBR
// TWBR = 72;  /* Sanity value */
// 0=1000, 1=888, 2=800, 8=500, 12=400, 24=250, 32=200, 72=100, 152=50 [Khz]
// F_CPU/16+(2*TWBR) // TWBR is a uint8_t

	switch (speed) {
		case  50:	TWBR=152;	this->_speed=50;	break;
		case 100:	TWBR=72;	this->_speed=100;	break;
		case 200:	TWBR=32;	this->_speed=200;	break;
		case 250:	TWBR=24;	this->_speed=250;       break;
		case 400:	TWBR=12;	this->_speed=400;       break;
		case 500:	TWBR=8;		this->_speed=500;       break;
		case 800:	TWBR=2;		this->_speed=800;       break;
		case 888:	TWBR=1;		this->_speed=888;       break;
		case 1000:	TWBR=0;		this->_speed=1000;      break;
		default:	TWBR=72; 	this->_speed=100;
	}
#endif
}


//
// Return a pointer to msg buffer to get instantiation guts ... helps debugging
//
char* I2C_eeprom::status() {
	sprintf(_statbuf,"\n24x%d @ 0x%02x, %dKhz\nKBytes: %d, Pages: %d, Page size: %d\nAddrbits: %d, Addrwords: %d\n",
		this->_deviceSize, 	this->_deviceAddress,	this->_speed,
		this->_Kbytes,		this->_pages, 		this->_pageSize,
		this->_addrBits, 	this->_addrWords);

	return _statbuf;
}



//
// Fill a block with byte xx
//
int I2C_eeprom::setBlock(const uint16_t memoryAddress, const uint8_t data, const uint16_t length) {
uint8_t buffer[I2C_TWIBUFFERSIZE];
	
	for (uint8_t i=0; i<I2C_TWIBUFFERSIZE; i++) buffer[i] = data;

	return ( _pageBlock(memoryAddress, buffer, length, false) );
}


//
// Write a single byte to PROM's <memoryAddress>
// Return number of bytes written; here always 1
//
int I2C_eeprom::writeByte(const uint16_t memoryAddress, const uint8_t data) {
	return ( _WriteBlock(memoryAddress, &data, 1) );
}


//
// Write bytes to PROM's <memoryAddress> starting at <buffer>
// Return number of bytes written
//
int I2C_eeprom::writeBlock(const uint16_t memoryAddress, const uint8_t* buffer, const uint16_t length) {
    return ( _pageBlock(memoryAddress, buffer, length, true) );
}


//
// Read byte at PROM's <memoryAddress>
// Return number of bytes read; here always 1
//
uint8_t I2C_eeprom::readByte(const uint16_t memoryAddress) {
uint8_t rdata;

	_ReadBlock(memoryAddress, &rdata, 1);
	return rdata;
}


//
// Read bytes from PROM starting at <memoryAddress> to <buffer>
// Return number of bytes read
//
uint16_t I2C_eeprom::readBlock(const uint16_t memoryAddress, uint8_t* buffer, const uint16_t length) {
uint16_t addr 	= memoryAddress;
uint16_t len 	= length;
uint16_t rv 	= 0;
uint8_t  cnt;

    while (len > 0) {
        cnt	 = min(len, I2C_TWIBUFFERSIZE);
        rv	+= _ReadBlock(addr, buffer, cnt);
        addr	+= cnt;
        buffer	+= cnt;
        len	-= cnt;
    }

    return rv;
}

//
// Utility functions
//
uint8_t		I2C_eeprom::get_deviceAddress() 	{ return _deviceAddress; 	}
int		I2C_eeprom::get_deviceSize() 		{ return _deviceSize; 		}
int 		I2C_eeprom::get_pages() 		{ return _pages; 		}
int 		I2C_eeprom::get_pageSize()		{ return _pageSize;		}
int 		I2C_eeprom::get_Kbytes()		{ return _Kbytes;		}
int 		I2C_eeprom::get_addrBits()		{ return _addrBits;		}
int 		I2C_eeprom::get_addrWords()		{ return _addrWords;		}
int 		I2C_eeprom::get_speed()			{ return _speed;		}




////////////////////////////////////////////////////////////////////
//
//	PRIVATE
//
////////////////////////////////////////////////////////////////////

//
// _pageBlock aligns buffer to page boundaries for writing.
// and to TWI buffer size
// returns 0 = OK otherwise error
int I2C_eeprom::_pageBlock(const uint16_t memoryAddress, const uint8_t* buffer, const uint16_t length, const bool incrBuffer) {
uint16_t	 addr = memoryAddress;
uint16_t	 len = length;
int		 rv = 0;

    while (len > 0) {
        uint8_t bytesUntilPageBoundary = this->_pageSize - addr % this->_pageSize;
        uint8_t cnt = min(len, bytesUntilPageBoundary);
        cnt = min(cnt, I2C_TWIBUFFERSIZE);

        rv = _WriteBlock(addr, buffer, cnt);
        if (rv != 0) return rv;

        addr += cnt;
        if (incrBuffer)
	   buffer += cnt;

        len -= cnt;
    }
    return rv;
}


//
// Supports one and 2 bytes addresses
//
void I2C_eeprom::_beginTransmission(const uint16_t memoryAddress) {
	Wire.beginTransmission(_deviceAddress);

	if (this->_addrWords > 1) 
		WIRE_WRITE((memoryAddress >> 8));	// Address High Byte

	WIRE_WRITE((memoryAddress & 0xFF)); 		// Address Low Byte
							// (or only byte for chips 16K or smaller
							// that only have one-word addresses)
}


//
// Write a block to PROM @ <memory address> from buffer pointer with length 
//
// pre: length <= this->_pageSize  && length <= I2C_TWIBUFFERSIZE;
// returns 0 = OK otherwise error
int I2C_eeprom::_WriteBlock(const uint16_t memoryAddress, const uint8_t* buffer, const uint8_t length) {
int	rv;

    waitEEReady();

    this->_beginTransmission(memoryAddress);

    WIRE_WRITE(buffer, length);

    rv = Wire.endTransmission();
    _lastWrite = micros();
    return rv;
}


// Pre: Buffer is large enough to hold length bytes
// returns bytes read
uint8_t I2C_eeprom::_ReadBlock(const uint16_t memoryAddress, uint8_t* buffer, const uint8_t length) {
int		rv;
uint8_t 	cnt = 0;
uint32_t	before = millis();

    waitEEReady();

    this->_beginTransmission(memoryAddress);

    rv = Wire.endTransmission();
    if (rv != 0) return 0;  // error

    Wire.requestFrom(_deviceAddress, length);
    cnt = 0;
    before = millis();
    while ((cnt < length) && ((millis() - before) < I2C_EEPROM_TIMEOUT)) {
        if (Wire.available())
		buffer[cnt++] = WIRE_READ();
    }
    return cnt;
}

void I2C_eeprom::waitEEReady() {

    // Wait until EEPROM gives ACK again.
    // this is a bit faster than the hardcoded 5 milliSeconds
    while ((micros() - _lastWrite) <= I2C_WRITEDELAY) {
        Wire.beginTransmission(_deviceAddress);
        if ( Wire.endTransmission() == 0 ) break;
    }
}
