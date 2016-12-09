#include "mbed.h"
#include "SDFileSystem.h"
#include "wave_player.h"
#include "uLCD_4DGL.h"
#include "PinDetect.h"
#include "math.h"
#include <vector>
#include <string>

uLCD_4DGL uLCD(p28, p27, p11);
SDFileSystem sd(p5, p6, p7, p8, p12, "sd"); //SD card
AnalogOut DACout(p18);

// use class to setup pushbuttons pins
PinDetect pb1(p25);
PinDetect pb2(p24);
PinDetect pb3(p23);
PinDetect pb4(p21);

//DigitalIn _cd(p12);


DigitalOut myled(LED1);


volatile bool play;
volatile int index;
volatile int volume;
int num = 0;
wave_player waver(&DACout);

vector<string> filenames; //filenames are stored in a vector string

void read_file_names(char *dir)
{
    DIR *dp;
    struct dirent *dirp;
    dp = opendir(dir);
    //read all directory and file names in current directory into filename vector
    while((dirp = readdir(dp)) != NULL) {
        filenames.push_back(string(dirp->d_name));
        num++;
    }
    num= num;
}

void pb1_hit_callback (void) //scroll forward
{
    if (play == false) {
        index = ((index + 1) % num);
        uLCD.locate(0,0);
        //string str = filenames[index].substr(0,filenames[index].length()- 4);
        uLCD.printf("%s\n", filenames[index].substr(0,filenames[index].length()- 4));
    }
}
// Callback routine is interrupt activated by a debounced pb1 hit
void pb2_hit_callback (void) //scrolls backward
{
    if (play == false) {
        index--;
        if(index < 0)
            index = num-1;
    }
}
// Callback routine is interrupt activated by a debounced pb2 hit
void pb3_hit_callback (void) //pause/play
{
    play=!play;
    //0->wav_player.play(slices);
}
// Callback routine is interrupt activated by a debounced pb4 hit
void pb4_hit_callback (void) //volume up
{

    volume = (volume + 1) % 16;
}


int main()
{
    uLCD.baudrate(3000000);
    // uLCD.cls();
// Use internal pullups for the three pushbuttons
    pb1.mode(PullUp);
    pb2.mode(PullUp);
    pb3.mode(PullUp);
    pb4.mode(PullUp);
    // Delay for initial pullup to take effect
    wait(.01);
    // Setup Interrupt callback functions for a pb hit
    pb1.attach_deasserted(&pb1_hit_callback);
    pb2.attach_deasserted(&pb2_hit_callback);
    pb3.attach_deasserted(&pb3_hit_callback);
    pb4.attach_deasserted(&pb4_hit_callback);
    // Start sampling pb inputs using interrupts
    pb1.setSampleFrequency();
    pb2.setSampleFrequency();
    pb3.setSampleFrequency();
    pb4.setSampleFrequency();
    // pushbuttons now setup and running

    // detect sd card
    while (sd.SD_inserted() == false) {
        uLCD.cls();
        uLCD.printf("Insert SD card");
        if (sd.SD_inserted()==true) {
            //uLCD.cls();
            break;
        }
        wait(0.2);
    }
    //while (sd.SD_inserted() == true){
    uLCD.cls();
    myled = !myled;
    //uLCD.printf("SD card detected!");
// read file names into vector of strings
    read_file_names("/sd/myMusic");
// print filename strings from vector using an iterator

//for(vector<string>::iterator it=filenames.begin(); it < filenames.end(); it++)

    while (sd.SD_inserted()==true) {

        if(play == true) {
            string str2 = filenames[index].substr(0,filenames[index].length()-4);
            uLCD.locate(0,1);
            uLCD.printf("Song Playing\n");
            uLCD.locate(0,0);
            uLCD.printf("%s\n",str2);
            FILE *wave_file;
            string filepath = "/sd/mymusic/";
            wave_file = fopen( (filepath+filenames[index]).c_str(), "r");
            waver.play(wave_file, &play, &volume);
            if (feof(wave_file)) { // when playback ends file will close and play will equal false
                fclose(wave_file);
                uLCD.cls();
                play=!play;
            }
        }

        // go back to menu
        if(play == false) {

            string str2 = filenames[index].substr(0,filenames[index].length()- 4);
            uLCD.locate(0,1);
            uLCD.cls();
            uLCD.locate(0,0);
            uLCD.printf("%s\n",str2);
        }

        wait(0.3);
    }

}