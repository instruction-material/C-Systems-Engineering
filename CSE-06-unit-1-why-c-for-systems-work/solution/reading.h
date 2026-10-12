#ifndef READING_H
#define READING_H

/* Return -1 below 0, 0 from 0 through 100, and 1 above 100. */
int classify_reading(int value);

/* Categories -1, 0, 1 map to below, within, above; others map to invalid. */
/* The unfinished learner implementation returns NULL. */
const char* reading_label(int category);

#endif
