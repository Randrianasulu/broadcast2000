typedef struct stream_element {
	FILE *file;
	FILE *units;
	unsigned char type;
	unsigned char nstream;
	union struc {
		Video_struc video;
		Audio_struc audio;
	};
	union au_struc {
		Vaunit_struc video;
		Aaunit_struc audio;
	};
	unsigned int rate;
	double delay;
	double next_clock_cycles;
	unsigned long delay_ms;
	unsigned char start; /* picture or audio frame */
	unsigned int bytes;
	unsigned int nsec;
	Timecode_struc SCR_delay;
	Timecode_struc next_SCR;
	Buffer_struc buffer;
	unsigned long buffer_size;
	struct stream_element *next;
} stream_element_t;
