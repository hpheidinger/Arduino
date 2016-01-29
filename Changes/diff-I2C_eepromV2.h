*** ../../../tmp/Arduino/libraries/I2C_EEPROM/I2C_eeprom.h	2016-01-21 15:41:17.175947470 +0100
--- ../I2C_eepromV2.h	2016-01-26 21:51:08.000000000 +0100
***************
*** 21,51 ****
  #include "Wiring.h"
  #endif
  
! #define I2C_EEPROM_VERSION "1.2.03"
! 
! // The DEFAULT page size. This is overriden if you use the second constructor.
! // I2C_EEPROM_PAGESIZE must be multiple of 2 e.g. 16, 32 or 64
! // 24LC256 -> 64 bytes
! #define I2C_EEPROM_PAGESIZE 64
  
  // TWI buffer needs max 2 bytes for eeprom address
  // 1 byte for eeprom register address is available in txbuffer
! #define I2C_TWIBUFFERSIZE  30
  
  // to break blocking read/write after n millis()
! #define I2C_EEPROM_TIMEOUT  1000
  
- // comment next line to keep lib small (idea a read only lib?)
- #define I2C_EEPROM_EXTENDED
  
! class I2C_eeprom
! {
  public:
-     /**
-      * Initializes the EEPROM with a default pagesize of I2C_EEPROM_PAGESIZE.
-      */
-     I2C_eeprom(const uint8_t deviceAddress);
  
      /**
       * Initializes the EEPROM for the given device address.
       *
--- 21,45 ----
  #include "Wiring.h"
  #endif
  
! #define I2C_EEPROM_VERSION "2.0.0b"
  
  // TWI buffer needs max 2 bytes for eeprom address
  // 1 byte for eeprom register address is available in txbuffer
! #define I2C_TWIBUFFERSIZE	30
  
  // to break blocking read/write after n millis()
! #define I2C_EEPROM_TIMEOUT	1000
  
  
! class I2C_eeprom {
! //-------------------------------------
! //	Public space
! //-------------------------------------
  public:
  
+     //
+     // Constructor 	... Use for ATMELs 24xx and compatibles
+     //
      /**
       * Initializes the EEPROM for the given device address.
       *
***************
*** 56,94 ****
       */
      I2C_eeprom(const uint8_t deviceAddress, const unsigned int deviceSize);
  
-     void begin();
-     int writeByte(const uint16_t memoryAddress, const uint8_t value);
-     int writeBlock(const uint16_t memoryAddress, const uint8_t* buffer, const uint16_t length);
-     int setBlock(const uint16_t memoryAddress, const uint8_t value, const uint16_t length);
- 
-     uint8_t readByte(const uint16_t memoryAddress);
-     uint16_t readBlock(const uint16_t memoryAddress, uint8_t* buffer, const uint16_t length);
- 
- #ifdef I2C_EEPROM_EXTENDED
-     int determineSize();
- #endif
  
  private:
!     uint8_t _deviceAddress;
!     uint32_t _lastWrite;     // for waitEEReady
!     uint8_t _pageSize;
  
      // for some smaller chips that use one-word addresses
!     bool _isAddressSizeTwoWords;
  
      /**
       * Begins wire transmission and selects the given address to write/read.
       * 
       * @param memoryAddress Address to write/read
       */
!     void _beginTransmission(const uint16_t memoryAddress);
  
!     int _pageBlock(const uint16_t memoryAddress, const uint8_t* buffer, const uint16_t length, const bool incrBuffer);
!     int _WriteBlock(const uint16_t memoryAddress, const uint8_t* buffer, const uint8_t length);
!     uint8_t _ReadBlock(const uint16_t memoryAddress, uint8_t* buffer, const uint8_t length);
  
!     void waitEEReady();
  };
- 
  #endif
- // END OF FILE
\ No newline at end of file
--- 50,133 ----
       */
      I2C_eeprom(const uint8_t deviceAddress, const unsigned int deviceSize);
  
  
+     //
+     // Prototypes
+     //
+     uint8_t	get_deviceAddress(void);
+     int		get_deviceSize(void);		// resembles removed/old: 'determinesize'; see also get_Kbytes()
+     int		get_pages(void);
+     int		get_pageSize(void);
+     int		get_Kbytes(void);
+     int		get_addrBits(void);
+     int		get_addrWords(void);
+     int		get_speed(void);
+ 
+     void	begin(int);			// Must supply a speed in Khz ... defaults to save 100[Khz]
+ 
+     char*	status(void);
+ 
+     int 	setBlock(	const uint16_t	memoryAddress,
+ 				const uint8_t	value,
+ 				const uint16_t	length);
+ 
+     int		writeByte(	const uint16_t	memoryAddress,
+ 				const uint8_t	value);
+ 
+     int		writeBlock(	const uint16_t	memoryAddress,
+ 				const uint8_t*	buffer,
+ 				const uint16_t	length);
+ 
+     uint8_t	readByte(	const uint16_t	memoryAddress);
+ 
+     uint16_t	readBlock(	const uint16_t	memoryAddress,
+ 				      uint8_t*	buffer,
+ 				const uint16_t	length);
+ 
+ 
+ //-------------------------------------
+ //	Private
+ //-------------------------------------
  private:
!     uint8_t	_deviceAddress;
!     uint16_t	_deviceSize;
!     uint16_t	_Kbytes;
!     uint16_t	_pages;	
!     uint8_t	_pageSize;
!     uint16_t	_addrBits;
!     uint16_t	_addrWords;
!     uint16_t	_speed=0;	// Sanity condition '0' for begin() not called
!     uint32_t 	_lastWrite;     // for waitEEReady
!     char	_statbuf[128];
  
      // for some smaller chips that use one-word addresses
!     //bool _isAddressSizeTwoWords;
!     bool	_TwoWordAddr;	// What about C1024 ??
  
+ 
+     //
+     // Prototypes
+     //
      /**
       * Begins wire transmission and selects the given address to write/read.
       * 
       * @param memoryAddress Address to write/read
       */
!     void	_beginTransmission(const uint16_t memoryAddress);
  
!     int		_pageBlock(	const uint16_t	memoryAddress,
! 				const uint8_t*	buffer,
! 				const uint16_t	length,
! 				const bool	incrBuffer);
! 
!     int		_WriteBlock(	const uint16_t	memoryAddress,
! 				const uint8_t*	buffer,
! 				const uint8_t	length);
! 
!     uint8_t	_ReadBlock(	const uint16_t	memoryAddress,
! 				      uint8_t*	buffer,
! 				const uint8_t	length);
  
!     void	waitEEReady();
  };
  #endif
