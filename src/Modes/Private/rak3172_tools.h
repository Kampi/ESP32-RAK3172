 /*
 * rak3172_tools.h
 *
 *  Copyright (C) Daniel Kampert, 2023
 *	Website: www.kampis-elektroecke.de
 *  File info: Tools collection for RAK3172 driver.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Errors and commissions should be reported to DanielKampert@kampis-elektroecke.de
 */

#ifndef RAK3172_TOOLS_H_
#define RAK3172_TOOLS_H_

#include "rak3172_defs.h"

/** @brief          Convert a hex string into a decimal buffer.
 *  @param Hex      Input hex string
 *  @param p_Buffer Output buffer
 *                  NOTE: The size of the buffer must be large enough to carry all values from the input string!
 */
void RAK3172_Tools_Hex2ASCII(std::string Hex, uint8_t* const p_Buffer);

#endif /* RAK3172_TOOLS_H_ */