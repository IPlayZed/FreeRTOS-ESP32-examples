// Use only 1 core for demo purposes.
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

#define DEFAULT_SERIAL_BAUD 115200
#define DEFAULT_STACK_SIZE 1024
#define DEFAULT_STACK_PRIORITY 1
#define DEFAULT_BUFFER_LENGTH 255
#define BUFFER_CLEAR 0
#define EOL '\n'
#define NULL_TERMINATOR '\0'
#define FLAG_UNSET 0
#define FLAG_SET 1

// Globals
static char* message = NULL;
static volatile uint8_t messageFlagReadyToPrint = FLAG_UNSET;

//*****************************************************************************
// Debug functions:
void _debugGetHeapInfo() {
  Serial.print("Free heap (bytes): ");
  Serial.println(xPortGetFreeHeapSize());
}
//*****************************************************************************

//*****************************************************************************
// Tasks
void readSerial(void *parameters) {

  char character;
  char buffer[DEFAULT_BUFFER_LENGTH];
  uint8_t bufferPosition = 0;

  memset(buffer, BUFFER_CLEAR, DEFAULT_BUFFER_LENGTH);
  
  while (1) {

    // If there is something on UART, read a byte (character).
    if (Serial.available() > 0) {
      character = Serial.read();

      // Store received character to buffer if it still fits into the buffer.
      Serial.print("DEBUG-Character stream: ");
      if (bufferPosition < DEFAULT_BUFFER_LENGTH - 1) {
        buffer[bufferPosition] = character;
        Serial.print(buffer[bufferPosition]);
        Serial.println();
        bufferPosition++;
      }
      
      if (character == EOL) {

        // Add the null terminator to the end of the string.
        buffer[bufferPosition - 1] = NULL_TERMINATOR;
        Serial.print("Buff: ");
        for (int i = 0; i<bufferPosition; i++) {
          Serial.print(buffer[i]);
        }
        Serial.println();

        // Try to allocate memory and copy over message. If message buffer is
        // still in use, ignore the entire message.
        if (messageFlagReadyToPrint == FLAG_UNSET) {
          message = (char *)pvPortMalloc(bufferPosition * sizeof(char));

          // If malloc returns 0 (out of memory), throw an error and reset.
          configASSERT(message);

          memcpy(message, buffer, bufferPosition);

          // Notify other task that message is ready.
          messageFlagReadyToPrint = FLAG_SET;
        }

        memset(buffer, BUFFER_CLEAR, DEFAULT_BUFFER_LENGTH);
        bufferPosition = 0;
      }
    }
  }
}

// Task: print message whenever flag is set and free buffer
void printSerial(void *parameters) {
  while (1) {

    // Wait for flag to be set and print message.
    if (messageFlagReadyToPrint == FLAG_SET) {
      Serial.print("Echo: ");
      Serial.println(message);
      Serial.println("== Before free ==");
      _debugGetHeapInfo();
      vPortFree(message);
      Serial.println("== After free ==");
      _debugGetHeapInfo();
      Serial.println();
      message = NULL;
      messageFlagReadyToPrint = FLAG_UNSET;
    }
  }
}

//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  Serial.begin(DEFAULT_SERIAL_BAUD);

  vTaskDelay(2000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("=============================================");
  Serial.println("---FreeRTOS Heap Demo---");
  Serial.println("Enter a string");

  // Start Serial receive task
  xTaskCreatePinnedToCore(readSerial,
                          "Read Serial",
                          DEFAULT_STACK_SIZE,
                          NULL,
                          DEFAULT_STACK_PRIORITY,
                          NULL,
                          app_cpu);

  // Start Serial print task
  xTaskCreatePinnedToCore(printSerial,
                          "Print Serial",
                          DEFAULT_STACK_SIZE,
                          NULL,
                          DEFAULT_STACK_PRIORITY,
                          NULL,
                          app_cpu);
  
  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {}
