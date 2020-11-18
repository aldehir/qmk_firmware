/*
 * APA102 lib V1.0a
 *
 * Controls APA102 RGB-LEDs
 * Author: Mikkel (Duckle29 on GitHub)
 *
 * Dec 22th, 2017  v1.0a Initial Version
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "apa102.h"
#include "quantum.h"

uint8_t apa102_led_brightness = APA102_DEFAULT_BRIGHTNESS;

void static apa102_start_frame(void);
void static apa102_end_frame(uint16_t num_leds);

void static apa102_send_frame(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness);
void static apa102_send_byte(uint8_t byte);

void apa102_setleds(LED_TYPE *start_led, uint16_t num_leds) {
    LED_TYPE *end = start_led + num_leds;

    apa102_start_frame();
    for (LED_TYPE *led = start_led; led < end; led++) {
        apa102_send_frame(led->r, led->g, led->b, apa102_led_brightness);
    }
    apa102_end_frame(num_leds);
}

void static apa102_init(void) {
    setPinOutput(RGB_DI_PIN);
    setPinOutput(RGB_CI_PIN);

    writePinLow(RGB_DI_PIN);
    writePinLow(RGB_CI_PIN);
}

void apa102_set_brightness(uint8_t brightness) {
    if (brightness > APA102_MAX_BRIGHTNESS) {
        apa102_led_brightness = APA102_MAX_BRIGHTNESS;
    } else if (brightness < 0) {
        apa102_led_brightness = 0;
    } else {
        apa102_led_brightness = brightness;
    }
}

void static apa102_send_frame(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness) {
    apa102_send_byte(0b11100000 | brightness);
    apa102_send_byte(blue);
    apa102_send_byte(green);
    apa102_send_byte(red);
}

void static apa102_start_frame(void) {
    apa102_init();
    apa102_send_byte(0);
    apa102_send_byte(0);
    apa102_send_byte(0);
    apa102_send_byte(0);
}

void static apa102_end_frame(uint16_t num_leds) {
    // This function has been taken from: https://github.com/pololu/apa102-arduino/blob/master/APA102.h
    // and adapted. The code is MIT licensed. I think thats compatible?
    //
    // The data stream seen by the last LED in the chain will be delayed by
    // (count - 1) clock edges, because each LED before it inverts the clock
    // line and delays the data by one clock edge.  Therefore, to make sure
    // the last LED actually receives the data we wrote, the number of extra
    // edges we send at the end of the frame must be at least (count - 1).
    //
    // Assuming we only want to send these edges in groups of size K, the
    // C/C++ expression for the minimum number of groups to send is:
    //
    //   ((count - 1) + (K - 1)) / K
    //
    // The C/C++ expression above is just (count - 1) divided by K,
    // rounded up to the nearest whole number if there is a remainder.
    //
    // We set K to 16 and use the formula above as the number of frame-end
    // bytes to transfer.  Each byte has 16 clock edges.
    //
    // We are ignoring the specification for the end frame in the APA102
    // datasheet, which says to send 0xFF four times, because it does not work
    // when you have 66 LEDs or more, and also it results in unwanted white
    // pixels if you try to update fewer LEDs than are on your LED strip.
    uint16_t iterations = (num_leds + 14) / 16;
    for (uint16_t i = 0; i < iterations; i++) {
        apa102_send_byte(0);
    }

    apa102_init();
}

#define APA102_SEND_BIT(byte, bit)               \
    do {                                         \
        writePin(RGB_DI_PIN, (byte >> bit) & 1); \
        writePinHigh(RGB_CI_PIN);               \
        writePinLow(RGB_CI_PIN);                \
    } while (0)

void static apa102_send_byte(uint8_t byte) {
    APA102_SEND_BIT(byte, 7);
    APA102_SEND_BIT(byte, 6);
    APA102_SEND_BIT(byte, 5);
    APA102_SEND_BIT(byte, 4);
    APA102_SEND_BIT(byte, 3);
    APA102_SEND_BIT(byte, 2);
    APA102_SEND_BIT(byte, 1);
    APA102_SEND_BIT(byte, 0);
}
