#include "tuning.h"
#include <napi.h>

Tuning::Tuning(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<Tuning>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    //allow tunings for 4 strings up to 8 strings only
    if (info.Length() < 4 || info.Length() > 8)
    {
        Napi::TypeError::New(env, "\nWrong number of arguments.!! Minimum strings required is 4 (ukelele) and maximum allowed is 8.\n")
            .ThrowAsJavaScriptException();
        this->valid_ = false;
    }
    else
    {
        //set valid to true first
        this->valid_ = true;
        this->numStrings_ = info.Length();
        for (unsigned int i = 0; i < info.Length() && this->valid_; i++)
        {
            for (int j = 0; j < 12; j++)
            {
                std::string input = Napi::String(env, info[i]).Utf8Value();
                int equal = notes[j].compare(input);
                if (equal == 0)
                {
                    this->tune_[i] = notes[j];
                    break;
                }
                else if (equal != 0 && j + 1 == 12)
                {
                    this->valid_ = false;
                }
            }
        }
        if (!this->valid_)
        {
            Napi::TypeError::New(env, "\nWrong Notes entered..!! Valid notes are A to G including #'s and excluding B# and E# which are not notes.\n")
                .ThrowAsJavaScriptException();
        }
    }
}

Napi::FunctionReference Tuning::tuningFuncRef;

void Tuning::Init(Napi::Env env, Napi::Object exports)
{
    //keeps the object in heap
    Napi::HandleScope scope(env);
    //define the tuning function
    Napi::Function tuningFunc = DefineClass(env, "Tuning", {});

    tuningFuncRef = Napi::Persistent(tuningFunc);
    tuningFuncRef.SuppressDestruct();

    exports.Set("Tuning", tuningFunc);
}

Napi::Object Tuning::NewInstance(Napi::Array arg)
{
    Napi::Object obj = tuningFuncRef.New({arg});
    return obj;
}
