#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
	#endif

	// Monotone tijd sinds boot in milliseconden (wrap toegestaan).
	uint32_t hal_millis(void);

	// Blokkerende delay in milliseconden (nu simpel; later RTOS-delay).
	void hal_delay_ms(uint32_t ms);

	#ifdef __cplusplus
}
#endif
