#include "SWI.h"

// ########## CONSTRUCTOR ##########
SWI::SWI(uint8_t pin) { 
  _pin = pin;
}
// ########## PRIVATE ##########
/*
 * @brief: сигнал сброса на линии SWI
 */
void SWI::doBreak(void) {
  // Настроить вывод на выход
  pinMode(_pin, OUTPUT);
  // Сигнал сброса
  digitalWrite(_pin, LOW);
  delayMicroseconds(SWI_DELAY_BREAK);
  // Отпустить вывод
  pinMode(_pin, INPUT);
  delayMicroseconds(SWI_DELAY_BREAK_RECOVERY);
}
/*
 * @brief: передача одного байта в контроллер побитово
 * @param payload: указатель на полезные данные
 */
void SWI::sendByteBitByBit(uint8_t *payload) {
  // Настроить вывод на выход
  pinMode(_pin, OUTPUT);
  // Формирования восьми битов
  for (uint8_t i = 0; i < 8; i++) {
    // Начало бита всегда низкого уровня
    digitalWrite(_pin, LOW);
    delayMicroseconds(SWI_DELAY_HOST_WIDTH1);
    // Если бит = 1 изменить состояние вывода на высокий уровень
    if (*payload >> i & 1) {
        digitalWrite(_pin, HIGH);
    }
    delayMicroseconds(SWI_DELAY_HOST_WIDTH0 - SWI_DELAY_HOST_WIDTH1);
    // Конец бита всегда высокого уровня
    digitalWrite(_pin, HIGH);
    delayMicroseconds(SWI_DELAY_HOST_CYCLE - SWI_DELAY_HOST_WIDTH0);
  }
  // Отпустить вывод
  pinMode(_pin, INPUT);
  delayMicroseconds(SWI_DELAY_STOP);
}
/*
 * @brief: прочитать один байт с линии данных SWI
 * @return: целое число размером один байт uint8_t
 */
uint8_t SWI::receiveByteBitByBit(void) {
  uint8_t result = 0; 
  uint8_t tries = SWI_DELAY_FAIL_TRIES;
  
  for (uint8_t i = 0; i < 8; i++) {
    tries = SWI_DELAY_FAIL_TRIES;
    // Если контроллер не отвечает в течении заданного времени
    while (digitalRead(_pin) != 0 && tries-- > 0) {
      if (tries == 1) {
        // Возвращаем 0x00
        return 0x00;
      }
    }
    delayMicroseconds(((SWI_DELAY_SLAVE_WIDTH0 - SWI_DELAY_SLAVE_WIDTH1) / 2) + SWI_DELAY_SLAVE_WIDTH1);
    // Читаем бит собирая байт
    result |= digitalRead(_pin) << i;
    delayMicroseconds(SWI_DELAY_SLAVE_CYCLE - SWI_DELAY_SLAVE_WIDTH0);
  }
  delayMicroseconds(SWI_DELAY_STOP);

  return result;
}
/*
 * @brief: отправить пакет байтов в контроллер
 * @param input_payload: указатель на массив полезных данных
 * @param payload_length: количество байтов для передачи
 */
void SWI::sendPack(uint8_t *input_payload, uint8_t payload_length) { 
  delayMicroseconds(SWI_DELAY_TIMEOUT);
  // Сброс
  SWI::doBreak();
  // Передаем байты
  while (payload_length --) {
    SWI::sendByteBitByBit((uint8_t*)input_payload ++);
  }
  // Сброс
  SWI::doBreak();
}
/*
 * @brief: принять пакет байтов от контроллера
 * @param crc_num_byte: номер байта контрольной суммы в пакете
 * @return: логичекое значение bool 
 */
bool SWI::receivePack(uint8_t crc_num_byte) {
  for (uint8_t i = 0; i < RECEIVE_BUFFER_SIZE; i ++) {
    _receive_buffer[i] = SWI::receiveByteBitByBit();
  }
  // Проверка контрольной суммы CRC8
  if (SWI::calculateCRC(_receive_buffer, crc_num_byte) == _receive_buffer[crc_num_byte]) {
    return true;
  }
  else {
    return false;
  }
}
/*
 * @brief: вычисляет контрольную сумму массива байтов (или одного байта)
 *  тип контрольной суммы - CRC8
 *  степень полинома CRC_POLYNOME = 0x31
 *  начальное значение CRC_START_MASK = 0xFF
 *  конечное значение CRC_END_MASK = 0x00
 *  отзеркаливание битов входного и выходного значений
 *  честно скопировано отсюда https://github.com/RobTillaart/CRC/blob/master/CRC.h
 * @param input_data: указатель на массив байтов для получения контрольной суммы
 * @param data_length: количество байтов в массиве
 * @return: целое число размером один байт uint8_t
 */
uint8_t SWI::calculateCRC(uint8_t* input_data, uint8_t data_length) {
  uint8_t crc = CRC_START_MASK;
  while (data_length --) {
    crc ^= bitReverse(*input_data ++);
    for (uint8_t i = 8; i; i --) {
      if (crc & CRC_BIT_MASK) {
        crc <<= 1;
        crc ^= CRC_POLYNOME;
      }
      else {
        crc <<= 1;
      }
    }
  }
  crc = bitReverse(crc ^ CRC_END_MASK);
  
  return crc;
}
/*
 * @brief: отзеркаливание битов
 *  честно скопировано отсюда https://github.com/RobTillaart/CRC/blob/master/CRC.h
 * @param input: байт, который нужно отразить
 * @return: целое число размером один байт uint8_t
 */
