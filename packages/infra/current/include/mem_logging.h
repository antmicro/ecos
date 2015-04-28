


#ifndef MEM_LOGGING_H__INCLUDED_
#define MEM_LOGGING_H__INCLUDED_


#define MEM_LOG_WRITE_CHAR(  ch)    mem_logging_write_char(ch)
#define MEM_LOG_WRITE_STRING(psz)   mem_logging_write_string(psz)

#ifdef __cplusplus
extern "C" {
#endif
    extern void mem_logging_write_char(  char ch);
	extern void mem_logging_write_string(char* psz);
#ifdef __cplusplus	
}
#endif

#endif /* MEM_LOGGING_H__INCLUDED_ */