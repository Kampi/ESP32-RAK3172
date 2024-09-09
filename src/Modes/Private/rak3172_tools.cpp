 /*
 * rak3172_tools.cpp
 *
 *  Copyright (C) Daniel Kampert, 2025
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

#include "rak3172_tools.h"

void RAK3172_Tools_Hex2ASCII(std::string Hex, uint8_t* const p_Buffer)
{
    size_t Offset;
    uint8_t High = 0;

    if((p_Buffer == NULL) || ((Hex.size() % 2) != 0))
    {
        return;
    }

    Offset = 0;
    for(size_t i = 0; i < Hex.size(); i++)
    {
        uint8_t Temp = 0;

        if((Hex.at(i) >= '0') && (Hex.at(i) <= '9'))
        {
            Temp = static_cast<uint8_t>(Hex.at(i)) - 48;
        }
        else if((Hex.at(i) >= 'a') && (Hex.at(i) <= 'f'))
        {
            Temp = static_cast<uint8_t>(Hex.at(i)) - 'a' + 10;
        }
        else if((Hex.at(i) >= 'A') && (Hex.at(i) <= 'F'))
        {
            Temp = static_cast<uint8_t>(Hex.at(i)) - 'A' + 10;
        }
        else
        {
            // Invalid character – abort conversion
            return;
        }

        // Save the high byte.
        if((i % 2) == 0)
        {
            High = Temp;
        }
        // Save the low byte and push the result to the string.
        else
        {
            *(p_Buffer + (Offset++)) = static_cast<uint8_t>((High << 0x04) + Temp);
        }
    }
}