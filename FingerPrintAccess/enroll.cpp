#include "mbed.h"
#include "uLCD_4DGL.h"
#include "SDFileSystem.h"
#include <string>
#include "GT511C3.hpp"

DigitalIn b_ok(p15);
DigitalIn b_esc(p16);
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
}

void SCR_start_session() {
    upload_to_scanner();
    int is_done = 0;
    int selection;
    while (!is_done) {
        TFT.cls();
        printf("\nNueva Huella: Seleccione OK para empezar")
       if (b_ok == 1) {
            printf("Añadir Huella seleccionado\n");
            add_new_fingerprint();
            
        }  else {
            printf("Esperando...");
        }
    }
}


/*
 * Check if the fingerprint exists in the database
 */
void read_from_fingerprint_scanner() {
    TFT.cls();
    display_center(1, "SCAN");
    left_arrow();
    int FID = -1;
    
    //Talk to the fingerprint and get an ID back
    display_center(5, "Place finger");
    display_center(6, "on scanner!");
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
    TFT.cls()
    
    if (!is_cancelled) {
        // Identify!
        FID = finger.Identify();
     
        display_center(5, "Remove finger");
        display_center(6, "from scanner!");
        finger.WaitPress(0);
        finger.CmosLed(0);
        if(FID!=-1)
        {
            display_center(8, "Student found!");
           
            this_session[FID] = 1;
        } else {
            display_center(8, "Student not found");
        }
         wait(2);
        if (FID >= 0 && FID < group_size) {
        } else {
            debug.printf("Sorry, ID failed!");
        }
    }
}


void upload_to_scanner() {
    finger.DeleteAllIDs();
   
    int indexScanner = 0;
     
    string fileNameScan;
    char indexC[3];
    sprintf(indexC,"%d",indexScanner);
    fileNameScan = "/sd/" + string(work_id) + "/" + indexC + ".bin";
    const char* conv_fileScan = fileNameScan.c_str();
    unsigned char data[packet_size];
//  Open SD File
     
    FILE *templateFile = fopen(conv_fileScan, "r");
    if(templateFile==NULL) {
       debug.printf("File is NULL!\n");   
    }
     
    while(templateFile != NULL) {
        fread(data, sizeof(unsigned char), sizeof(data),templateFile);
        
        if(finger.SetTemplate(indexScanner,data,packet_size)) {
            indexScanner++;  
            fclose(templateFile); 
        }

        string fileNameScan;
        char indexC[3];
        sprintf(indexC,"%d",indexScanner);
        fileNameScan = "/sd/" + string(work_id) + "/" + indexC + ".bin";
        conv_fileScan = fileNameScan.c_str();
     
        debug.printf("Attempt to find file %s\n", conv_fileScan);
     
        FILE *templateFile = fopen(conv_fileScan, "r");
        if(templateFile==NULL)
        {
            debug.printf("THERE IS NO FILE!\n");
            break;   
        }
    }
}



void SCR_header() {
    TFT.cls();
    printf("******************\n");
    printf("Para empezar ponga su huella\n");
    printf("******************\n\n");
}


int db_exists(char * work_DB) {
    
    FILE *file = fopen("/sd/owner_id_file.txt", "r");

    printf("Abriendo Archivo: %s\n",filename);
    char buffer[6];
    debug.printf("Finding database\n");
    if(file==NULL)
    {
        printf("No hay archivo\n");
    }
    while (fgets(buffer, 6, file))
    {

        if (strcmp(work_idDB,buffer)==0) {
            debug.printf("Found!\n");

            fclose(file);
            display_center(11, "Authenticated,");
            display_center(12, "session starts!");
            wait(scr_switch_wait);
            return 1;
        }
    }
    fclose(file);
    printf("No se encuentra en la base")
    wait(scr_switch_wait);
    return 0;
}


void add_new_fingerprint() {
   
   string fileNameSID;
   int cancelled = 0;
   
   do {
        TFT.cls();
        printf("Asignando número...")
        char worker_id[sid_size];
        if (worker_id != 0){
            worker_id++;}
        else{ 
            worker_id =0;}
        
        wait(wait_time);
        fileNameSID = "/sd/" + string(work_id) + "/" + string(sid_file);
        const char* conv_fileSID = fileNameSID.c_str();

        int eid = get_enroll_id(conv_fileSID, worker_id);
        if (eid == -1) {
            printf("\n\nError, inténtelo de nuevo\n");
        } else {
            
            char eidc[4];
            printf(eidc, "%d",eid);
            printf("ID de Huella = %s\n",eidc);
            enroll_new_fingerprint(work_id, eidc);
            printf("\n\nHuella agregada\n");
            
        }
        wait(1);
        cancelled = is_enroll_more();
        wait(2);
    } while(!cancelled);
}

int is_enroll_more() {
    TFT.cls();
    printf("¿Desea agregar otra huella?")
    int selection;
    if (b_ok == 1)
        selection = 1;
    return selection;
}

int get_enroll_id(const char * filename, char * sid) {

    debug.printf("GetEnrollID File %s\n",filename);
    FILE *file = fopen(filename, "r");
    if(file==NULL)
    {
        debug.printf("File NULL\n");
    }
    int index = 0;
    int is_found = 0;
    char buffer[10];
    while (fgets(buffer, 10, file))
    {
        debug.printf("A\n");
        debug.printf("Buffer = %s\n",buffer);
        debug.printf("SID = %s\n",sid);
        if (strcmp(buffer,sid) == 0)
        {
            debug.printf("Found at index %d!",index);
            printf("Por favor, ponga su huella 3 VECES\n");
            fclose(file);
            is_found = 1;
            break;   
        }
        index++;    
    }    
    fclose(file);
    if(!is_found) {
        return -1;
        debug.printf("Error, no hay ID");
    } else {
        return index/2; 
    }  
}

/*
 * Agregar nueva huella
 *
 * @param 
 * @return none
 */
void enroll_new_fingerprint(char* groupIDF, char* index) {

    int EnrollID = -1;
    
    unsigned char datas[498];
    debug.printf("Trying to Enroll\n");
    finger.Enroll(EnrollID, progress);
    
    finger.RecvData(datas, 498);
    debug.printf("Index = %s, Int Index = %d",index, atoi(index));
 
    finger.SetTemplate(atoi(index),datas,packet_size);

    FILE *templateFile = fopen(conv_file, "w");
    
    if(templateFile == NULL) {
             debug.printf("Could not open file for write\n");
        }
    
    fwrite(datas,sizeof(unsigned char), sizeof(datas),templateFile);

    fclose(templateFile); 
 
    finger.CmosLed(1);
}

int progress(int status, char *msg) {
    debug.printf("%s",msg);
    return 0;
}
