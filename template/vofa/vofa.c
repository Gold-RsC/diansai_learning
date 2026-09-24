#include "vofa.h"


size_t Vofa_JustFloat(float* _data, size_t _num) {
    return Vofa_Printf("%*s\x00\x00\x80\x7F", _num * sizeof(_data[0]), (char*)_data);
}
