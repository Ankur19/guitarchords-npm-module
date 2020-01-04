#include "positions.h"

/*input:
 * <Tuning> object
 * Chord Name. One of A to G#
 * Chord Type. One of Major, Minor, 7th
 */
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
        if (!currentTune->isValid())
        {
            Napi::TypeError::New(env, "\n Not a valid tuning .\n")
                .ThrowAsJavaScriptException();
            return env.Null();
        }
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

    Napi::Array returnTuning = Napi::Array::New(env, numStrings);
    std::string *tuning = currentTune->Val();
    for (unsigned int i = 0; i < numStrings; i++)
    {
        returnTuning[i] = Napi::Value::From(env, tuning[i]);
    }
    //contains allNotes in the current guitar(<6 notes in each fret> * <12 frets>)
    std::string *allNotes = new std::string[numStrings * 12];
    getNoteGrid(allNotes, numStrings, tuning);
    //once we have the notes. Find the chord positions
    getChordShapes(allNotes, type, rootNote, numStrings, 1, 3);
    return returnTuning;
};

void Positions::Init(Napi::Env env, Napi::Object exports)
{
    //keeps the object in heap
    Napi::HandleScope scope(env);

    exports.Set("getPositions", Napi::Function::New(env, getPositions));
}

void Positions::getNoteGrid(std::string *allNotes, unsigned int numStrings, std::string *tuning)
{

    unsigned int startPositions[numStrings];

    for (unsigned int i = 0; i < numStrings; i++)
    {
        for (unsigned int j = 0; j < 12; j++)
        {
            if (frets[j].compare(tuning[i]) == 0)
            {
                startPositions[i] = j;
                break;
            }
        }
    }

    for (unsigned int fret = 0; fret < 12; fret++)
    {
        for (unsigned int j = 0; j < numStrings; j++)
        {
            allNotes[fret * numStrings + j] = frets[startPositions[j] + fret];
        }
    }
}

void Positions::getScaleNotes(std::string scaleType, std::string note, std::string *noteArray)
{
    //get the scale. major or minor
    std::string noteLength = scaleMap.at(scaleType);
    //set the current position of the note
    unsigned int currentPosition = 0;
    for (unsigned int j = 0; j < 12; j++)
    {
        if (frets[j].compare(note) == 0)
        {
            currentPosition = j;
            break;
        }
    }
    for (unsigned int i = 0; i < noteLength.length(); i++)
    {
        noteArray[i] = frets[currentPosition];
        //fullnote add 2, halfnote add 1
        if (noteLength.at(i) == 'W')
        {
            currentPosition += 2;
        }
        else
        {
            currentPosition += 1;
        }
    }
}

void Positions::getChordShapes(std::string *allNotes, std::string chordType, std::string chord, unsigned int numStrings, unsigned int startFret, unsigned int width)
{
    //noteArray contains all the notes in the scale
    std::string *noteArray = new std::string[7];
    getScaleNotes(chordType, chord, noteArray);

    //chordNotes contains the notes that form the chord
    std::vector<std::string> chordNotes;
    for (unsigned int i = 0; i < notePositions.at(chordType).size(); i++)
    {
        chordNotes.push_back(noteArray[notePositions.at(chordType).at(i)]);
    }

    unsigned int startIndex = startFret * numStrings;
    //make a list of string numbers
    std::vector<int> strings;
    for (unsigned int i = 1; i <= numStrings; i++)
        strings.push_back(i);

    //make a map of notes in each guitar string. valid notes would indicate the index of the note in chordNotes else
    std::map<int, int *> validNotesMap;

    for (std::vector<int>::size_type i = 0; i != strings.size(); i++)
    {
        //width + 1 because there can be open notes
        validNotesMap.insert(std::pair<int, int *>(i, new int[width + 1]));
        for (unsigned int j = 0; j <= width; j++)
        {
            if (j == 0)
            {
                validNotesMap.at(i)[j] = isValidPosition(allNotes[i], chordNotes);
            }
            else
            {
                validNotesMap.at(i)[j] = isValidPosition(allNotes[(startFret + i - 1) * numStrings + j], chordNotes);
            }
        }
    }
}
int Positions::isValidPosition(std::string currentNote, std::vector<std::string> chordNotes)
{
    for (unsigned int i = 0; i < chordNotes.size(); i++)
    {
        if (chordNotes[i].compare(currentNote) == 0)
        {
            return i;
        }
    }
    return -1;
}
//void Positions::recursiveNoteSearch(std::string *allNotes, unsigned int startIndex, unsigned int numStrings, unsigned int width, std::vector<std::string> shapeArr, std::string validNotes[])
