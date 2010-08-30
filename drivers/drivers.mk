C_SOURCE+= \
		drivers/gpio/gpio.c \
		drivers/emac/emac.c \
		drivers/rtc/rtc.c \
		drivers/wdt/wdt.c

CXX_SOURCE+= \
		drivers/uart/uart.cpp \
		drivers/gpdma/gpdma.cpp \
		drivers/gpdma/dma_memcpy.cpp
