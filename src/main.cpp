#include <Arduino.h>
#include <WiFi.h>

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// NOTE: GPI13, GPI15, GPIO02, GPIO4 must be reserved. ADC2 is used by Wi-Fi

// const int RED = 15;
// const int GREEN = 2;
// const int BLUE = 4;
// const int BUTTON = 17;

// const int BUILTIN_LED = 2; 

const int BUZZER = 12;
const int MQ_9 = 36; // A0

int sensorValue = 0;
const int threshold = 290;

const char *ssid = "stc_wifi_59D4";
const char *password = "6T6R776ZSV";


void gasSensor(void *parameter)
{
  while (1)
  {
    sensorValue = analogRead(MQ_9);
    Serial.println(sensorValue);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}
void buzzerSound(void *parameter)
{
  while (1)
  {
    if (sensorValue > threshold)
    {
      digitalWrite(BUZZER, HIGH);
      vTaskDelay(300 / portTICK_PERIOD_MS);
      digitalWrite(BUZZER, LOW);
      vTaskDelay(300 / portTICK_PERIOD_MS);
    }
    else
    {
      digitalWrite(BUZZER, LOW);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

// void blinkLed(void *parameter)
// {
//   while (1)
//   {
//     if (sensorValue <= threshold)
//     {
//       digitalWrite(BUILTIN_LED, HIGH);
//       vTaskDelay(500 / portTICK_PERIOD_MS);
//       digitalWrite(BUILTIN_LED, LOW);
//       vTaskDelay(500 / portTICK_PERIOD_MS);
//     }
//     else
//     {
//       digitalWrite(BUILTIN_LED, LOW);
//       vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
//   }
// }

void setup()
{
  Serial.begin(9600);
  Serial.print("Setup and loop task running on core ");
  Serial.print(xPortGetCoreID());
  Serial.print(" with priority ");
  Serial.println(uxTaskPriorityGet(NULL));
  // pinMode(BUTTON, INPUT);
  // attachInterrupt(digitalPinToInterrupt(BUTTON), buttonPressed, FALLING);
  // pinMode(RED, OUTPUT);
  // pinMode(GREEN, OUTPUT);
  // pinMode(BLUE, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // WiFi setup
  WiFi.mode(WIFI_STA); // Set mode to station (client)
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  xTaskCreatePinnedToCore(
      gasSensor,     // Function to implement the task
      "Gas Sensor", // Name of the task
      2048,          // Stack size in words
      NULL,          // Task input parameter
      1,             // Priority of the task
      NULL,          // Task handle.
      app_cpu        // Core where the task should run
  );

  xTaskCreatePinnedToCore(
      buzzerSound,
      "Buzzer Sound",
      1024,
      NULL,
      1,
      NULL,
      app_cpu);

  // xTaskCreatePinnedToCore(
  //     blinkLed,
  //     "Blink LED",
  //     1024,
  //     NULL,
  //     1,
  //     NULL,
  //     app_cpu);
}

void loop()
{

}
