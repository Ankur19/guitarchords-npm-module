#ifndef TUNING_H
#define TUNING_H

#include "napi.h"
#include <string>
#include <map>

class Tuning : public Napi::ObjectWrap<Tuning>
{
public:
    static void Init(Napi::Env env, Napi::Object exports);
    static Napi::Object NewInstance(Napi::Array arg);
    Tuning(const Napi::CallbackInfo &info);
    std::string *Val() const { return tune_; };
    bool isValid() const { return valid_; };
    unsigned int getNumStrings() const { return numStrings_; };

private:
    static Napi::FunctionReference tuningFuncRef;
    const std::string notes[12] = {"A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
    std::string *tune_ = new std::string[8];
    bool valid_;
    unsigned int numStrings_;
};

#endif