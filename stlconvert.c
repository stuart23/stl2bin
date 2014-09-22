#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <regex.h>
#include <stdint.h>

float ScientificToFloat(char *in_String) {
	// Loop Variables
	int Counter = 0;
	int Length = strlen(in_String) + 1;
	// Flags and signs
	int NegativeFlag = 0;
	int DecimalFlag = 0;
	int ExponentSign = 0; // -1 = Negative, 0 = None, 1 = Positive
	// Numerical Data
	int Exponent = 0;
	int FinalDivision = 1;
	long Digits = 0;
	// Loop per each character. Ignore anything weird.
	for (;Counter < Length; Counter++) {
		// Depending on the current character
		switch (in_String[Counter]) {
			// On any digit
			case '0': case '5':
			case '1': case '6':
			case '2': case '7':
			case '3': case '8':
			case '4': case '9':
				// If we haven't reached an exponent yet ("e")
				if (ExponentSign == 0) {
					// Adjust the final division if a decimal was encountered
					if (DecimalFlag) FinalDivision *= 10;
					// Add a digit to our main number
					Digits = (Digits * 10) + (in_String[Counter] - '0');
					// If we passed an "e" at some point
				} else {
					// Add a digit to our exponent
					Exponent = (Exponent * 10) + (in_String[Counter] - '0');
				}
				break;
				// On a negative sign
			case '-':
				// If we passed an 'e'
				if (ExponentSign > 0)
					// The exponent sign will be negative
					ExponentSign = -1;
					// Otherwise we are still dealing with the main number
				else
					// Set the negative flag. We will negate the main number later.
					NegativeFlag = 1;
				break;
				// If we encounter some kind of "e"
			case 'e': case 'E':
				// Set the exponent flag
				ExponentSign = 1;
				break;
				// If we encounter a period
			case '.':
				// Set the decimal flag. We will start tracking decimal depth.
				DecimalFlag = 1;
				break;
				// We gladly accept all sorts of additional garbage.
			default:
				break;
		}
	}
	// If the negative flag is set, negate the main number
	if (NegativeFlag)
		Digits = 0 - Digits;
	// If the exponent is supposed to be negative, negate it now
	if (ExponentSign < 0)
		Exponent = 0 - Exponent;
	// Return the calculated result of our observations
	return ((double)Digits / (double)FinalDivision) * (double)(pow((double)10.0f, (double)Exponent));
}

   

