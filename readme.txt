This is an excessive rewrite of Rob Tillaart's I2C EEPROM
library that was picked up in revision 1.2.05 from GitHub.

See the history in the .cpp file for essential changes

Context-diffs are supplied in the directory 'Changes'
against the original code revision outlined above.

The code works with EEprom 24xx01..512 (ATMELs and compatibles)
It was tested on an ArduinoMega256 with a 24C64 in several 
sketches and it turned out that it works as expected under
the test conditions.

Anyway, it needs thorough testing with other EEprom types than the
24C64 which was unfortunately the only type I had at hand.

See the example sketch 'I2C-eeprom-dump.ino' under examples for a
practical use-case.

------------
(2016-01-26)
Heinz-Peter Heidinger (hph, hph[at]comserve-it-services.de)
Essen/Germany
