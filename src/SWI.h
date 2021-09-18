/*
 * Библиотека позволяет получить информацию из аксессуара с разъемом Lightning
 ** по интерфейсу SWI на контактах ID0 или ID1, он же ID_BUS, ACC_ID.
 ** Работает с:
 *** Кабель Lightning/USB
 *** Кабель USB‑C/Lightning
 *** EarPods с разъёмом Lightning
 *** Адаптер Lightning/выход 3,5 мм для наушников
 *** Возможно, раотает и с другими кабелями
 * 
 * Протокол SWI реализован программно и использует стандартные функции Ардуино
 ** pinMode()
 ** digitalRead()
 ** digitalWrite()
 ** delayMicroseconds()
 * 
 * Ссылки:
 ** http://ramtin-amin.fr/#tristar
 ** https://nyansatan.github.io/lightning/
 ** https://appleinsider.com/articles/13/05/09/apples-lightning-connector-finally-detailed-in-patent-filing
 ** https://www.techinsights.com/blog/systems-analysis-apple-lightning-usb-cable
 ** https://blog.lambdaconcept.com/post/2019-10/iphone-bootrom-debug/
 * 
 * Проект SWI Library на GitHub - https://github.com/S-LABc/SWI-Lib
 * Декодер протокола SWI на GitHub - https://github.com/S-LABc/SWI-Protocol-Decoder
 * 
 * Контакты:
 ** YouTube - https://www.youtube.com/channel/UCbkE52YKRphgkvQtdwzQbZQ
 ** Telegram - https://www.t.me/slabyt
 ** GitHub - https://github.com/S-LABc
 ** Gmail - romansklyar15@gmail.com
 * 
 * Copyright (C) 2021. v1.0 / Скляр Роман S-LAB
 */

#pragma once
#include <Arduino.h>

// Выводы по умолчанию для разных плат (зависит от ядра)
#define STM32_SWI_DEFAULT_PIN          PB12
#define ESP8266_SWI_DEFAULT_PIN        D4
#define ESP32_SWI_DEFAULT_PIN          4
#define ARDUINO_BOARDS_SWI_DEFAULT_PIN 3
// Максимальный размер буфера и номера байтов контрольных сумм CRC8
#define RECEIVE_BUFFER_SIZE  22
#define NUM_CRC_CHIP_ID      7
#define NUM_CRC_VID_TO_IDSN  12
#define NUM_CRC_ASN          21
#define NUM_CRC_MSN          21
#define NUM_CRC_UNKNOWN_PACK 6
// Настройки алгоритма вычисления контрольной суммы CRC8
#define CRC_START_MASK 0xFF
#define CRC_END_MASK   0x00
#define CRC_POLYNOME   0x31
#define CRC_BIT_MASK   0x80
// Константы пакетов отправляемых хостом и их размер
#define SWI_CHIP_ID_PACK_SIZE 4
#define SWI_CHIP_ID_PACK      {0x74, 0x00, 0x02, 0x1F}
#define SWI_VID_TO_IDSN_SIZE  2
#define SWI_VID_TO_IDSN       {0x76, 0x10}
#define SWI_ASN_PACK_SIZE     2
#define SWI_ASN_PACK          {0x78, 0x0F}
#define SWI_MSN_PACK_SIZE     2
#define SWI_MSN_PACK          {0x7A, 0xB3}
#define SWI_UNKNOWN_PACK_SIZE 2
#define SWI_UNKNOWN_PACK      {0x72, 0x71}
// Настройки задержек протокола SWI
#define SWI_DELAY_FAIL_TRIES     100
#define SWI_DELAY_TIMEOUT        1300
#define SWI_DELAY_BREAK          12
#define SWI_DELAY_BREAK_RECOVERY 3
#define SWI_DELAY_STOP           8
#define SWI_DELAY_HOST_CYCLE     8
#define SWI_DELAY_HOST_WIDTH1    2
#define SWI_DELAY_HOST_WIDTH0    5
#define SWI_DELAY_SLAVE_CYCLE    10
#define SWI_DELAY_SLAVE_WIDTH1   3
#define SWI_DELAY_SLAVE_WIDTH0   7
// Номера байтов в принимаемом пакете Chip ID
enum ChipIDBytes {
  CHIP_ID_BYTE1 = 1, // Байт в элементе 0 не интересует
  CHIP_ID_BYTE2,
  CHIP_ID_BYTE3,
  CHIP_ID_BYTE4,
  CHIP_ID_BYTE5,
  CHIP_ID_BYTE6,
};
// Номера байтов в принимаемом пакете с VID по ID-SN
enum VIDtoIDSN {
  BYTE_VID = 1, // Байт в элементе 0 не интересует
  BYTE_PID,
  BYTE_VER,
  BYTE_AV,
  IDSN_BYTE1,
  IDSN_BYTE2,
  IDSN_BYTE3,
  IDSN_BYTE4,
  IDSN_BYTE5,
  IDSN_BYTE6,
};
// Номера байтов в принимаемом пакете с Unknown Pack
enum UnknownPack {
  UNKNOWN_PACK_BYTE1 = 1, // Байт в элементе 0 не интересует
  UNKNOWN_PACK_BYTE2,
  UNKNOWN_PACK_BYTE3,
  UNKNOWN_PACK_BYTE4,
};

class SWI {
  private:
    uint8_t _pin; // Вывод микроконтроллера
    uint8_t _receive_buffer[RECEIVE_BUFFER_SIZE]; // Буфер для принятых данных

    void doBreak(void); // Сигнал сброса на линии SWI
    void sendByteBitByBit(uint8_t *payload); // Передача байта побитово
    uint8_t receiveByteBitByBit(void); // Получить байт побитово с линии SWI
	
    void sendPack(uint8_t *input_payload, uint8_t payload_length); // Передача байтов в контроллер
    bool receivePack(uint8_t crc_num_byte); // Принять пакет от чипа и проверить CRC8
    
    uint8_t calculateCRC(uint8_t* input_data, uint8_t data_length); // Вычисление контрольной суммы CRC8
    uint8_t bitReverse(uint8_t input); // Обращение битов в байте
    
  public:
    SWI(uint8_t pin); // Конструктор
	
    bool isLightningConnected(void); // Проверка связи с чипом
    
    byte getLightningChipID(uint8_t num_byte);
	
    byte getLightningVID(void);
    byte getLightningPID(void);
    byte getLightningVER(void);
    byte getLightningAV(void);
    byte getLightningIDSN(uint8_t num_byte);
	
    char* getLightningASN(void);
	
    char* getLightningMSN(void);
	
	byte getLightningUnknownPack(uint8_t num_byte); // Не знаю за что отвечают эти данные
};