int main(int argc, char *argv[])
{
int filenamesize;			// The size of the string filename
int i;					// Integer used for indexing loops when counting through X,Y,Z coordinates or normal vectors
int vertexcount; 			// Counter used to find when three vertices had been written so that attribute can be written
uint32_t numberoftris;			// Counter that records number of triangles in each file
uint16_t attribute;			// Two bit identifier used by some STL programs. The attribute bytes are written for each triangle
char *filename;				// The input filename to the program, ie the ASCII STL file 
char *outname; 				// The output filename from the program, ie the BINARY STL file
char *solidname;			// The name in the ASCII STL file of the "solid"
char line[100];				// Holds each subsequent line of the input file
char gotstring[12];			// Holds the extracted substring identified using regexp
char header[80];			// An 80 byte string that is written to the start of an STL file acording to the protocol.
FILE *inputfile, *outputfile;		// Input and output file handles
float coordinate;			// The X or Y or Z coordinate or vector that is extracted from the regex
regex_t solid_test, normal_test, vertex_test, endsolid_test;
regmatch_t line_matches[4], solid_matches[2];

regcomp( &solid_test, "solid\\s*\\([a-zA-Z0-9_\\:\\-]\\+\\)", 0 );
regcomp( &endsolid_test, "endsolid", 0 );
regcomp( &normal_test, "facet normal\\s*\\(\\-\\?[0-9].[0-9]\\+e[\\+\\|\\-][0-9]\\{2\\}\\)\\s*\\(\\-\\?[0-9].[0-9]\\+e[\\+\\|\\-][0-9]\\{2\\}\\)\\s*\\(\\-\\?[0-9].[0-9]\\+e[\\+\\|\\-][0-9]\\{2\\}\\)", 0 );
regcomp( &vertex_test, "vertex\\s*\\(\\-\\?[0-9].[0-9]\\+e[\\+\\|\\-][0-9]\\{2\\}\\)\\s*\\(\\-\\?[0-9].[0-9]\\+e[\\+\\|\\-][0-9]\\{2\\}\\)\\s*\\(\\-\\?[0-9].[0-9]\\+e[\\+\\|\\-][0-9]\\{2\\}\\)", 0 );
regcomp( &vertex_test, "vertex\\s*\\(\\-\\?[0-9].[0-9]\\+e[\\+\\|\\-][0-9]\\{2\\}\\)\\s*\\(\\-\\?[0-9].[0-9]\\+e[\\+\\|\\-][0-9]\\{2\\}\\)\\s*\\(\\-\\?[0-9].[0-9]\\+e[\\+\\|\\-][0-9]\\{2\\}\\)", 0 );

if ( argv[1] == NULL)			// Make sure that an argument is supplied to the program
{
	printf("No input file supplied\n");
	return(-3);
}
filenamesize = 0;
while( argv[1][filenamesize]) filenamesize++;
filename = argv[1];
inputfile = fopen(filename,"r");

if( inputfile == NULL )
{
	printf("Error while opening %s.\n",filename);
	return(-1);
}

fgets(line, 100, inputfile); // Get the first line of the STL

if( regexec( &solid_test, line, 2, solid_matches, 0 ) ) // Ensure that the file is ASCII by testing that header is "solid___"
{
	printf("File is not ASCII STL\n");
	return(-2);
}

solidname = malloc(solid_matches[1].rm_eo - solid_matches[1].rm_so);
strncpy(solidname, &line[solid_matches[1].rm_so], solid_matches[1].rm_eo - solid_matches[1].rm_so);
solidname[solid_matches[1].rm_eo - solid_matches[1].rm_so]= '\0';

outname = malloc(9+solid_matches[1].rm_eo - solid_matches[1].rm_so+filenamesize+1);

strncpy(outname,filename,filenamesize-4);
outname[filenamesize-4] = '-';
strcat(outname,solidname);
strcat(outname,"-bin.stl");

outputfile = fopen(outname,"wb");

//Write header to binary file
attribute=0;
strcpy(header, solidname);
strcat(header, "-File Created by Stuart Buckingham ASCII to binary STL tool                     ");
fwrite(&header, sizeof(header[0]), 80 , outputfile);
// Placeholder to fill with number of tris later
fwrite("    ", 4, 1, outputfile);

numberoftris = 0;
while( fgets(line, 100, inputfile) != NULL )
{
	if( !regexec( &normal_test, line, 4, line_matches, 0 ) )
	{
		numberoftris++;
		vertexcount = 0;
		for( i = 1; i <= 3; i++ ) {
			strncpy(gotstring, &line[line_matches[i].rm_so], line_matches[i].rm_eo - line_matches[i].rm_so + 1);
			coordinate = ScientificToFloat(gotstring);
			fwrite(&coordinate, 4, 1, outputfile);	
		}
	}
	if( !regexec( &vertex_test, line, 4, line_matches, 0 ) )
	{
		for( i = 1; i <= 3; i++ ) {
			strncpy(gotstring, &line[line_matches[i].rm_so], line_matches[i].rm_eo - line_matches[i].rm_so + 1);
			coordinate = ScientificToFloat(gotstring);
			fwrite(&coordinate, 4, 1, outputfile);
		}
		vertexcount++;
		if ( vertexcount == 3 )
		{
			fwrite(&attribute, 2, 1, outputfile); // Write the 2 bytes for Attribute byte count
		}
	}
	if( !regexec( &endsolid_test, line, 0, NULL, 0 ) )
	{
		fseek(outputfile,80,SEEK_SET);
		fwrite(&numberoftris, sizeof(numberoftris), 1, outputfile);
		fclose(outputfile);	
		printf("Solid: \"%s\" containing %d triangles written to %s\n", solidname, numberoftris, outname);
		while( fgets(line, 100, inputfile) != NULL )
			if( !regexec( &solid_test, line, 2, solid_matches, 0 ) ) // Find next occurance of the start of another solid
			{
				solidname = malloc(solid_matches[1].rm_eo - solid_matches[1].rm_so);
				strncpy(solidname, &line[solid_matches[1].rm_so], solid_matches[1].rm_eo - solid_matches[1].rm_so);
				solidname[solid_matches[1].rm_eo - solid_matches[1].rm_so]= '\0';
				outname = malloc(9+solid_matches[1].rm_eo - solid_matches[1].rm_so+filenamesize+1);
				strncpy(outname,filename,filenamesize-4);
				outname[filenamesize-4] = '-';
				outname[filenamesize-3] = '\0';
				strcat(outname,solidname);
				strcat(outname,"-bin.stl");

				outputfile = fopen(outname,"wb");

				//Write new header to binary file
				attribute=0;
				strcpy(header, solidname);
				strcat(header, "-File Created by Stuart Buckingham ASCII to binary STL tool                     ");
				fwrite(&header, sizeof(header[0]), 80 , outputfile);
				// Placeholder to fill with number of tris later
				fwrite("    ", 4, 1, outputfile);
				
				numberoftris = 0;
				break;	
			}
		
	}

} 

fclose(inputfile);
return(0);
}
