#ifndef FILEHTAL_H
#define FILEHTAL_H

#include "sizes.h"
#include <stdio.h>

#define MAX_TITLE 64
#define MAX_PROPERTIES 32
#define MAX_LENGTH 1024


class HTALTag
{
public:
	HTALTag();
	~HTALTag();

	int set_delimiters(char left_delimiter, char right_delimiter);
	int reset_tag();     // clear all structures

	int read_tag(char *input, long &position, long length);

	int title_is(const char *title);        // test against title and return 1 if they match
	char *get_title();
	int get_title(const char *value);
	int test_property(const char *property, char *value);
	char *get_property_text(int number);
	int get_property_int(int number);
	float get_property_float(int number);
	char *get_property(const char *property);
	int get_property(const char *property, char *value);
	long get_property(const char *property, long default_);
#if !defined __alpha__ && !defined __ia64__ && !defined __x86_64__ && !defined __powerpc64__
	longest get_property(const char *property, longest default_);
#endif
	int get_property(const char *property, int default_);
	float get_property(const char *property, float default_);

	int set_title(const char *text);       // set the title field
	int set_property(const char *text, char *value);
	int set_property(const char *text, long value);
#if !defined __alpha__ && !defined __ia64__ && !defined __x86_64__ && !defined __powerpc64__
	int set_property(const char *text, longest value);
#endif
	int set_property(const char *text, int value);
	int set_property(const char *text, float value);
	int write_tag();

	char tag_title[MAX_TITLE];       // title of this tag

	char *tag_properties[MAX_PROPERTIES];      // list of properties for this tag
	char *tag_property_values[MAX_PROPERTIES];     // values for this tag

	int total_properties;
	int len;         // current size of the string

	char string[MAX_LENGTH];
	char temp_string[32];       // for converting numbers
	char left_delimiter, right_delimiter;
};


class FileHTAL
{
public:
	FileHTAL(char left_delimiter = '<', char right_delimiter = '>');
	~FileHTAL();

	int terminate_string();         // append the terminal 0
	int append_newline();       // append a newline to string
	int append_tag();           // append tag object
	int append_text(char const *text);
	int append_text(char const *text, long len);        // add generic text to the string

// Text array is dynamically allocated and deleted when FileHTAL is deleted
	char* read_text();         // read text, put it in *output, and return it
	int read_text_until(char *tag_end, char *output);     // store text in output until the tag is reached
	int read_tag();          // read next tag from file, ignoring any text, and put it in tag
	// return 1 on failure

	int write_to_file(char *filename);           // write the file to disk
	int write_to_file(FILE *file);           // write the file to disk
	int read_from_file(char *filename, int ignore_error = 0);          // read an entire file from disk
	int read_from_string(char *string);          // read from a string

	int reallocate_string(long new_available);     // change size of string to accomodate new output
	int set_shared_string(char *shared_string, long available);    // force writing to a message buffer
	int rewind();

	char *string;      // string that contains the actual file
	long position;    // current position in string file
	long length;      // length of string file for reading
	long available;    // possible length before reallocation
	int share_string;      // string is shared between this and a message buffer so don't delete

	HTALTag tag;
	long output_length;
	char *output;       // for reading text
	char left_delimiter, right_delimiter;
	char filename[1024];  // Filename used in the last read_from_file or write_to_file
};

#endif
