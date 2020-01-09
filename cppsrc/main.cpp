/* cppsrc/main.cpp */
#include "napi.h"
#include "tuning.h"
#include "positions.h"

#define NODE_GYP_MODULE_NAME guitarchords
using namespace Napi;

Napi::Boolean checkValidTuning(const CallbackInfo &info)
{
  Napi::Env env = info.Env();
  Tuning *tune = Napi::ObjectWrap<Tuning>::Unwrap(info[0].As<Napi::Object>());
  return Napi::Boolean::New(env, tune->isValid());
}

Napi::Object createTuning(const Napi::CallbackInfo &info)
{
  return Tuning::NewInstance(info[0].As<Napi::Array>());
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports)
{
  Tuning::Init(env, exports);
  Positions::Init(env, exports);
  exports.Set(Napi::String::New(env, "createTuning"), Napi::Function::New(env, createTuning));
  exports.Set(Napi::String::New(env, "checkValidTuning"), Napi::Function::New(env, checkValidTuning));
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)
