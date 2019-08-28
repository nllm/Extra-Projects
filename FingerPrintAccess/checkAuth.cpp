#include "mbed.h"
#include "uLCD_4DGL.h"
#include "SDFileSystem.h"
#include <string>
#include "GT511C3.hpp"

DigitalIn b_ok(p15);
DigitalIn b_esc(p16);
DigitalOut auth_ok(p17);
ST7735_TFT TFT(p5, p6, p7, p8, p25, p24,"TFT"); // create a global TFT object for LCD Display
Serial debug(USBTX,USBRX);
SDFileSystem sd(p11, p12, p13, p14, "sd");
GT511C3 finger(p9, p10);

// ************** CONST VARIABLE **************** //
// Size
const int packet_size = 498;
const int id_size = 5;
const int sid_size = 9;
const int group_size = 200;
// Wait Time
const float scr_switch_wait = 1;
const float wait_time = 0.04;

// File Name
const char * owner_id_file = "owner_id_file.txt";
const char * sid_file = "sid_file.txt";

// VARIABLE
char work_id[id_size];
char sid_temp[sid_size];
int this_session[group_size];

//FUNCTIONS
void start_program();
void SCR_start_session();
void SCR_header();
int is_enroll_more();
void SCR_enroll_new_fingerprint();
int db_exists(char * work_idDB);
void read_from_fingerprint_scanner();
void upload_to_scanner();
int get_enroll_id(const char * filename, char * sid);
void enroll_new_fingerprint(char* groupIDF, char* index);
int progress(int status, char *msg);
void add_new_fingerprint();

int main() {
    while (1) {
        start_program();
    }
}

void start_program() {
    int is_done = 0;
    auth_ok = 0;
    TFT.set_orientation(1);
    TFT.claim(stdout);        // send stdout to the TFT display 
    TFT.background(Black);    // set background to black
    TFT.foreground(Green);    // set chars to white
    TFT.cls();
    TFT.set_font((unsigned char*) Arial12x12);  // select the font

    while (!is_done) { 
        TFT.cls();
        SCR_header();
        SCR_start_session();
        if (auth_ok != 1){
            is_done = 1;
        }
}

void SCR_start_session() {
    upload_to_scanner();
    int is_done = 0;
    int selection;
    while (!is_done) {
        TFT.cls();
        printf("\nEscanee su huella porfavor")
        read_from_fingerprint_scanner();
}


/*
 * Check if the fingerprint exists in the database
 */
void read_from_fingerprint_scanner() {
    TFT.cls();
    printf("\nEscaneando")
    int FID = -1;
    
    //Check ID of fingerprint
    finger.CmosLed(1);        
    finger.WaitPress(1);
    int is_cancelled = 0;
    while(finger.Capture(1) != 0 && !is_cancelled) {
        if (!Down) {
            is_cancelled = 1;
            finger.WaitPress(0);
            finger.CmosLed(0);
        }
    }
    TFT.cls();
    
    if (!is_cancelled) {
        // Identifying
        FID = finger.Identify();
     
        printf("\nChecando huella")
        finger.WaitPress(0);
        finger.CmosLed(0);
        if(FID!=-1)
        {
            printf( "Acceso Autorizado\nEncendiendo Máquina");
           
            this_session[FID] = 1;
            auth_ok = 1

        } else {
            printf("NO AUTORIZADO");
        }
         wait(2);
        if (FID >= 0 && FID < group_size) {
        } else {
            debug.printf("Intentelo de nuevo");
        }
    }
}
