// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#define SERIAL_BAUD 115200
#define DEFAULT_STACK_SIZE 1024
#define DEFAULT_STACK_PRIORITY 1

// Pins
const int led_pin = 26;

// Globals
static volatile int led_delay = 500;   // ms

//*****************************************************************************
// Tasks

// Task: Blink LED at rate set by global variable
void toggleLED(void *parameter) {
  while (1) {
    digitalWrite(led_pin, HIGH);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
  }
}

// Task: Read from serial terminal
// Note: This does NOT expect any line ending after serial data is sent.
void readSerial(void *parameters) {

  // Loop forever
  while (1) {

    // Read characters from serial
    if (Serial.available() > 0) {
      led_delay = Serial.parseInt();
      // Update delay variable and reset buffer if we get a newline character
      Serial.print("Updated LED delay to: ");
      Serial.println(led_delay);
    }
  }
}

//*****************************************************************************
// Main

void setup() {

  // Configure pin
  pinMode(led_pin, OUTPUT);

  // Configure serial and wait a second
  Serial.begin(SERIAL_BAUD);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println("Multi-task LED Demo");
  Serial.println("Enter a number in milliseconds to change the LED delay.");

  // Start blink task
  xTaskCreatePinnedToCore(          // Use xTaskCreate() in vanilla FreeRTOS
            toggleLED,              // Function to be called
            "Toggle LED",           // Name of task
            DEFAULT_STACK_SIZE,     // Stack size (bytes in ESP32, words in FreeRTOS)
            NULL,                   // Parameter to pass
            DEFAULT_STACK_PRIORITY, // Task priority
            NULL,                   // Task handle
            app_cpu);               // Run on one core for demo purposes (ESP32 only)
            
  // Start serial read task
  xTaskCreatePinnedToCore(          // Use xTaskCreate() in vanilla FreeRTOS
            readSerial,             // Function to be called
            "Read Serial",          // Name of task
            DEFAULT_STACK_SIZE,     // Stack size (bytes in ESP32, words in FreeRTOS)
            NULL,                   // Parameter to pass
            DEFAULT_STACK_PRIORITY, // Task priority (must be same to prevent lockup)
            NULL,                   // Task handle
            app_cpu);               // Run on one core for demo purposes (ESP32 only)

  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {
  // Execution should never get here
}
