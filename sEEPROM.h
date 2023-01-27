/**
 * @file sEEPROM.h
 * @author silvio3105 (www.github.com/silvio3105)
 * @brief Simple EEPROM driver header file.
 * 
 * @copyright Copyright (c) 2023, silvio3105
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

#ifndef _SEEPROM_H_
#define _SEEPROM_H_

// Define it here and undef it if known chip is not selected
#define SEEPROM_CS

/** \addtogroup sEEPROM
 * @{
 * Simple EEPROM driver. It provides functions for easy access to EEPROM on STM32 & nRF52 chips.
 * Check chip datasheet for EEPROM start address(es) and length(s).
 * It is possible to create multiple objects which represents different parts of EEPROM(eg., EEPROM for device config, EEPROM for device history etc.).
*/

// STM32L051
#ifdef STM32L051xx

/** \defgroup STM32L051
 * @{
 * EEPROM for STM32L051 MCU.
*/


// ----- INCLUDE FILES
#include			<stm32l051xx.h>
#include			<system_stm32l0xx.h>


// ----- DEFINES
// ERROR CODES
#define SEEPROM_NOK				0 /**< @brief Return code for not OK status. */
#define SEEPROM_OK				1 /**< @brief Return code for OK status. */
#define SEEPROM_OF				2 /**< @brief Return code for prevented overflow. */

// EEPROM
#define SEEPROM_START			0x08080000 /**< @brief EEPROM start address. */
#define SEEPROM_SIZE			2048 /**< @brief EEPROM size in bytes. */
#define SEEPROM_END				(SEEPROM_START + SEEPROM_SIZE) /**< @brief EEPROM end address. */

// VALUES
#define PEKEY_VALUE_1			0x89ABCDEF /**< @brief Value 1 to unlock EEPROM and PECR. */
#define PEKEY_VALUE_2			0x02030405 /**< @brief Value 2 to unlock EEPROM and PECR. */


// ----- CLASSES
/**
 * @brief EEPROM class.
 * 
 */
class sEEPROM {
	// PUBLIC STUFF
	public:
	// OBJECT CONSTRUCTORS AND DECONSTRUCTORS
	/**
	 * @brief Object constructor.
	 * 
	 * @param s EEPROM start address.
	 * @param len EEPROM length in bytes.
	 * @return No return value.
	 */
	sEEPROM(uint32_t s, uint16_t len);

	/**
	 * @brief Object deconstructor.
	 * 
	 * @return No return value.
	 */
	~sEEPROM(void);


	/**
	 * @brief Read \c len bytes from EEPROM.
	 * 
	 * @param startOffset Start address offset in bytes.
	 * @param output Pointer to output array.
	 * @param len Size of \c output array in bytes.
	 * @return \c SEEPROM_OF if reading \c len bytes will go outside defined area.
	 * @return \c SEEPROM_OK is read is successful.
	 */
	uint8_t read(uint16_t startOffset, void* output, uint16_t len);

	/**
	 * @brief Write \c len bytes to EEPROM.
	 * 
	 * @param startOffset Start address offset in bytes.
	 * @param value Pointer to input values to write.
	 * @param len Length of \c value in bytes.
	 * @return \c SEEPROM_OF if writing \c len bytes will overflow defined area.
	 * @return \c SEEPROM_OK is write is successful.
	 */
	uint8_t write(uint16_t startOffset, void* value, uint16_t len);

	/**
	 * @brief Erase \c len words in EEPROM.
	 * 
	 * @param startOffset Start address offset in bytes.
	 * @param len Number of words to erase.
	 * @return \c SEEPROM_NOK if \c len is not aligned by 4 bytes.
	 * @return \c SEEPROM_OF if erasing \c len words will erase words outside defined area.
	 * @return \c SEEPROM_OK if erasing is successful.
	 */
	uint8_t erase(uint16_t startOffset, uint16_t len);


	// PRIVATE STUFF
	private:
	// VARIABLES
	uint32_t start = 0x0; /**< @brief EEPROM start address. */
	uint16_t length = 0x0; /**< @brief EEPROM length in bytes. */

	// METHOD DECLARATIONS
	/**
	 * @brief Backend write method.
	 * 
	 * This method handles writes to EEPROM. It is called by main write method.
	 * 
	 * @tparam T \c value type
	 * @param startAddr Start address.
	 * @param value Pointer to array with values of \c T type.
	 * @param len Number of members in \c value array.
	 */
	template<typename T>
	void write(T* startAddr, T* value, uint16_t len)
	{
		uint16_t idx = 0;

		do
		{
			// Wait for EEPROM if busy
			while (FLASH->SR & FLASH_SR_BSY);

			// Write value
			startAddr[idx] = value[idx];

			// Increase index
			idx++;
		}
		while (idx != len);		
	}

	/**
	 * @brief Unlock write access to EEPROM and PECR register.
	 * 
	 * @return No return value.
	 */
	inline void unlockEEPROM(void)
	{
		// Wait if flash is busy
		while (FLASH->SR & FLASH_SR_BSY);

		// Write required values to unlock EEPROM and PECR
		FLASH->PEKEYR = PEKEY_VALUE_1;
		FLASH->PEKEYR = PEKEY_VALUE_2;
	}

	/**
	 * @brief Alias for \ref unlockEEPROM
	 * 
	 * @return No return value.
	 */
	inline void unlockPECR(void)
	{
		unlockEEPROM();
	}

	/**
	 * @brief Lock write access to EEPROM and PECR register.
	 * 
	 * @return No return value.
	 */
	inline void lockEEPROM(void)
	{
		// Wait if flash is busy
		while (FLASH->SR & FLASH_SR_BSY);

		// Lock EEPROM and PECR by writing 1 to PELOCK bit
		FLASH->PECR |= FLASH_PECR_PELOCK;
	}

	/**
	 * @brief Alias for \ref lockEEPROM
	 * 
	 * @return No return value.
	 */
	inline void lockPECR(void)
	{
		lockEEPROM();
	}
};


/**@}*/

#else
#undef SEEPROM_CS
#warning "sEEPROM: Selected chip is not supported!"
#endif // STM32L051xx

/**@}*/

#endif // _SEEPROM_H_

// END WITH NEW LINE
