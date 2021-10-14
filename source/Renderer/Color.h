#pragma once

#include <stdint.h>

struct Color
{
	union
	{
		uint32_t value;
		struct
		{
			uint8_t blue;
			uint8_t green;
			uint8_t red;
			uint8_t alpha;
		};
		uint8_t BGR[3];
	};
};