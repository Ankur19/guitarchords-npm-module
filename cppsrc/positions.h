#ifndef POSITIONS_H
#define POSITIONS_H

#include "tuning.h"

const std::string majorName = "MAJOR";
const std::string minorName = "MINOR";
const std::string frets[24] = {"A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
const std::string noteTypes[3] = {majorName, minorName};
const std::map<std::string, std::string> scaleMap = {std::pair<std::string, std::string>(majorName, "WWHWWWH"), std::pair<std::string, std::string>(minorName, "WWHWWWH")};
const std::vector<int> majorPositions = {1, 3, 5};
const std::vector<int> minorPositions = {1, 3, 5};
const std::map<std::string, std::vector<int>> notePositions = {std::pair<std::string, std::vector<int>>(majorName, majorPositions), std::pair<std::string, std::vector<int>>(minorName, minorPositions)};

class Positions
{
public:
    static void Init(Napi::Env env, Napi::Object exports);
    static Napi::Value getPositions(const Napi::CallbackInfo &info);

private:
    static void getNoteGrid(std::vector<std::string> *allNotes, unsigned int numStrings, std::string *tuning);
    static void getScaleNotes(std::string scaleType, std::string note, std::vector<std::string> *noteArray);
    static void getChordShapes(std::vector<std::string> *allNotes, std::string chordType, std::string chord, unsigned int numStrings, unsigned int startFret, unsigned int width, std::vector<std::string> *arrayToFill);
    static int isValidPosition(std::string currentNote, std::vector<std::string> chordNotes);
    static void recursiveNoteSearch(unsigned int currentString, unsigned int *numStrings, std::map<int, int *> *validNotesMap, std::vector<std::string> *chordNotes, std::vector<unsigned int> pendingStrings, std::vector<std::string> pendingNotes, std::map<int, int> validNotePositions, unsigned int *width, std::vector<std::string> *arrayToFill);
    static void addValidChord(unsigned int *numStrings, std::map<int, int> validNotePositionsNew, std::vector<std::string> *chordNotes, std::vector<std::string> *arrayToFill);
};
#endif