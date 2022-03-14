// Use only 1 core for demo purposes.
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

#define DEFAULT_SERIAL_BAUD 115200
#define DEFAULT_STACK_SIZE 1024
#define MORE_STACK_SIZE 1300
#define DEFAULT_STACK_PRIORITY 1

void printMemoryInfo() {
    // This will print out the remaining stack memory in words.
    Serial.print("High water mark (in words): ");
    Serial.println(uxTaskGetStackHighWaterMark(NULL));

    // This will print out the free heap memory bytes before malloc.
    Serial.print("Heap memory before malloc (in bytes): ");
    Serial.println(xPortGetFreeHeapSize());
    Serial.println();
}

// Each task in ESP-IDF (FreeRTOS) requires a bare minimum of free 786 bytes for its stack, which is reserved for each task.
// This means that after all the variables on the stack are reserved space to, there must be left at least 786 bytes of free space.
// In this example we demonstrate that using DEFAULT_STACK_SIZE when creating the task, we will run out of stack buffer space.
// The reason for this is that 1024 - 768 = 256 bytes is left after minimum needed stack space is allocated.
// We would need at least 100 * 4 = 400 bytes of free space for our array.
// However, we do not have that much (256 < 400), so there is a stack overflow, causing an error.
// This triggers https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/fatal-errors.html#unhandled-debug-exception .
// To demonstrate a normally working program, replace the stack size with MORE_STACK_SIZE in stack creation.
void stackOverflowTask(void* param) {
  // If we leave out the while loop and only run the content inside of it, there will be a different error:
  // E (1015) FreeRTOS: FreeRTOS***ERROR*** A stack overflow in task Demo Task has been detected.
  while (1) {
    //int a = 1;
    int arr[100];
    // This for cycle is so just that the compiler does not optimize out the allocated arr[].
    for (int i = 0; i < 100; i++) {
      arr[i] = 1;
    }
    Serial.println(arr[0]);
    printMemoryInfo();
    //vTaskDelay(100 / portTICK_PERIOD_MS); // TODO: find out why this prevent guru mediation error.
  }
}

// In this task we continously claim memory from the heap, which will cause th
void heapNoFreeTask(void* param) {
  while (1) {
    int *ptr = (int*)pvPortMalloc(1024 * sizeof(int));
    // This for cycle is so just that the compiler does not optimize out the allocated *ptr.
    for (int i = 0; i < 1024; i++) {
      ptr[i] = 2;
    }
    printMemoryInfo();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void heapWithFreeTask(void* param) {
  while(1) {
    int *ptr = (int*)pvPortMalloc(1024 * sizeof(int));
    if (ptr == NULL) {
      Serial.println("Not enough heap.");
    }
    else {
      // This for cycle is so just that the compiler does not optimize out the allocated *ptr.
      for (int i = 0; i < 1024; i++) {
        ptr[i] = 2;
      }
    }
    printMemoryInfo();
    vPortFree(ptr);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(DEFAULT_SERIAL_BAUD);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("--FreeRTOS Memory Demo--");

  xTaskCreatePinnedToCore(
    heapNoFreeTask,
    "Demo Task",
    DEFAULT_STACK_SIZE,
    NULL,
    DEFAULT_STACK_PRIORITY,
    NULL,
    app_cpu);

  vTaskDelete(NULL);

}

void loop() {}
