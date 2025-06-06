#ifndef FUNCTIONSESPNOW_H
#define FUNCTIONSESPNOW_H

#include <esp_now.h>
#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "MAC_addresses.h" //Archivo de cabecera  con las direcciones MAC de los ESP32
#include "Structs.h"
#include "UtilitiesFunctions.h"
#include <ESPmDNS.h>

void sendDate(int peekID);
int searchSender(const uint8_t *mac);

// Se ejecutará cada que se ENVIEN datos con ESP-NOW
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
	int ID = searchSender(mac_addr);

	switch (ID)
	{
	case accsModule:
		resultAccs = status;
		break;
	case lightModule:
		resultLight = status;
		break;
	case tempModule:
		resultTemp = status;
		break;
		break;
	}
	
	if (status == ESP_NOW_SEND_SUCCESS)
		controlStatusLED(CLEAR);
	else
		controlStatusLED(ERROR);
}

// Envia un Mensaje por medio de ESP-NOW con el ID especificado
void sendData(uint8_t peerID, espNowData data)
{
	switch (peerID)
	{
	case tempModule:
		controlStatusLED(SENDTEMP);
		esp_now_send(MACS[peerID], (uint8_t *)&data.temperatureModule, sizeof(tempDevice));
		break;
	case lightModule:
		controlStatusLED(SENDLIGHT);
		esp_now_send(MACS[peerID], (uint8_t *)&data.lightModule, sizeof(lightDevices));
		break;
	case accsModule:
		controlStatusLED(SENDACCS);
		esp_now_send(MACS[peerID], (uint8_t *)&data.accessModule, sizeof(accsDevice));
		break;
	default:
		controlStatusLED(ERROR);
		Serial.println("ID INVALIDO");
		break;
	}
}

// Devuelve true si la direccion MAC coincide, usado en SearchSender
bool areMacEquals(const uint8_t *src, const uint8_t *dst)
{
	for (auto i = 0; i < 6; i++)
	{
		if (src[i] != dst[i])
			return false;
	}
	return true;
}

// Devuelve el ID del emisor dada una direccion MAC
int searchSender(const uint8_t *mac)
{
	for (auto senderID = 0; senderID < MACS_COUNT; senderID++)
	{
		if (areMacEquals(mac, MACS[senderID]))
			return senderID;
	}
	controlStatusLED(ERROR);
	return -1;
}

void onDataReceived(const uint8_t *mac, const uint8_t *data, int len)
{
	Serial.printf("Packet received from: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	Serial.printf("Bytes received: %d\n", len);

	int senderID = searchSender(mac); // Busca el ID del emisor

	if (len == sizeof(char))
	{ // Primero validamos si no es una peticion de parte de un Modulo
		char actionR = data[0];
		switch (actionR)
		{
		case requestTime: // Peticion para dar la fecha
			switch (senderID)
			{
			case accsModule:
				controlStatusLED(RECEIVEDACCS);
				break;
			case lightModule:
				controlStatusLED(RECEIVEDLIGHT);
				break;
			case tempModule:
				controlStatusLED(RECEIVEDTEMP);
				break;
			}
			sendDate(senderID);
			break;
		default:
			break;
		}
		return;
	}

	switch (senderID) // Sino, copiamos los datos en la estructura del Host de acuerdo al origen
	{
	case tempModule: // Temp
		controlStatusLED(RECEIVEDTEMP);
		memcpy(&receivedData.temperatureModule, data, sizeof(tempDevice));
		break;
	case lightModule: // Luz
		controlStatusLED(RECEIVEDLIGHT);
		memcpy(&receivedData.lightModule, data, sizeof(lightDevices));
		break;
	case accsModule: // Accs
		controlStatusLED(RECEIVEDACCS);
		memcpy(&receivedData.accessModule, data, sizeof(accsDevice));						   // Copiamos los datos recibidos a la variable global
		getActualDate(receivedData.accessModule.date, sizeof(receivedData.accessModule.date)); // Actualizamos la fecha de los datos
		accsEvent accsHistoryData;															   // Creamos una nueva variable para almacenar el acceso recibido
		strcpy(accsHistoryData.key, receivedData.accessModule.key);							   // llenamos el struct
		strcpy(accsHistoryData.date, receivedData.accessModule.date);
		accsHistoryData.status = receivedData.accessModule.status;
		saveAccsHistory(accsHistoryData); // Lo guardamos en el almacenamiento interno del ESP32
		newAccsAction = true;
		break;
	default:
		Serial.println("ID INVALIDO");
		controlStatusLED(ERROR);
		break;
	}
}

// Registra un dispositivo a la red de ESP-NOW con el ID especificado
void registerPeek(uint8_t id, wifi_interface_t iface)
{
	esp_now_peer_info_t peerInfo = {};
	memcpy(peerInfo.peer_addr, MACS[id], 6);
	peerInfo.channel = 0;
	peerInfo.encrypt = false;
	peerInfo.ifidx = iface;

	if (esp_now_add_peer(&peerInfo) != ESP_OK)
	{
		Serial.println("Error registrando peer");
	}
	else
	{
		Serial.print("Peer registrado: ");
		Serial.println(id);
	}
}

// Registra todos los dispositivos
void static registerPeeks()
{
	for (auto peekID = 0; peekID < MACS_COUNT; peekID++)
	{
		registerPeek(peekID, WIFI_IF_STA); // HOST
										   // RegisterPeek(peek_id, WIFI_IF_AP); //MODULO
	}
}

void sendDate(int peekID)
{
	char date[20];
	getDateServer(date, sizeof(date));
	Serial.println("date:");
	Serial.println(date);
	Serial.println("sizeof():");
	Serial.println(sizeof(date));
	esp_now_send(MACS[peekID], (uint8_t *)&date, sizeof(date));
}

// Inicializa ESP-NOW
void static initEspNow()
{
	controlStatusLED(WAITING);
	WiFi.mode(WIFI_AP_STA); // Modo AP y Station HOST

	// WiFi.mode(WIFI_AP); //Modo AP MODULO
	Serial.printf("\n\nConectando a la RED: %s\n", ssid);
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(250);
		Serial.print(".");
	}
	// Imprime la Direccion IP del Server para acceder a el
	Serial.println("\n\nWiFi Conectado Corecctamente!\nDireccion IP: ");
	Serial.print(WiFi.localIP());
	Serial.printf("\nCanal Wifi: %d\n\n", WiFi.channel());

	if (!MDNS.begin("smarthomeesp32"))
	{
		Serial.println("Error iniciando mDNS");
	}
	else
	{
		Serial.println("mDNS iniciado: http://smarthomeesp32.local/");
	}

	if (esp_now_init() != ESP_OK)
	{
		Serial.println("Error initializing ESP-NOW");
		controlStatusLED(ERROR);
	}
	else
	{
		esp_now_register_send_cb(onDataSent);
		esp_now_register_recv_cb(onDataReceived);
		registerPeeks();
		controlStatusLED(CLEAR);
	}
	for (int peekID = 0; peekID < MACS_COUNT; peekID++)
	{
		sendDate(peekID);
	}
}

bool requestModule(int peerID, char action)
{ // ACCESO: K=Eliminar LLaves, D=Request Data
	return esp_now_send(MACS[peerID], (uint8_t *)&action, sizeof(char));
}

#endif