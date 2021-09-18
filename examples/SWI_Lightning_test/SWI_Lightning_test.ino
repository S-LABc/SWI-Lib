/*
 * SWI_Ligthtning_test
 *
 ** Пример использования функций бибилотеки SWI.h для обмена данными
 ** по интерфейсу SWI на контактах ID0 или ID1, он же ID_BUS, ACC_ID.
 ** Работает с:
 *** Кабель Lightning/USB
 *** Кабель USB‑C/Lightning
 *** EarPods с разъёмом Lightning
 *** Адаптер Lightning/выход 3,5 мм для наушников
 *** Возможно, раотает и с другими кабелями
 * 
 * ########## !!! В А Ж Н О !!! ##########
 ** ОБЯЗАТЕЛЬНО используйте согласователь уровней,
 ** например TXS0108E(HW-221) или подобный.
 * 
 ** Протокол SWI реализован программно и использует стандартные функции Arduino IDE
 *** pinMode()
 *** digitalRead()
 *** digitalWrite()
 *** delayMicroseconds()
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

// Подключаем библиотеку
#include <SWI.h>

/* Создаем объект SWI с указанием вывода микроконтроллера
 * ### Обязательно использовать согласователь уровней на 2.5В! ###
 * STM32_HDQ_DEFAULT_PIN = PB12
 * ESP8266_HDQ_DEFAULT_PIN = D4
 * ESP32_HDQ_DEFAULT_PIN = 5
 * ARDUINO_BOARDS_HDQ_DEFAULT_PIN = 3
 * или какой-нибудь еще
 */
SWI SWI(STM32_SWI_DEFAULT_PIN);

// Храненит промежуточное значение для удобного визуального отображения байтов
byte temp = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  // Если аксессуар подключен
  if (SWI.isLightningConnected()) {
    // Читаем данные и выводим в "Монитор порта"
    readAccessoryData();
  }
  else {
    // Выводим сообщение об отсутствии аксессуара
    Serial.println("Accessory not detected");
  }
  
  delay(3000); // Проверяем раз в 3 секунды
}

void readAccessoryData() {
  Serial.println("----- Accessory Info -----");
  // Вывод информации о Chip ID
  // CHIP_ID_BYTE1, CHIP_ID_BYTE2, CHIP_ID_BYTE3,
  // CHIP_ID_BYTE4, CHIP_ID_BYTE5, CHIP_ID_BYTE6
  Serial.print("Chip ID: ");
  for (uint8_t i = CHIP_ID_BYTE1; i <= CHIP_ID_BYTE6; i ++) {
    temp = SWI.getLightningChipID(i);
    if (temp <= 0x0F) {
      Serial.print('0');
    }
    Serial.print(temp, HEX);
    Serial.print(' ');
  }
  Serial.println();

  // Вывод информации о VID
  Serial.print("VID: ");
  temp = SWI.getLightningVID();
  if (temp <= 0x0F) {
    Serial.print('0');
  }
  Serial.print(temp, HEX);
  Serial.print(" | ");

  // Вывод информации о PID
  Serial.print("PID: ");
  temp = SWI.getLightningPID();
  if (temp <= 0x0F) {
    Serial.print('0');
  }
  Serial.print(temp, HEX);
  Serial.print(" | ");

  // Вывод информации о VER
  Serial.print("VER: ");
  temp = SWI.getLightningVER();
  if (temp <= 0x0F) {
    Serial.print('0');
  }
  Serial.print(temp, HEX);
  Serial.print(" | ");

  // Вывод информации о AV
  Serial.print("AV: ");
  temp = SWI.getLightningAV();
  if (temp <= 0x0F) {
    Serial.print('0');
  }
  Serial.println(temp, HEX);

  // Вывод информации о ID-SN
  // IDSN_BYTE1, IDSN_BYTE, IDSN_BYTE3,
  // IDSN_BYTE4, IDSN_BYTE5, IDSN_BYTE6
  Serial.print("ID-SN: ");
  for (uint8_t i = IDSN_BYTE1; i <= IDSN_BYTE6; i ++) {
    temp = SWI.getLightningIDSN(i);
    if (temp <= 0x0F) {
      Serial.print('0');
    }
    Serial.print(temp, HEX);
    Serial.print(' ');
  }
  Serial.println();
  
  // Вывод информации о ASN
  Serial.print("ASN: ");
  Serial.println(SWI.getLightningASN());

  // Вывод информации о MSN
  Serial.print("MSN: ");
  Serial.println(SWI.getLightningMSN());
  
  // Вывод информации о Unknown Pack
  // UNKNOWN_PACK_BYTE1, UNKNOWN_PACK_BYTE2,
  // UNKNOWN_PACK_BYTE3, UNKNOWN_PACK_BYTE4
  Serial.print("Unknown Pack: ");
  for (uint8_t i = UNKNOWN_PACK_BYTE1; i <= UNKNOWN_PACK_BYTE4; i ++) {
    temp = SWI.getLightningUnknownPack(i);
    if (temp <= 0x0F) {
      Serial.print('0');
    }
    Serial.print(temp, HEX);
    Serial.print(' ');
  }
  Serial.println();
  
  Serial.println();
}
