#ifndef UTILITIESFUNCTIONS_H
#define UTILITIESFUNCTIONS_H

#include "Structs.h"
#include <Arduino.h>
#include "FunctionsEspNow.h"

void printAccsDevice(const accsDevice& device) {
    Serial.print("createKey: ");
    Serial.println(device.createKey);
    
    Serial.print("Key: ");
    for (int i = 0; i < 16; i++) {
      Serial.print(device.key[i]);
    }
    Serial.println();
  
    Serial.print("Mode: ");
    Serial.println(device.mode);
  
    Serial.println("Keys:");
    for (int i = 0; i < MAX_KEYS_NUM; i++) {
      Serial.print("Key ");
      Serial.print(i);
      Serial.print(": ");
      for (int j = 0; j < 16; j++) {
        Serial.print(device.keys[i][j]);
      }
      Serial.println();
    }
  
    Serial.print("Status: ");
    Serial.println(device.status);
  }


void setDateServer(char* date, size_t size){
  const char* ntpServer = "pool.ntp.org";
  const long  gmtOffset_sec = 0;          // Offset horario GMT (0 para UTC)
  const int   daylightOffset_sec = 3600;  // Ajuste de horario de verano (si aplica)
  struct tm timeInfo;

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  if (!getLocalTime(&timeInfo)) {
    Serial.println("Error al obtener la hora");
    snprintf(date, size, "Error");
    return;
  }

  strftime(date, size, "%Y-%m-%dT%H:%M:%S", &timeInfo); //Lo castea a tipo ISO
}

void setDateString(char* date){
  struct tm timeinfo;
  memset(&timeinfo, 0, sizeof(timeinfo)); // Limpiar estructura
  Serial.println("String Recibido: ");
  for(int i=0;i++;i<sizeof(date)){
    Serial.print(date[i]);
  }
  // Parsear string ISO: "YYYY-MM-DDTHH:MM:SS"
  if (sscanf(date, "%d-%d-%dT%d:%d:%d",
             &timeinfo.tm_year, &timeinfo.tm_mon, &timeinfo.tm_mday,
             &timeinfo.tm_hour, &timeinfo.tm_min, &timeinfo.tm_sec) != 6) {
    Serial.println("Error al parsear la fecha");
    return;
  }

  timeinfo.tm_year -= 1900; // Año desde 1900
  timeinfo.tm_mon  -= 1;    // Mes 0-11

  time_t t = mktime(&timeinfo); // Convertir a timestamp (segundos desde 1970)
  if (t == -1) {
    Serial.println("Error al convertir a timestamp");
    return;
  }
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);
  Serial.print("Hora establecida desde string: ");
  Serial.println(date);
}


#endif