uint8_t SWI::bitReverse(uint8_t input) {
  uint8_t reversed = input;
  
  reversed = (((reversed & 0xAA) >> 1) | ((reversed & 0x55) << 1));
  reversed = (((reversed & 0xCC) >> 2) | ((reversed & 0x33) << 2));
  reversed =          ((reversed >> 4) | (reversed << 4));
  
  return reversed;
}

// ########## PUBLIC ##########
/*
 * @brief: проверить соединение с чипом
 * @return: логичекое значение bool
 */
bool SWI::isLightningConnected(void){
  uint8_t pack[SWI_CHIP_ID_PACK_SIZE] = SWI_CHIP_ID_PACK;
  SWI::sendPack(pack, SWI_CHIP_ID_PACK_SIZE);

  if (SWI::receiveByteBitByBit() == pack[0] + 1) { // 0x75
    return true;
  }
  else {
    return false;
  }
}
/*
 * @brief: получить информацию о Chip ID
 * @param num_byte: номер байта из принятого пакета
 * @return: целое число размером один байт byte
 */
byte SWI::getLightningChipID(uint8_t num_byte) {
  uint8_t pack[SWI_CHIP_ID_PACK_SIZE] = SWI_CHIP_ID_PACK;
  SWI::sendPack(pack, SWI_CHIP_ID_PACK_SIZE);

  if (SWI::receivePack(NUM_CRC_CHIP_ID)) {
    return _receive_buffer[num_byte];
  }
  else {
    return 0x00;
  }
}
/*
 * @brief: получить информацию о VID
 * @return: целое число размером один байт byte
 */
byte SWI::getLightningVID(void) {
  uint8_t pack[SWI_VID_TO_IDSN_SIZE] = SWI_VID_TO_IDSN;
  SWI::sendPack(pack, SWI_VID_TO_IDSN_SIZE);

  if (SWI::receivePack(NUM_CRC_VID_TO_IDSN)) {
    return _receive_buffer[BYTE_VID];
  }
  else {
    return 0x00;
  }
}
/*
 * @brief: получить информацию о PID
 * @return: целое число размером один байт byte
 */
byte SWI::getLightningPID(void) {
  uint8_t pack[SWI_VID_TO_IDSN_SIZE] = SWI_VID_TO_IDSN;
  SWI::sendPack(pack, SWI_VID_TO_IDSN_SIZE);

  if (SWI::receivePack(NUM_CRC_VID_TO_IDSN)) {
    return _receive_buffer[BYTE_PID];
  }
  else {
    return 0x00;
  }
}
/*
 * @brief: получить информацию о VER
 * @return: целое число размером один байт byte
 */
byte SWI::getLightningVER(void) {
  uint8_t pack[SWI_VID_TO_IDSN_SIZE] = SWI_VID_TO_IDSN;
  SWI::sendPack(pack, SWI_VID_TO_IDSN_SIZE);

  if (SWI::receivePack(NUM_CRC_VID_TO_IDSN)) {
    return _receive_buffer[BYTE_VER];
  }
  else {
    return 0x00;
  }
}
/*
 * @brief: получить информацию о AV
 * @return: целое число размером один байт byte
 */
byte SWI::getLightningAV(void) {
  uint8_t pack[SWI_VID_TO_IDSN_SIZE] = SWI_VID_TO_IDSN;
  SWI::sendPack(pack, SWI_VID_TO_IDSN_SIZE);

  if (SWI::receivePack(NUM_CRC_VID_TO_IDSN)) {
    return _receive_buffer[BYTE_AV];
  }
  else {
    return 0x00;
  }
}
/*
 * @brief: получить информацию о IDSN
 * @param num_byte: номер байта из принятого пакета
 * @return: целое число размером один байт byte
 */
byte SWI::getLightningIDSN(uint8_t num_byte) {
  uint8_t pack[SWI_VID_TO_IDSN_SIZE] = SWI_VID_TO_IDSN;
  SWI::sendPack(pack, SWI_VID_TO_IDSN_SIZE);

  if (SWI::receivePack(NUM_CRC_VID_TO_IDSN)) {
    return _receive_buffer[num_byte];
  }
  else {
    return 0x00;
  }
}
/*
 * @brief: получить набор данных ASN
 * @return: указатель на массив за вычетом первого элемента
 */
char* SWI::getLightningASN(void) {
  uint8_t pack[SWI_ASN_PACK_SIZE] = SWI_ASN_PACK;
  SWI::sendPack(pack, SWI_ASN_PACK_SIZE);

  if (SWI::receivePack(NUM_CRC_ASN)) {
    return (char*)_receive_buffer + 1;
  }
  else {
    return (char*)0x00;
  }
}
/*
 * @brief: получить набор данных MSN
 * @return: указатель на массив за вычетом первого элемента
 */
char* SWI::getLightningMSN(void) {
  uint8_t pack[SWI_MSN_PACK_SIZE] = SWI_MSN_PACK;
  SWI::sendPack(pack, SWI_MSN_PACK_SIZE);

  if (SWI::receivePack(NUM_CRC_MSN)) {
    return (char*)_receive_buffer + 1;
  }
  else {
    return (char*)0x00;
  }
}
/*
 * @brief: получить байты пакета с кодом 0x73 
 * @param num_byte: номер байта из принятого пакета
 * @return: целое число размером один байт byte
 */
byte SWI::getLightningUnknownPack(uint8_t num_byte) {
  uint8_t pack[SWI_UNKNOWN_PACK_SIZE] = SWI_UNKNOWN_PACK;
  SWI::sendPack(pack, SWI_UNKNOWN_PACK_SIZE);

  if (SWI::receivePack(NUM_CRC_UNKNOWN_PACK)) {
    return _receive_buffer[num_byte];
  }
  else {
    return 0x00;
  }
}