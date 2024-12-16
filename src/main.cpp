#include <Arduino.h>
#include <WiFi.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

const int BUZZER = 12;
const int MQ_9 = 36; // A0

int sensorValue = 0;
const int threshold = 100;

#define SSID "network-name"
#define PASSWORD "password"

// Initialize Telegram BOT
#define BOT_TOKEN "token-value-here"
// "https://api.telegram.org/bot{BOT_TOKEN}/getUpdates"// to get chat id after sending any message to the created bot
#define CHAT_ID "chat-id-value-here"
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
// X509List cert(TELEGRAM_CERTIFICATE_ROOT); 

unsigned long lastTimeSent = 0;
unsigned long interval = 60000;  // 1 minute

void gasSensor(void *parameter) {
  TickType_t xLastWakeTime; 
  const TickType_t xFrequency = 500 / portTICK_PERIOD_MS; 
  xLastWakeTime = xTaskGetTickCount(); // Initialize xLastWakeTime with the current time 
  while (1) {
    sensorValue = analogRead(MQ_9);
    Serial.println(sensorValue);
    vTaskDelayUntil(&xLastWakeTime, xFrequency); // Maintain a consistent period
  }
}

void buzzerSound(void *parameter) {
  while (1) {
    if (sensorValue > threshold) {
      digitalWrite(BUZZER, HIGH);
      vTaskDelay(300 / portTICK_PERIOD_MS); // 300ms ON
      digitalWrite(BUZZER, LOW);
      vTaskDelay(300 / portTICK_PERIOD_MS); // 300ms OFF
    } else {
      digitalWrite(BUZZER, LOW);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}
void notifications(void *parameter) {
  while (1) {
    if (sensorValue > threshold) {
      Serial.println("Sending Telegram message...");
      bot.sendMessage(CHAT_ID, "Gas detected! Sensor value: " + String(sensorValue), "");
      vTaskDelay(60000 / portTICK_PERIOD_MS); // to prevent immediate re-triggering.
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(9600);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.print("\n Setup and loop task running on core ");
  Serial.print(xPortGetCoreID());
  Serial.print(" with priority ");
  Serial.println(uxTaskPriorityGet(NULL));
  
  pinMode(BUZZER, OUTPUT);

  // WiFi setup
  WiFi.mode(WIFI_STA); // Set mode to station (client)
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setInsecure(); // Skip certificate verification (not recommended for production)
  bot.sendMessage(CHAT_ID, "Gas Sensor started!", "");
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  // vTaskStartScheduler();

  xTaskCreatePinnedToCore(
      gasSensor,     // Function to implement the task
      "Gas Sensor",  // Name of the task
      1024,          // Stack size in words
      NULL,          // Task input parameter
      1,             // Priority of the task
      NULL,          // Task handle.
      app_cpu        // Core where the task should run
  );

  xTaskCreatePinnedToCore(buzzerSound, "Buzzer Sound", 2048, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(notifications, "Notifications", 8192, NULL, 2, NULL, app_cpu);

  // Delete "setup and loop" task
  // vTaskDelete(NULL);

}

void loop() {
  // Execution should never get here
}
