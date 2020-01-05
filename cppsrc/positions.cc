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
    unsigned int startFret;
    unsigned int width;
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
    //validations pending
    startFret = Napi::Number(env, info[3]).Int32Value();
    width = Napi::Number(env, info[4]).Int32Value();
    // validations pending end

    const unsigned int numStrings = currentTune->getNumStrings();

    std::string *tuning = currentTune->Val();
    //contains allNotes in the current guitar(<6 notes in each fret> * <12 frets>)
    std::vector<std::string> allNotes;
    getNoteGrid(&allNotes, numStrings, tuning);
    //once we have the notes. Find the chord positions
    std::vector<std::string> *arrayToFill = new std::vector<std::string>();
    getChordShapes(&allNotes, type, rootNote, numStrings, startFret, width, arrayToFill);

    if (arrayToFill->size() == 0)
    {
        return env.Null();
    }
    Napi::Array returnArray = Napi::Array::New(env, arrayToFill->size());
    for (unsigned int i = 0; i < arrayToFill->size(); i++)
    {
        returnArray[i] = Napi::Value::From(env, arrayToFill->at(i));
    }

    return returnArray;
};

void Positions::Init(Napi::Env env, Napi::Object exports)
{
    //keeps the object in heap
    Napi::HandleScope scope(env);

    exports.Set("getPositions", Napi::Function::New(env, getPositions));
}

void Positions::getNoteGrid(std::vector<std::string> *allNotes, unsigned int numStrings, std::string *tuning)
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
            (*allNotes).push_back(frets[startPositions[j] + fret]);
        }
    }
}

