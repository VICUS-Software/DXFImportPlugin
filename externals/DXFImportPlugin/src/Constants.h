#ifndef CONSTANTS_H
#define CONSTANTS_H

/*! Defines an invalid id */
extern unsigned int INVALID_ID;

extern const char * XML_READ_ERROR;
extern const char * XML_READ_UNKNOWN_ATTRIBUTE;
extern const char * XML_READ_UNKNOWN_ELEMENT;
extern const char * XML_READ_UNKNOWN_NAME;


const double MAX_SEGMENT_ARC_LENGHT			= 30;
const unsigned int SEGMENT_COUNT_ARC		= 30;
const unsigned int SEGMENT_COUNT_CIRCLE		= 30;
const unsigned int SEGMENT_COUNT_ELLIPSE	= 30;

// Multiplyer for different layers and their heights
const double Z_MULTIPLYER					= 0.00000;
// default line width
const double DEFAULT_LINE_WEIGHT			= 0.05;
// multiplier to apply to width of entities
const double DEFAULT_LINE_WEIGHT_SCALING	= 0.005;

#endif // CONSTANTS_H
