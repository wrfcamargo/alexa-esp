#include <Arduino.h>
#include "fauxmoESP.h"

#ifdef ESP32
    #include <WiFi.h>
    #define RELAY_PIN_1 12
    #define RELAY_PIN_2 14
#else
    #define RELAY_PIN_1 5
    #define RELAY_PIN_2 4
#endif

#define SERIAL_BAUDRATE 115200

#define WIFI_SSID "House_Bennett_WGO_Fibra"
#define WIFI_PASS "joaopedropreguicinha"

#define TOM "Tv"
#define LAMP "Lâmpada 1"

// --------------------------------------------------------------------------------------------- //

static void wifiSetup(void);

// --------------------------------------------------------------------------------------------- //

fauxmoESP fauxmo;

static void wifiSetup(void)
{
    //Define o como STA
    WiFi.mode(WIFI_STA);

    Serial.printf("[WIFI] Conectado ao %s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }

    Serial.printf("\n[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

void setup(void)
{
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println();

    wifiSetup();

    //Dispositivos a serem ligados
    pinMode(RELAY_PIN_1, OUTPUT);
    digitalWrite(RELAY_PIN_1, HIGH);

    pinMode(RELAY_PIN_2, OUTPUT);
    digitalWrite(RELAY_PIN_2,HIGH);

    // Por padrão, fauxmoESP cria seu próprio servidor web na porta definida
    // A porta TCP deve ser 80 para dispositivos gen3 (o padrão é 1901)
    // Isso deve ser feito antes da chamada enable()
    fauxmo.createServer(true); // Cria o servidor
    fauxmo.setPort(80); // Necessário para os dispositivos gen3

    // Você deve chamar enable(true) assim que tiver uma conexão WiFi
    // Você pode ativar ou desativar a biblioteca a qualquer momento
    // Desativá-lo impedirá que os dispositivos sejam descobertos e trocados
    fauxmo.enable(true);

    // Você pode usar diferentes maneiras de chamar a Alexa para modificar o estado dos dispositivos:
    // "Alexa, ligar Ventilador"

    // Adiciona os dispositivos 
    fauxmo.addDevice(TOM);
    fauxmo.addDevice(LAMP);

    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value)
    {
        // Retorno de chamada quando um comando da Alexa é recebido.
        // Você pode usar device_id ou device_name para escolher o elemento no qual realizar uma ação (relé, LED, ...)
        // O state é um booleano (ON / OFF) e value um número de 0 a 255 (se você disser "definir a luz da cozinha para 50%", receberá 128 aqui).

        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);

        if (strcmp(device_name, TOM) == 0)
        {
            Serial.println("RELAY 1 switched by Alexa");
            digitalWrite(RELAY_PIN_1, state ? LOW : HIGH);
        }

        if (strcmp(device_name, LAMP) == 0)
        {
            Serial.println("RELAY 2 switched by Alexa");
            digitalWrite(RELAY_PIN_2, state ? LOW : HIGH);
        }
    });
}

void loop(void)
{
    static unsigned long last = millis();

    // fauxmoESP usa um servidor TCP assíncrono, mas um servidor UDP sincronizado
    // Portanto, temos que pesquisar manualmente os pacotes UDP
    fauxmo.handle();

    if ((millis() - last) > 5000)
    {
        last = millis();
        Serial.printf("[MAIN] Free heap: %d bytes\n", ESP.getFreeHeap());
    }
}