/**
 * @file sEEPROM.cpp
 * @author silvio3105 (www.github.com/silvio3105)
 * @brief Simple EEPROM translation unit.
 * 
 * @copyright Copyright (c) 2023
 * 
 */

/*
License

Copyright (c) 2023, silvio3105 (www.github.com/silvio3105)

Access and use of this Project and its contents are granted free of charge to any Person.
The Person is allowed to copy, modify and use The Project and its contents only for non-commercial use.
Commercial use of this Project and its contents is prohibited.
Modifying this License and/or sublicensing is prohibited.

THE PROJECT AND ITS CONTENT ARE PROVIDED "AS IS" WITH ALL FAULTS AND WITHOUT EXPRESSED OR IMPLIED WARRANTY.
THE AUTHOR KEEPS ALL RIGHTS TO CHANGE OR REMOVE THE CONTENTS OF THIS PROJECT WITHOUT PREVIOUS NOTICE.
THE AUTHOR IS NOT RESPONSIBLE FOR DAMAGE OF ANY KIND OR LIABILITY CAUSED BY USING THE CONTENTS OF THIS PROJECT.

This License shall be included in all methodal textual files.
*/


// ----- INCLUDE FILES
#include			"sEEPROM.h"

#ifdef SEEPROM_CS

// ----- METHOD DEFINITIONS
sEEPROM::sEEPROM(uint32_t s, uint16_t len)
{
	start = s;
	length = len;
}

sEEPROM::~sEEPROM(void)
{
	start = 0x0;
	length = 0x0;
}


uint8_t sEEPROM::read(uint16_t startOffset, void* output, uint16_t len)
{
	// If required number of bytes to read go outside EEPROM sector
	if ((start + startOffset + len) < (start + len)) return SEEPROM_OF;

	uint16_t idx = 0;
	uint8_t* addr = (uint8_t*)(start + startOffset);

	do
	{
		// Read value from EEPROM
		((uint8_t*)output)[idx] = addr[idx];

		// Increase index
		idx++;
	}
	while (idx != len);

	return SEEPROM_OK;
}

uint8_t sEEPROM::write(uint16_t startOffset, void* value, uint16_t len)
{
	// If required number of bytes to write go outside EEPROM sector
	if ((start + startOffset + len) < (start + len)) return SEEPROM_OF;

	// Unlock EEPROM write access
	unlockEEPROM();

	// Calculate number of 4 byte values
	uint16_t len4 = len / 4;
	len = len % 4;

	// Calculate number of 2 byte values
	uint16_t len2 = len / 2;
	len = len % 2;

	// Write 4 bytes values if needed
	if (len4)
	{
		write<uint32_t>((uint32_t*)(start + startOffset), (uint32_t*)value, len4);

		// Move offset address
		startOffset += (len4 * 4);
	}

	// Write 2 byte values if needed
	if (len2)
	{
		write<uint16_t>((uint16_t*)(start + startOffset), (uint16_t*)((uint16_t*)value + startOffset), len2);

		// Move offset address
		startOffset += (len2 * 2);
	}

	// Write 1 byte values if needed
	if (len) write<uint8_t>((uint8_t*)(start + startOffset), (uint8_t*)((uint8_t*)value + startOffset), len);

	// Lock EEPROM write access
	lockEEPROM();

	return SEEPROM_OK;
}

uint8_t sEEPROM::erase(uint16_t startOffset, uint16_t len)
{
	// Check if offset address is aligned by 4 bytes
	if (startOffset % 4) return SEEPROM_NOK;

	// Check for EEPROM overflow
	if ((start + startOffset + (len * 4)) < (start + (len * 4))) return SEEPROM_OF;

	uint16_t idx = 0;
	uint32_t* addr = (uint32_t*)(start + startOffset);

	// Unlock EEPROM write access
	unlockEEPROM();

	// Enable EEPROM erase
	FLASH->PECR |= FLASH_PECR_ERASE;

	do
	{
		// Erase four bytes
		addr[idx] = 0x00;

		// Wait for interrupt
		__WFI();

		// Increase index
		idx++;
	}
	while (idx != len);

	// Disable EEPROM erase
	FLASH->PECR &= ~FLASH_PECR_ERASE;

	// Lock EEPROM write access
	lockEEPROM();

	return SEEPROM_OK;
}

#endif // SEEPROM_CS

// END WITH NEW LINE
