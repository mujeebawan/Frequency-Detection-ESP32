#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "ESPAsyncWebServer.h"
#include <arduinoFFT.h>
#include <driver/i2s.h>

// Wi-Fi Configuration
const char *ssid = "YOUR-SSID";
const char *password = "YOUR-PASSWORD";

// Server Configuration
AsyncWebServer server(80);
StaticJsonDocument<250> jsonDocument;
char buffer[250];

// FFT Configuration
#define I2S_WS 25               // I2S Word Select pin (LRCLK)
#define I2S_SD 32               // I2S Serial Data pin (DOUT)
#define I2S_SCK 33              // I2S Clock pin (BCLK)
#define BUTTON_PIN 35           // GPIO for the button

#define SAMPLING_FREQUENCY 40000 // Sampling frequency (40 kHz)
#define SHORT_SAMPLES 1024       // For 1-second FFT
#define LONG_SAMPLES 2048        // For 4-second FFT (reduced from 4096)

float short_real[SHORT_SAMPLES];
float short_imag[SHORT_SAMPLES];
float long_real[LONG_SAMPLES];
float long_imag[LONG_SAMPLES];

ArduinoFFT<float> shortFFT = ArduinoFFT<float>(short_real, short_imag, SHORT_SAMPLES, SAMPLING_FREQUENCY, true);
ArduinoFFT<float> longFFT = ArduinoFFT<float>(long_real, long_imag, LONG_SAMPLES, SAMPLING_FREQUENCY, true);

volatile bool shortFFTTriggered = false; // Flag for short FFT task
volatile bool longFFTTriggered = false;  // Flag for long FFT task

volatile double detectedShortFreq = 0.0;
volatile double detectedLongFreq = 0.0;

portMUX_TYPE fftMux = portMUX_INITIALIZER_UNLOCKED;

// I2S Configuration
void i2sSetup() {
  i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLING_FREQUENCY,
      .bits_per_sample = i2s_bits_per_sample_t(I2S_BITS_PER_SAMPLE_16BIT),
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 4,
      .dma_buf_len = 512, // Reduced from 1024 to conserve memory
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0};
  i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_SCK,
      .ws_io_num = I2S_WS,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = I2S_SD};

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}

// Interrupt Service Routine for button press
void IRAM_ATTR onButtonPress() {
  portENTER_CRITICAL(&fftMux);
  shortFFTTriggered = true; // Trigger short FFT
  longFFTTriggered = true;  // Trigger long FFT
  portEXIT_CRITICAL(&fftMux);
}

// Function to sample I2S data
void sampleI2S(int samples, float *real, float *imag) {
  int16_t i2s_data[samples];
  size_t bytes_read;
  i2s_read(I2S_NUM_0, i2s_data, samples * sizeof(int16_t), &bytes_read, portMAX_DELAY);

  if (bytes_read == 0) {
    Serial.println("I2S read error: No data received");
    return;
  }

  for (int i = 0; i < samples; i++) {
    real[i] = (float)i2s_data[i];
    imag[i] = 0.0; // Initialize imaginary part to 0
  }
}

// Task to process 1-second FFT
void processShortFFT(void *parameter) {
  while (true) {
    bool triggered = false;

    portENTER_CRITICAL(&fftMux);
    if (shortFFTTriggered) {
      triggered = true;
      shortFFTTriggered = false;
    }
    portEXIT_CRITICAL(&fftMux);

    if (triggered) {
      digitalWrite(2,HIGH);
      Serial.println("Processing Short FFT...");
      memset(short_real, 0, sizeof(short_real));
      memset(short_imag, 0, sizeof(short_imag));

      sampleI2S(SHORT_SAMPLES, short_real, short_imag);

      if (shortFFT.majorPeak() > 0) {
        shortFFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
        shortFFT.compute(FFTDirection::Forward);
        shortFFT.complexToMagnitude();
        detectedShortFreq = shortFFT.majorPeak();
      
        Serial.printf("Short FFT Frequency: %.2f Hz\n", detectedShortFreq);
      } else {
        Serial.println("Invalid data for Short FFT.");
      }
    }
    digitalWrite(2,LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Process every 1 second
  }
}

// Task to process 4-second FFT
void processLongFFT(void *parameter) {
  while (true) {
    bool triggered = false;

    portENTER_CRITICAL(&fftMux);
    if (longFFTTriggered) {
      triggered = true;
      longFFTTriggered = false;
    }
    portEXIT_CRITICAL(&fftMux);

    if (triggered) {
      digitalWrite(2,HIGH);
      Serial.println("Processing Long FFT...");
      memset(long_real, 0, sizeof(long_real));
      memset(long_imag, 0, sizeof(long_imag));

      sampleI2S(LONG_SAMPLES, long_real, long_imag);

      if (longFFT.majorPeak() > 0) {
        longFFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
        longFFT.compute(FFTDirection::Forward);
        longFFT.complexToMagnitude();
        detectedLongFreq = longFFT.majorPeak();
        
        Serial.printf("Long FFT Frequency: %.2f Hz\n", detectedLongFreq);
        if ((detectedLongFreq > 3000) && (detectedLongFreq<3500))
        {
          Serial.print("Fire Alarm");
        }
      } else {
        Serial.println("Invalid data for Long FFT.");
      }
    }
    digitalWrite(2,LOW);
    vTaskDelay(4000 / portTICK_PERIOD_MS); // Process every 4 seconds
  }
}

// HTTP Server Setup
void setupRouting() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    jsonDocument.clear();
    jsonDocument["shortFreq"] = detectedShortFreq;
    jsonDocument["longFreq"] = detectedLongFreq;
    serializeJson(jsonDocument, buffer);
    request->send(200, "application/json", buffer);
  });

  server.begin();
  Serial.println("HTTP Server started.");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting setup...");
  
  // Setup button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(2,OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButtonPress, FALLING);

  // I2S Setup
  i2sSetup();
  Serial.printf("Free heap after I2S setup: %d bytes\n", ESP.getFreeHeap());

  // Wi-Fi Setup
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to Wi-Fi...");
    delay(1000);
  }
  Serial.println("Wi-Fi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // HTTP Server Setup
  setupRouting();
  Serial.printf("Free heap after server setup: %d bytes\n", ESP.getFreeHeap());

  // Start FFT Tasks
  if (xTaskCreatePinnedToCore(processShortFFT, "Short FFT", 4096, NULL, 1, NULL, 1) == pdPASS) {
    Serial.println("Short FFT task created successfully.");
  } else {
    Serial.println("Failed to create Short FFT task.");
  }

  if (xTaskCreatePinnedToCore(processLongFFT, "Long FFT", 8192, NULL, 1, NULL, 1) == pdPASS) {
    Serial.println("Long FFT task created successfully.");
  } else {
    Serial.println("Failed to create Long FFT task.");
  }
}

void loop() {
  // Main loop does nothing; tasks handle functionality
}


