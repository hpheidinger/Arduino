//
//               FILE:  I2C_eeprom-dump.ino
//             AUTHOR:  hph
//            PURPOSE:  Demonstrate or test I2C_eepromV2 library Rev. 2.0.0b (==>)
//            Support:  24x01..512; Not currently for 24x1024 (24[X]M01) or even bigger, Maybe in Rev. 2.1
//           Platform:  ArduinoMega256
// Sketch consumption:  ~11500 Bytes
//               Date:  2016-01-27
//---------------------------------------------------------------------------------------------------------

// Must include I2C basic library
#include <Wire.h>       

#include <I2C_eepromV2.h>

// For a 24C64; 1 for 24x01, 2 for 24x02, 8 for 24x08, 16 for 24x16 ... a.s.o.
#define  PROMtype    64
#define  PROMaddr    0x50

#define  TESTbyte    0x55
#define  DESTRUCTIVE false      // !!! CAREFUL: 'true' writes the entire PROM with TESTbyte !!!!!

// Instantiate that thing ...
I2C_eeprom ee(PROMaddr, PROMtype);


void mystatus() {
     Serial.println("------------------ begin mystatus --------------");
     Serial.print  ("Device Address : ");
     Serial.println(ee.get_deviceAddress());
     
     Serial.print  ("Device Size[Kb]: ");
     Serial.println(ee.get_deviceSize());
     
     Serial.print  ("Memory Size[KB]: ");
     Serial.println(ee.get_Kbytes());
     
     Serial.print  ("Pages          : ");
     Serial.println(ee.get_pages());
     
     Serial.print  ("Page size      : ");
     Serial.println(ee.get_pageSize());
     
     Serial.print  ("Address bits   : ");
     Serial.println(ee.get_addrBits());
     
     Serial.print  ("Address words  : ");
     Serial.println(ee.get_addrWords());
     
     Serial.print  ("Speed [Khz]    : ");
     Serial.println(ee.get_speed());
     Serial.println("------------------ end mystatus --------------");
}


void readstats( uint32_t dump_diff ) {
char  buf[10];

        Serial.println("Done reading...");
        Serial.print  ("It took ");
        dtostrf((float)dump_diff/1000/1000, 5,2, buf );
        Serial.print  (buf); 
        Serial.print  (" seconds @ ");
        Serial.print  (ee.get_speed());
        Serial.println("Khz I2C bus speed");  

}  

//
// Dump PROM by single-byte reads ...
//
void ByteDumpEEPROM(uint16_t addr, uint16_t length) {
long  start,  diff;
int   i;
int   count;
char  abuf[8];
char  vbuf[4];
 
      count=0;
      while (count < length ) {
         sprintf(abuf, "%04x: ", count);
         Serial.print(abuf);

         for (i=0; i<16; i++) {
            sprintf(vbuf, "%02x ", ee.readByte(count++));
            Serial.print(vbuf);
         }
         Serial.println();  
      }    
}



//
// Dump PROM by pages according to pagesize ...
//
// addr is a PAGE addr, length is cut length (modulo) pagesize
//
void PageDumpEEPROM(uint16_t addr, uint16_t length) {
long  start,  diff;
int   i,j, bufchunk;
char  abuf[8];
char  vbuf[4];
int   page, pages;
int   pagesize = ee.get_pageSize();
int   memaddr;
byte  pagebuf[256];    // This is huge ... take care on an UNO
 
      page  = addr;
      pages = (length/pagesize);
      
      while ( pages-- > 0  ) {
         memaddr = page*pagesize;
         ee.readBlock( memaddr, pagebuf, pagesize );    // Fetch an entire page
          
         for (j=0; j<pagesize/16; j++)    {
               bufchunk=j*16;
               sprintf(abuf, "%04x: ", page*pagesize+bufchunk);
               Serial.print(abuf);  
               
               for (i=0; i<16; i++) {
                  sprintf(vbuf, "%02x ", pagebuf[i+bufchunk] );
                  Serial.print(vbuf);
               }
               Serial.println();
         } 
         page++; 
      }    
}


//
// Print status through library status() function
//
void lib_status() {
        Serial.println("Library status ...");
        Serial.println(ee.status());
}        
  

void setup() {
long    start;
long    diff;
int     eetype, eebytes;
char    typestr[30];
char*   msg;
        
        Serial.begin(115200);   // Assure before any other 'Serial.xxxx' cmd
        Serial.print("\12");
        
        msg=ee.status();           // ... before any ee.begin()
        Serial.println(msg);
        
        
        if (ee.get_speed() == 0) {    // Forgot ee.begin() ??? (speed is still 0!)
           Serial.println("<XX>.begin() not called! Fixing ...");
           
           ee.begin(0);         // supply a speed in Khz !! [0],50,100,200,250,400,500,800,888 or 1000
        }                       // defaults to 100 on illegal value; if not called it defaults to internal '0'
                                // a value of '0' sets the default speed=100[Khz] like any other illegal value
                      
        msg=ee.status();           // ... after forgotten begin
        Serial.println(msg);
        
        
        ee.begin(400);          // Re-configure decently; override default of 100 Khz
                                // You might try other speeds according to the datasheet of your PROM
        
        //msg=ee.status();        // ... after decent begin() ...  
        //Serial.println(msg);
        // ... or this one; saves memory, costs time ...
        Serial.println(ee.status());
         
        Serial.println("\n-------------------------------");
        Serial.print  ("Dump check -  I2C eeprom library ");
        Serial.print  (I2C_EEPROM_VERSION);
        Serial.println();
        
        Serial.println("Calling library status function ...");
        // msg=ee.status();
        // Serial.println(msg);
        // ... or this one; saves memory, costs even more time ...
        lib_status();
        
        eetype  = ee.get_deviceSize();
        eebytes = ee.get_Kbytes()*1024;
        
        if (DESTRUCTIVE == true)
           ee.setBlock(0, TESTbyte, eebytes);
        
        sprintf(typestr, "Chip is a 24x%d, Size in Bytes: %5d\n", eetype, eebytes);
        Serial.println(typestr);
        
        mystatus();    // Check all the get_xxxx() functions in a local procedure
        

        //
        // Now dump the PROM byte-wise
        //  ... according to its size with the values determined
        //
        Serial.println("\n -- Dumping bytes ...");
        start = micros();
        ByteDumpEEPROM(0,eebytes);
        diff  = micros() - start;
        readstats(diff);
        
        //
        // Now dump the PROM page-wise
        //  ... according to its size with the values determined
        //
        Serial.println("\n -- Dumping pages ...");
        start = micros();
        PageDumpEEPROM(0,eebytes);
        diff  = micros() - start;
        readstats(diff);
}

void loop() {/* Just waste time, honeypie */}

