#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

#include "main.h"

int main(int argc, char *argv[]) {
	if (argc != 2)  {
		printf(ANSI_COLOR_RED "ERROR: No argument or too many arguments specified.\n" ANSI_RESET);
		printf("usage: %s filename\n", argv[0]);
		exit(255);
    }
    
    printf(ANSI_BOLD "Trying to open x3v voice bank \"%s\" for reading...\n" ANSI_RESET, argv[1]);
    
    // Try to open the file.
    FILE *x3v = fopen(argv[1], "r");
    
    if(!x3v) {
		perror(ANSI_COLOR_RED "Couldn't open the sample bank for reading");
		printf(ANSI_RESET);
		exit(254);    
    }
    
  	rewind(x3v);
  	
  	// Read the first byte; the number of samples.
    char num_samples = 0x00;
    fread(&num_samples, sizeof(typeof(num_samples)), 1, x3v);
    
    printf("Found %i samples in sample bank.\n", num_samples);
    
    VoiceEntry samples[num_samples];
    
    printf(ANSI_COLOR_GREEN "Found samples at: ");
    
    // Reset all the VoiceEntry structs to defaults
    for(int i = 0; i < num_samples; i++) {
    	for(int j = 0; j < 255; j++) {
    		samples[i].description[j] = 0x00;
    	}
    }
    
    // Loop through all samples and squelch them into the arrayinator
    for(int i = 0; i < num_samples; i++) {
    	samples[i].startPos = ftell(x3v);
    	fread(samples[i].ym2612regs, sizeof(char), 25, x3v);
    	fread(&samples[i].descLength, sizeof(uint8_t), 1, x3v);
    	fread(samples[i].description, sizeof(char), samples[i].descLength, x3v);
    	
    	printf("$%X ", samples[i].startPos);
    }
    
    printf("\n" ANSI_RESET);
    
    // Create output directory.
    printf("Creating EIF output directory...\n");
    mkdir("output", 0755);
    
    if(file_exists("output") == 0) {
		perror(ANSI_COLOR_RED "Couldn't create output folder");
		printf(ANSI_RESET);
		exit(253);    
    }
    
    // Switch into the output directory.
    chdir("output");
    
    uint8_t eifVoice[29];
    for(int j = 0; j < 29; j++) {
    	eifVoice[j] = 0x00;
	} 
    
    // Maps X3V voice byte to EIF voice byte
    const int SMPSToEIFMap[29] = { 0x00, 0x01, 0x03, 0x02, 0x04, 0x15, 0x17, 0x16, 0x18, 0x05, 0x07, 0x06, 0x08, 0x09, 0x0B, 0x0A, 0x0C, 0x0D, 0x0F, 0x0E, 0x10, 0x11, 0x13, 0x12, 0x14, 0xFF, 0xFF, 0xFF, 0xFF	 };
    
    // Buffer to hold filename
    char sampleOut[255+5];  
    for(int j = 0; j < 255; j++) {
    	sampleOut[j] = 0x00;
	} 
        
    for(int i = 0; i < num_samples; i++) {
  		sprintf (sampleOut, "%s.eif", samples[i].description);
  		remove_all_chars(sampleOut, '/');
  		
    	FILE *EIFOutput = fopen(sampleOut, "w");
    	
    	if(!EIFOutput) {
			perror(ANSI_COLOR_RED "Couldn't create output file");
    		printf(ANSI_COLOR_RED "Can't create file %s!\n" ANSI_RESET, sampleOut);   		
			exit(252);
    	}
    	
    	for(int j = 0; j < 29-4; j++) {
    		eifVoice[j] =  samples[i].ym2612regs[SMPSToEIFMap[j]];
    	}
    	
    	fwrite(eifVoice, 29, sizeof(uint8_t), EIFOutput);
    	fclose(EIFOutput);
    	
    	printf("Finished writing %s.\n", sampleOut);
    }
}

void remove_all_chars(char* str, char c) {
	char *pr = str, *pw = str;

    while (*pr) {
		*pw = *pr++;
		pw += (*pw != c);
    }
    
    *pw = '\0';
}


uint8_t file_exists(const char * filename) {
	FILE *file;

	if ((file = fopen(filename, "r"))) {
		fclose(file);
		return 1;
	}
    
	return 0;
}
