#include <Arduino.h>
#include <SPI.h>
#include <NeoPixelBus.h>

const uint16_t numLeds = 16;
const uint8_t pinWS = PIN_PA7;

NeoPixelBus<NeoGrbFeature, NeoWs2812Method> strip(numLeds, pinWS);


void setup() {
	strip.Begin();
	strip.Show();

  //digitalWrite(pinWS, HIGH);
}

void loop()
{   
 //Loading
	for(uint16_t i = 0; i < numLeds; i++){
		for(uint16_t j = 0; j < numLeds; j++){
			strip.SetPixelColor(j, RgbColor(0,0,0));
      strip.SetPixelColor(j+1, RgbColor(0,0,0));
      strip.SetPixelColor(j+2, RgbColor(0,0,0));
      strip.SetPixelColor(j+3, RgbColor(0,0,0));
		}
		strip.SetPixelColor(i, RgbColor(255,100,0));
    strip.SetPixelColor(i+1, RgbColor(255,100,0));
    strip.SetPixelColor(i+2, RgbColor(255,100,0));
    strip.SetPixelColor(i+3, RgbColor(255,100,0));
		strip.Show();
		delay(1000/16);
	}		

  //Płynna zmiana kolorów - autonomia

 RgbColor colors[] = {
    RgbColor(255, 255, 0), // Żółty
    RgbColor(255, 165, 0), // Pomarańczowy
    RgbColor(255, 0, 0),   // Czerwony
    RgbColor(128, 0, 128), // Fioletowy
    RgbColor(0, 0, 255),   // Niebieski
    RgbColor(0, 255, 0)    // Zielony
  };

  const int colorCount = sizeof(colors) / sizeof(colors[0]);

  for (uint16_t colorIndex = 0; colorIndex < colorCount; colorIndex++) {
    RgbColor startColor = colors[colorIndex];
    RgbColor endColor = colors[(colorIndex + 1) % colorCount];

    for (int j = 0; j < 256; j++) {
      float progress = j / 255.0;
      RgbColor blendedColor = RgbColor::LinearBlend(startColor, endColor, progress);

      for (uint16_t i = 0; i < numLeds; i++) {
        strip.SetPixelColor(i, blendedColor);
      }
      strip.Show();
      delay(5); 
    }
  }

//efekt migotania - rover
  for (int j = 0; j < 10; j++) {
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < numLeds; i += 3) {
        strip.SetPixelColor(i + q, RgbColor(25, 100, 10)); 
      }
      strip.Show();
      delay(100);

      for (uint16_t i = 0; i < numLeds; i += 3) {
        strip.SetPixelColor(i + q, RgbColor(0, 0, 0));
      }
    }
  }

//fala - manipulator
  for (uint16_t j = 0; j < numLeds * 6; j++) {
    for (uint16_t i = 0; i < numLeds; i++) {
      float brightness = sin((i + j) * PI / numLeds);
      RgbColor color = RgbColor(240, 100, 50 * brightness);

      strip.SetPixelColor(i, color);
    }
    strip.Show();
    delay(40);
  }

//estop
for(uint16_t k=0; k<4;k++){
  for(uint16_t i=0; i<numLeds;i++){
    strip.SetPixelColor(i, RgbColor(255,0,0));
    strip.Show();
   delay(1);
  }
  delay(200);
  for (uint16_t j = 0; j < numLeds; j++){
 strip.SetPixelColor(j,RgbColor(0,0,0));
    strip.Show();
    delay(1);
  }
  delay(200);
}
}