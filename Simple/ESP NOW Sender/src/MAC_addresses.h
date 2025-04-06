#include <cstdint>

const uint8_t MAC_HOST[] = {0x78,0x42,0x1C,0x69,0x3A,0x68};
const uint8_t MAC_TEMP[] = {0x78,0x42,0x1C,0x68,0x8D,0xC4};
const uint8_t MAC_LGHT[] = {0x78,0x42,0x1C,0x68,0x9C,0xA8};
const uint8_t MAC_ACCS[] = {0x78,0x42,0x1C,0x68,0x32,0xB0};

const uint8_t* MACS[] = {MAC_HOST,MAC_TEMP,MAC_LGHT,MAC_ACCS};
const uint8_t MACS_COUNT = sizeof(MACS) / sizeof(uint8_t*);