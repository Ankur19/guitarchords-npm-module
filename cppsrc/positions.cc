#include "positions.h"

Napi::Value Positions::getPositions(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Tuning *currentTune;
    std::string rootNote;
    std::string type;
    //input validations
    if (info[0].IsObject())
    {
        currentTune = Napi::ObjectWrap<Tuning>::Unwrap(info[0].As<Napi::Object>());
    }
    else
    {
        Napi::TypeError::New(env, "\n First argument must be a tuning object. <guitarchords::Tuning()> .\n")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    if (info[1].IsString())
    {
        rootNote = info[1].As<Napi::String>().Utf8Value();
        bool found = false;
        for (unsigned int i = 0; i < 12; i++)
        {
            if (frets[i].compare(rootNote) == 0)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Napi::TypeError::New(env, "\n Second Argument must be a Chord name in capital letters. Chord not found. \n")
                .ThrowAsJavaScriptException();
            return env.Null();
        }
    }
    else
    {
        Napi::TypeError::New(env, "\n Second Argument must be a Chord name in capital letters. \n")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    if (info[2].IsString())
    {
        type = info[2].As<Napi::String>().Utf8Value();
        bool found = false;
        for (unsigned int i = 0; i < 3; i++)
        {
            if (type.compare(noteTypes[i]) == 0)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Napi::TypeError::New(env, "\n Third Argument must be one of MAJOR, MINOR, 7th. \n")
                .ThrowAsJavaScriptException();
            return env.Null();
        }
    }
    else
    {
        Napi::TypeError::New(env, "\n Third Argument must be a Chord Type in capital letters. \n")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    const unsigned int numStrings = currentTune->getNumStrings();

    std::string tuning[numStrings];
    Napi::Array returnTuning = Napi::Array::New(env, numStrings);
    std::string *start = currentTune->Val();
    for (unsigned int i = 0; i < numStrings; i++)
    {
        tuning[i] = start[i];
        returnTuning[i] = Napi::Value::From(env, start[i]);
    }

    return returnTuning;
};

void Positions::Init(Napi::Env env, Napi::Object exports)
{
    //keeps the object in heap
    Napi::HandleScope scope(env);

    exports.Set("getPositions", Napi::Function::New(env, getPositions));
}