void Positions::getScaleNotes(std::string scaleType, std::string note, std::vector<std::string> *noteArray)
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
    for (unsigned int i = 0; i < noteLength.size(); i++)
    {
        (*noteArray).push_back(frets[currentPosition]);
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

void Positions::getChordShapes(std::vector<std::string> *allNotes, std::string chordType, std::string chord, unsigned int numStrings, unsigned int startFret, unsigned int width, std::vector<std::string> *arrayToFill)
{
    //noteArray contains all the notes in the scale
    std::vector<std::string> *noteArray = new std::vector<std::string>();
    getScaleNotes(chordType, chord, noteArray);

    //chordNotes contains the notes that form the chord. we have to subtract -1 since positions in the constant start from 1
    std::vector<std::string> chordNotes;
    for (unsigned int i = 0; i < notePositions.at(chordType).size(); i++)
    {
        chordNotes.push_back((*noteArray)[notePositions.at(chordType).at(i) - 1]);
    }

    //make a list of string numbers
    std::vector<int> strings;
    for (unsigned int i = 0; i < numStrings; i++)
        strings.push_back(i);

    //make a map of notes in each guitar string. valid notes would indicate the index of the note in chordNotes else
    std::map<int, int *> validNotesMap;

    for (unsigned int i = 0; i < strings.size(); i++)
    {
        //width + 1 because there can be open notes
        validNotesMap.insert(std::pair<int, int *>(i, new int[width + 1]));
        for (unsigned int j = 0; j <= width; j++)
        {
            if (j == 0)
            {
                validNotesMap.at(i)[j] = isValidPosition((*allNotes)[i], chordNotes);
            }
            else
            {
                validNotesMap.at(i)[j] = isValidPosition((*allNotes)[(startFret + j - 1) * numStrings + i], chordNotes);
            }
        }
    }

    //now we have the valid notes map
    //only thing pending is to find all the combinations of the chordNotes. Playing them will be fun

    std::vector<unsigned int> pendingStrings;
    for (unsigned int i = 0; i < numStrings; i++)
    {
        pendingStrings.push_back(i);
    }
    std::vector<std::string> pendingNotes = chordNotes;
    std::map<int, int> validNotePositions;

    recursiveNoteSearch(pendingStrings[0], &numStrings, &validNotesMap, &chordNotes, pendingStrings, pendingNotes, validNotePositions, &width, arrayToFill);
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
void Positions::recursiveNoteSearch(unsigned int currentString, unsigned int *numStrings, std::map<int, int *> *validNotesMap, std::vector<std::string> *chordNotes, std::vector<unsigned int> pendingStrings, std::vector<std::string> pendingNotes, std::map<int, int> validNotePositions, unsigned int *width, std::vector<std::string> *arrayToFill)
{
    if (pendingStrings.size() < pendingNotes.size() || pendingStrings.size() == 0)
    {
        return;
    }
    else
    {
        bool found = false;
        //when strings are pending
        for (unsigned int i = 0; i <= (*width); i++)
        {
            if ((*validNotesMap).at(currentString)[i] > -1 && (*validNotesMap).at(currentString)[i] < (int)(*chordNotes).size())
            {
                //if atleast one valid note then donot remove from pendingStrings. else remove
                found = true;
                //make new validNotePosition
                std::map<int, int> *validNotePositionsNew = new std::map<int, int>;
                for (unsigned int j = 0; j < validNotePositions.size(); j++)
                {
                    validNotePositionsNew->insert(std::pair<int, int>(j, validNotePositions.at(j)));
                };
                validNotePositionsNew->insert(std::pair<int, int>(currentString, (*validNotesMap).at(currentString)[i]));

                //make new pendingStrings
                std::vector<unsigned int> *pendingStringsNew = new std::vector<unsigned int>;
                for (unsigned int j = 0; j < pendingStrings.size(); j++)
                {
                    pendingStringsNew->push_back(pendingStrings.at(j));
                }
                for (unsigned int j = 0; j < pendingStringsNew->size(); j++)
                {
                    if (pendingStringsNew->at(j) == currentString)
                    {
                        pendingStringsNew->erase(pendingStringsNew->begin() + j);
                        break;
                    }
                }
                //make new pendingNotes
                std::vector<std::string> *pendingNotesNew = new std::vector<std::string>;
                for (unsigned int j = 0; j < pendingNotes.size(); j++)
                {
                    pendingNotesNew->push_back(pendingNotes.at(j));
                }
                for (unsigned int j = 0; j < pendingNotesNew->size(); j++)
                {
                    if (pendingNotesNew->at(j).compare((*chordNotes)[(*validNotesMap).at(currentString)[i]]) == 0)
                    {
                        pendingNotesNew->erase(pendingNotesNew->begin() + j);
                        break;
                    }
                }
                //this means that note is valid
                if (pendingNotesNew->size() == 0)
                {
                    addValidChord(numStrings, *validNotePositionsNew, chordNotes, arrayToFill);
                }
                if (pendingStringsNew->size() > 0)
                {
                    recursiveNoteSearch(pendingStringsNew->at(0), numStrings, validNotesMap, chordNotes, *pendingStringsNew, *pendingNotesNew, *validNotePositionsNew, width, arrayToFill);
                }
            }
        }
        if (!found)
        {
            std::vector<unsigned int> *pendingStringsNew = new std::vector<unsigned int>;
            for (unsigned int j = 0; j < pendingStrings.size(); j++)
            {
                pendingStringsNew->push_back(pendingStrings.at(j));
            }
            for (unsigned int j = 0; j < pendingStringsNew->size(); j++)
            {
                if (pendingStringsNew->at(j) == currentString)
                {
                    pendingStringsNew->erase(pendingStringsNew->begin() + j);
                    break;
                }
            }
            if (pendingStrings.size() > 0)
            {
                recursiveNoteSearch(pendingStrings[0], numStrings, validNotesMap, chordNotes, *pendingStringsNew, pendingNotes, validNotePositions, width, arrayToFill);
            }
        }
        pendingStrings.clear();
        pendingNotes.clear();
        validNotePositions.clear();
    }
}
void Positions::addValidChord(unsigned int *numStrings, std::map<int, int> validNotePositionsNew, std::vector<std::string> *chordNotes, std::vector<std::string> *arrayToFill)
{
    std::string pattern = "";
    for (unsigned int i = 0; i < (*numStrings); i++)
    {
        if (validNotePositionsNew.find(i) == validNotePositionsNew.end())
        {
            //not found
            pattern += "X";
        }
        else
        {
            pattern += (*chordNotes)[validNotePositionsNew.at(i)];
        }
    }
    (*arrayToFill).push_back(pattern);
}