#ifndef __KALMAN_H__
#define __KALMAN_H__
#include <stdint.h>

#define KALMAN_WINDOW_SIZE 5

#define KALMAN_HARMONIC_GAIN_2nd 0.1f
#define KALMAN_HARMONIC_GAIN_3rd 0.1f
#define KALMAN_HARMONIC_GAIN_5th 0.05f

typedef struct {
    struct {
        float Q;
        float R;
        float harmonic_gain[3];
    } param;

    struct {
        float window[KALMAN_WINDOW_SIZE];
        uint8_t index;

    } _state;

    float X;

} Kamlman_t;

#endif
