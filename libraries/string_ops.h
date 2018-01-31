#ifndef STRINGOPS_H
#define STRINGOPS_H

std::string strtoupper( std::string );

int strToCharArr( std::string, char* );
std::string charArrToStr( char* );
int strToByteArr( std::string, unsigned char*, int );
std::string byteArrToStr( unsigned char* );

#endif // STRINGOPS_H
