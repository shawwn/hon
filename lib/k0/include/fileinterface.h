#ifndef FILE_INTERFACE_H

#define FILE_INTERFACE_H

class FILE_INTERFACE;


FILE_INTERFACE * fi_fopen(const char *fname,const char *spec,void *mem=0,size_t len=0);
void             fi_fclose(FILE_INTERFACE *file);
size_t           fi_fread(void *buffer,size_t size,size_t count,FILE_INTERFACE *fph);
size_t           fi_fwrite(const void *buffer,size_t size,size_t count,FILE_INTERFACE *fph);
size_t           fi_fprintf(FILE_INTERFACE *fph,const char *fmt,...);
size_t           fi_fflush(FILE_INTERFACE *fph);
size_t           fi_fseek(FILE_INTERFACE *fph,size_t loc,size_t mode);
size_t           fi_ftell(FILE_INTERFACE *fph);
size_t           fi_fputc(char c,FILE_INTERFACE *fph);
size_t           fi_fputs(const char *str,FILE_INTERFACE *fph);
size_t           fi_feof(FILE_INTERFACE *fph);
size_t           fi_ferror(FILE_INTERFACE *fph);
void *           fi_getMemBuffer(FILE_INTERFACE *fph,size_t &outputLength);  // return the buffer and length of the file.


#endif
