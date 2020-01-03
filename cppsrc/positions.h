#ifndef POSITIONS_H
#define POSITIONS_H

#include "tuning.h"

const std::string frets[24] = {"A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
const std::string noteTypes[3] = {"MAJOR", "MINOR", "7TH"};

class Positions
{
public:
    static void Init(Napi::Env env, Napi::Object exports);
    static Napi::Value getPositions(const Napi::CallbackInfo &info);

private:
};
#endif