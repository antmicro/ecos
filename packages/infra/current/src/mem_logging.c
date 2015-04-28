

#include <stdio.h>
#include <cyg/infra/mem_logging.h>

#warning ### TUPN.ADD ### For test logging ###

#define LOG_LINES		2048
#define LOG_CHARS		 128

#if LOG_CHARS < 4
#error LOG_CHARS error
#endif

static char g_log_buff[LOG_LINES][LOG_CHARS] ;
static int  g_nLine = 0;
static int  g_nPos = 0;


static void mem_write_next_char(char ch)
{
	if( ch=='\r') return;
	if( ch=='\n')
	{
		g_nLine++;				//Next line
		g_nLine%=LOG_LINES;
		g_nPos = 0;				//Start of line
		return;
	}

	if( g_nPos >= LOG_CHARS )
	{
		g_nLine++;				//Next line
		g_nLine%=LOG_LINES;
		g_nPos = 0;				//Start of line

		g_log_buff[ g_nLine ][ g_nPos++ ] = '.';
		g_log_buff[ g_nLine ][ g_nPos++ ] = '.';
		g_log_buff[ g_nLine ][ g_nPos++ ] = '.';
	}
	g_log_buff[ g_nLine ][ g_nPos++ ] = ch;
}

void mem_logging_write_char(char ch)
{
	if ( ch == '\n' )
	{
		mem_write_next_char('\r');
	}
	
	mem_write_next_char( ch );
}

void mem_logging_write_string(char* psz)
{
	while(*psz)	mem_logging_write_char( *psz++ );
}
