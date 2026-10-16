#if defined(ARDUINO_ARCH_STM32)
  #undef USB_MANUFACTURER
  #undef USB_PRODUCT
  #define USB_MANUFACTURER ".axs instruments"
  #define USB_PRODUCT "MC8P"
#endif