#include <Arduino.h>
#include <SPI.h>
#include <NeoPixelBus.h>
#include <mcp2515.h>
#include "libVescCan/VESC.hpp"

const uint16_t numLeds = 16;
const uint8_t pinWS = PIN_PA7;

// 20 - command id (status 10); 20 - vesc id (NUC)
// remember to change vesc id in both defines below!!!
#define CAN_FILTER 0x2020
#define VESC_ID_NUC 0x20

#define CAN_INT_PIN PIN_PB5

VESC_Status_10 vesc_status_10_frame = {0};

NeoPixelBus<NeoGrbFeature, NeoWs2812Method> strip(numLeds, pinWS);

MCP2515 mcp2515(PIN_PA4);

struct can_frame received_can_frame;

void led_estop();
void led_unknown();
void led_autonomy();
void led_driving();
void led_manipulator();

void zero_can_filters() {
    mcp2515.setFilterMask(MCP2515::MASK0, true, 0xFFFFFFFF);
    mcp2515.setFilterMask(MCP2515::MASK1, true, 0xFFFFFFFF);
    mcp2515.setFilter(MCP2515::RXF0, true, 0);
    mcp2515.setFilter(MCP2515::RXF1, true, 0);
    mcp2515.setFilter(MCP2515::RXF2, true, 0);
    mcp2515.setFilter(MCP2515::RXF3, true, 0);
    mcp2515.setFilter(MCP2515::RXF4, true, 0);
    mcp2515.setFilter(MCP2515::RXF5, true, 0);
}

void setup_can_filters() {
  // receive buffer 0
  // accept only one vesc id - the NUC, and only status 10
  mcp2515.setFilterMask(MCP2515::MASK0, true, 0x1FFFFFFF);
  mcp2515.setFilter(MCP2515::RXF0, true, CAN_FILTER);

  // receive buffer 1
  // accept only one vesc id - the NUC, and only status 10
  mcp2515.setFilterMask(MCP2515::MASK1, true, 0x1FFFFFFF);
  mcp2515.setFilter(MCP2515::RXF2, true, CAN_FILTER);
}

void process_vesc_frame(VESC_RawFrame* raw_frame) {
    // rover status
    if (raw_frame->command == VESC_COMMAND_STATUS_10 && raw_frame->vescID == VESC_ID_NUC) {
        VESC_ZeroMemory(&vesc_status_10_frame, sizeof(vesc_status_10_frame));
        VESC_convertRawToStatus10(&vesc_status_10_frame, raw_frame);
    }
}

void process_can_frame(can_frame frame) {
  VESC_RawFrame raw_vesc_frame = *reinterpret_cast<VESC_RawFrame*>(&frame);
  process_vesc_frame(&raw_vesc_frame);
}

void can_interrupt_handler() {
  uint8_t irq = mcp2515.getInterrupts();

  if (irq & MCP2515::CANINTF_RX0IF) {
      if (mcp2515.readMessage(MCP2515::RXB0, &received_can_frame) == MCP2515::ERROR_OK) {
          process_can_frame(received_can_frame);
      }
  }

  if (irq & MCP2515::CANINTF_RX1IF) {
      if (mcp2515.readMessage(MCP2515::RXB1, &received_can_frame) == MCP2515::ERROR_OK) {
          process_can_frame(received_can_frame);
      }
  }

  mcp2515.clearInterrupts();
}

// IMPORTANT!!! Go to can.h from the mcp2515 library and remove __attribute__((aligned(8))) from the can_frame struct
void setup() {
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  zero_can_filters();
  setup_can_filters();
  mcp2515.setNormalMode();

	strip.Begin();

  attachInterrupt(CAN_INT_PIN, can_interrupt_handler, FALLING);
}


void loop()
{  
  while (true) {
    if (vesc_status_10_frame.communicationState == VESC_STATUS_10_COMMUNICATIONSTATE_FAULTED) {
      led_unknown();
      continue;
    }
    
    switch (vesc_status_10_frame.controlMode) {
      case VESC_STATUS_10_CONTROLMODE_AUTONOMY:
        led_autonomy();
        break;
      case VESC_STATUS_10_CONTROLMODE_ESTOP:
        led_estop();
        break;
      case VESC_STATUS_10_CONTROLMODE_MANIPULATOR:
        led_manipulator();
        break;
      case VESC_STATUS_10_CONTROLMODE_ROVER:
        led_driving();
        break;
      default:
        led_unknown();
        break;
    }
  }
}

bool estop_blink = false;
void led_estop() {
    for (uint16_t i = 0; i < numLeds; i++) {
      strip.SetPixelColor(i, estop_blink ? RgbColor(255,0,0) : RgbColor(0,0,0));
    }
    strip.Show();
    estop_blink = !estop_blink;
    delay(random(40, 200));
}

uint8_t manipulator_offset = 0;
void led_manipulator() {
    for (uint16_t i = 0; i < 4; i++) {
      strip.SetPixelColor(4*i, (manipulator_offset != 0) ? RgbColor(85,107,47) : RgbColor(0, 127, 0));
      strip.SetPixelColor(4*i+1, (manipulator_offset != 1) ? RgbColor(85,107,47) : RgbColor(0, 127, 0));
      strip.SetPixelColor(4*i+2, (manipulator_offset != 2) ? RgbColor(85,107,47) : RgbColor(0, 127, 0));
      strip.SetPixelColor(4*i+3, (manipulator_offset != 3) ? RgbColor(85,107,47) : RgbColor(0, 127, 0));
    }
    strip.Show();
    manipulator_offset++;
    if (manipulator_offset > 3) manipulator_offset = 0;
    delay(200);
}

uint8_t autonomy_offset = 0;
void led_autonomy() {
    for (uint16_t i = 0; i < 4; i++) {
      strip.SetPixelColor(4*i, (autonomy_offset == 0) ? RgbColor(173,216,230) : RgbColor(0,0,0));
      strip.SetPixelColor(4*i+1, (autonomy_offset == 0) ? RgbColor(173,216,230) : RgbColor(0,0,0));
      strip.SetPixelColor(4*i+2, (autonomy_offset == 1) ? RgbColor(173,216,230) : RgbColor(0,0,0));
      strip.SetPixelColor(4*i+3, (autonomy_offset == 1) ? RgbColor(173,216,230) : RgbColor(0,0,0));
    }
    strip.Show();
    autonomy_offset++;
    if (autonomy_offset > 1) autonomy_offset = 0;
    delay(150);
}

uint8_t driving_offset = 0;
void led_driving() {
    for (uint16_t i = 0; i < 4; i++) {
      strip.SetPixelColor(4*i, (driving_offset == 0) ? RgbColor(0,100,0) : RgbColor(0,0,0));
      strip.SetPixelColor(4*i+1, (driving_offset == 1) ? RgbColor(0,100,0) : RgbColor(0,0,0));
      strip.SetPixelColor(4*i+2, (driving_offset == 0) ? RgbColor(0,100,0) : RgbColor(0,0,0));
      strip.SetPixelColor(4*i+3, (driving_offset == 1) ? RgbColor(0,100,0) : RgbColor(0,0,0));
    }
    strip.Show();
    driving_offset++;
    if (driving_offset > 1) driving_offset = 0;
    delay(200);
}

bool unknown_blink = false;
void led_unknown() {
    for (uint16_t i = 0; i < numLeds; i++) {
      strip.SetPixelColor(i, unknown_blink ? RgbColor(255,255,0) : RgbColor(0,0,0));
    }
    strip.Show();
    unknown_blink = !unknown_blink;
    delay(250);
}