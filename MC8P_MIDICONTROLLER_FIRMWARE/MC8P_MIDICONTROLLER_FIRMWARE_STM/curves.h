// =============================================================================
// curves.h — pot position (0-1023) -> CC value through a curve and min/max range
//
// Curves: NOR (linear), INV (inverted), LOG (x^2, fine at the bottom),
// ILG (1 - x^2). A min above max flips the range.
// =============================================================================
#pragma once
#include "state.h"

// Helper function to map potentiometer value with range and direction
int mapMidiValueWithParams(int rawValue, byte minVal, byte maxVal, CurveType curve) {
  // First, normalize the raw value to 0.0-1.0 range
  float normalized = (float)rawValue / 1023.0;

  // Apply curve transformation
  float transformed;

  switch (curve) {
    case CURVE_LOG:  // Logarithmic (slow start, fast end)
      // Square the value for logarithmic feel (x^2)
      // This gives fine control at low values and faster changes at high values
      transformed = normalized * normalized;
      break;

    case CURVE_ILG:  // Inverse Logarithmic (fast start, slow end)
      // Actually this should be: fast changes at low values, fine control at high values
      // Formula: 1 - (1-x)² gives fast start, slow end
      // But you want: slow descent that accelerates (fine at high, fast at low)
      // So we use: 1 - x² (inverse of LOG)
      transformed = 1.0 - (normalized * normalized);
      break;

    case CURVE_INV:  // Inverted linear
      transformed = 1.0 - normalized;
      break;

    case CURVE_NOR:  // Normal linear
      // Fall through to default
    default:
      transformed = normalized;
      break;
  }

  // Map the transformed value (0.0-1.0) to the min-max range
  int mappedValue;
  if (minVal <= maxVal) {
    // Normal range
    mappedValue = minVal + (transformed * (maxVal - minVal));
  } else {
    // Inverted range (min > max)
    mappedValue = maxVal + ((1.0 - transformed) * (minVal - maxVal));
  }

  // Constrain to valid range
  mappedValue = constrain(mappedValue, min(minVal, maxVal), max(minVal, maxVal));

  return mappedValue;
}
