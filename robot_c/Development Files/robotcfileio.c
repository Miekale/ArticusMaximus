
// TFileHandle rather than ifstream


//TFileHandle fin;
//bool fileOkay = openReadPC(fin, "num.txt");
//5 Read functions
//all return booleans to check if read was successful
//    readCharPC(TFileHandle, charVariable);
//    readTextPC(TFileHandle, stringVariable);
//    readIntPC(TFileHandle, intVar);
//    readFloatPC(TFileHandle, floatVar)

// Example: read values from file and display on screen
// need to download file to rc file directory

// Example code to read file
// Format: file with 1st line indicating how many numbers after the first line
#include "PC_FileIO.c"

task main() {
    TFileHandle fin;
    bool fileOkay = openReadPC(fin, "num.txt");

    if (!fileOkay) {
        displayString(5, "Error!");
        wait1Msec(500);
    }
    else {
        int numValue = 0;
        readIntPC(fin, numValue);

        for (int line=0; line < numValue; line++)
        {
            float value = 0;
            readFloat(fin, value);

        }
    }
}