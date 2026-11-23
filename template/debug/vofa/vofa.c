#include "vofa.h"


size_t Vofa_JustFloat(Float_t* _data, size_t _num) {
    size_t length        = 0;
    const uint8_t* bytes = (const uint8_t*)_data;
    for (size_t i = 0; i < _num; ++i) {
        length += Vofa_Printf("%c%c%c%c",
                              bytes[i * sizeof(Float_t) + 0],
                              bytes[i * sizeof(Float_t) + 1],
                              bytes[i * sizeof(Float_t) + 2],
                              bytes[i * sizeof(Float_t) + 3]);
    }

    length += Vofa_Printf("%c%c%c%c", 0x00, 0x00, 0x80, 0x7F);

    return length;
}
