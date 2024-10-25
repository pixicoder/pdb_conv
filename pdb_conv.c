/*
    PalmOS PDB converter
    by Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru

    This file is public domain

    Changelog:
    v2.0 (25 oct 2024): all versions are combined into a single C file;
    v1.x (2002 - 2007): development of different versions for PsyTexx1, ArmZX, PsyTexx2 and other PalmOS apps.
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

const char* g_usage = "\
PalmOS PDB converter by Alexander Zolotov / WarmPlace.ru\n\
(build " __DATE__ "; " __TIME__ ")\n\
Usage:\n\
 File->PDB: pdb_conv [-w] [-bSIZE] [-tTYPE] srcfile1 [srcfile2] ...\n\
 PDB->File: pdb_conv -r pdbfile1 [pdbfile2] ...\n\
Options:\n\
 -w     write PDB (optional; default);\n\
 -r     read PDB;\n\
 -bSIZE set max block size (optional; default=54000);\n\
 -tTYPE set PDB file type (optional):\n\
        PSYX - default (PsyTexx2, SunVox);\n\
        MOD - PsyTexx1 MOD;\n\
        SNA - ArmZX;\n\
\n";

uint8_t g_pdb_header[ 76 ] = 
{
    0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    0,0,0,0, 0x3B,0x62,0xC2,0x5C, 0x3B,0x62,0xC2,0x5C, 0x3B,0x62,0xC2,0x5C,
    0,0,0,0, 0,0,0,0, 0,0,0,0, 0x50,0x53,0x59,0x58, //Type: PSYX
    0x5A,0x55,0x4C,0x55, 0,0,0,0, 0,0,0,0 //Creator: ZULU
};

#define INT32_SWAP( n ) \
    ( ( (((uint32_t)n) << 24 ) & 0xFF000000 ) | \
      ( (((uint32_t)n) << 8 ) & 0x00FF0000 ) | \
      ( (((uint32_t)n) >> 8 ) & 0x0000FF00 ) | \
      ( (((uint32_t)n) >> 24 ) & 0x000000FF ) )

#define INT16_SWAP( n ) \
    ( ( (((uint16_t)n) << 8 ) & 0xFF00 ) | \
      ( (((uint16_t)n) >> 8 ) & 0x00FF ) )

uint32_t get_file_size( const char* filename )
{
    uint32_t len;
    FILE* f = fopen( filename, "rb" );
    if( f )
    {
	fseek( f, 0, 2 );
	len = ftell( f );
	fclose( f );
	return len;
    }
    return 0;
}

char* check_output_filename( char* name )
{
    int len = strlen( name );
    char* rv = (char*)malloc( len + 256 );
    rv[ 0 ] = 0;
    strcat( rv, name );
    while( 1 )
    {
	FILE* f = fopen( rv, "rb" );
	if( f )
	{
	    //Already exists. Change filename:
	    fclose( f );
	    len = strlen( rv );
	    int i = 0;
	    for( i = len - 1; i >= 0; i-- )
	    {
		if( rv[ i ] == '.' ) break;
	    }
	    if( i == 0 )
	    {
		//No file extension:
		strcat( rv, "_" ); //FILENAME -> FILENAME_
	    }
	    else
	    {
		//FILENAME.EXT -> FILENAME_.EXT
		for( int i2 = len + 1; i2 >= i; i2-- ) rv[ i2 ] = rv[ i2 - 1 ];
		rv[ i ] = '_';
	    }
	}
	else break;
    }
    return rv;
}

void file2pdb( char* type, int block_size, char* src_filename, char* dest_filename )
{
    uint32_t size = get_file_size( src_filename );
    if( size == 0 )
    {
	printf( "ERROR: can't read %s\n", src_filename );
	return;
    }

    char* dest = check_output_filename( dest_filename );
    printf( "Converting %s (%d bytes) -> %s (type=%s) ...\n", src_filename, size, dest, type );

    //Set name:
    for( int i = 0; i < 31; i++ )
    {
        if( src_filename[ i ] == 0 ) { g_pdb_header[ i ] = 0; break; }
        g_pdb_header[ i ] = src_filename[ i ];
    }
    g_pdb_header[ 31 ] = 0;

    //Set type:
    g_pdb_header[ 60 ] = ' ';
    g_pdb_header[ 61 ] = ' ';
    g_pdb_header[ 62 ] = ' ';
    g_pdb_header[ 63 ] = ' ';
    int type_len = strlen( type );
    for( int t = 0; t < type_len && t < 4; t++ )
	g_pdb_header[ 60 + t ] = type[ t ];

    if( strcmp( type, "MOD" ) == 0 )
    {
	//PsyTexx1 MOD:
	uint8_t* mod = (uint8_t*)malloc( size );
	FILE* f = fopen( src_filename, "rb" );
	fread( mod, 1, size, f );
	fclose( f );
	bool oldMOD = 1; //old MOD with 15 samples
	int num_samples = 31;
	int num_channels = 4;
	uint8_t* mt = &mod[ 1080 ];
	if( mt[ 0 ] == 'M' && mt[ 2 ] == 'K' ) oldMOD = 0;
	if( mt[ 0 ] == 'F' && mt[ 1 ] == 'L' && mt[ 2 ] == 'T' ) oldMOD = 0;
	if( mt[ 1 ] == 'C' && mt[ 2 ] == 'H' && mt[ 3 ] == 'N' ) oldMOD = 0;
	if( mt[ 0 ] == 'C' && mt[ 1 ] == 'D' && mt[ 2 ] == '8' && mt[ 3 ] == '1' ) oldMOD = 0;
	if( mt[ 0 ] == 'O' && mt[ 1 ] == 'K' && mt[ 2 ] == 'T' && mt[ 3 ] == 'A' ) oldMOD = 0;
	if( mt[ 2 ] == 'C' && mt[ 3 ] == 'N' ) oldMOD = 0;
	if( oldMOD ) num_samples = 15;
	if( !oldMOD )
	{
    	    if( mt[ 0 ] == '6' ) num_channels = 6;
    	    if( mt[ 0 ] == '8' || mt[ 0 ] == 'O' ) num_channels = 8;
    	    if( mt[ 0 ] == 'C' ) num_channels = 12;
    	    if( mt[ 0 ] == 'X' ) num_channels = 16;
	}
	int pat_size = 4 * 64 * num_channels;
	int num_pats = 0;
	for( int i = 0; i < 128; i++ )
	{
	    int n = mod[ 952 + i ];
	    if( n > num_pats ) num_pats = n;
	}
	num_pats++;
	int sample_len[ 31 ];
	for( int i = 0, s = 0; i < 930; i += 30, s++ )
	{
	    int slen = ( (int)mod[ 42 + i ] << 8 ) | mod[ 42 + i + 1 ];
	    slen *= 2;
	    if( slen == 0 ) slen = 2;
	    sample_len[ s ] = slen;
	}
	printf( "MOD: %d channels; %d samples; %d patterns\n", num_channels, num_samples, num_pats );
	bool err = 0;
	if( num_pats * pat_size > 1024*64 )
	{
	    printf( "ERROR: MODs with more than 64kb of pattern data are not supported\n" );
	    err = 1;
	}
	if( num_samples != 31 )
	{
	    printf( "ERROR: only 31-sample MOD format is supported\n" );
	    err = 1;
	}
	if( !err )
	{
	    f = fopen( (const char*)dest, "wb" );
	    fwrite( g_pdb_header, 1, 76, f ); //write header
	    uint32_t records = 1 + 1 + 1 + 31;
	    uint16_t nrec = INT16_SWAP( records );
	    fwrite( &nrec, 1, 2, f ); //save number of records
	    uint32_t table = 8 * records; //offset table size

	    //Record table:
	    uint32_t off = 0;
	    uint32_t record[ 2 ] = { 0, 0 };
    	    record[ 0 ] = INT32_SWAP( 78 + table + off ); //END record offset
	    fwrite( record, 1, 8, f ); off += 4;
    	    record[ 0 ] = INT32_SWAP( 78 + table + off ); //MOD HEADER record offset
	    fwrite( record, 1, 8, f ); off += 1084;
    	    record[ 0 ] = INT32_SWAP( 78 + table + off ); //PATTERNS record offset
	    fwrite( record, 1, 8, f ); off += num_pats * pat_size;
	    for( int s = 0; s < 31; s++ )
	    {
    		record[ 0 ] = INT32_SWAP( 78 + table + off ); //SAMPLE record offset
		fwrite( record, 1, 8, f ); off += sample_len[ s ];
	    }

	    //Data:
	    fwrite( "END.", 1, 4, f );
	    fwrite( mod, 1, 1084, f ); //header
	    fwrite( mod + 1084, 1, num_pats * pat_size, f ); //patterns
	    off = 1084 + num_pats * pat_size;
	    for( int s = 0; s < 31; s++ )
	    {
		int slen = sample_len[ s ];
		if( slen == 2 )
		    fwrite( "  ", 1, 2, f ); //empty sample
		else
		{
		    fwrite( &mod[ off ], 1, slen, f ); //sample data
		    off += slen;
		}
	    }

	    fclose( f );
	}
	free( mod );
    }
    else
    {
	//Other apps:
	FILE* f = fopen( (const char*)dest, "wb" );
	uint32_t records = ( size / block_size ) + 1; //get number of records (blocks)
	fwrite( g_pdb_header, 1, 76, f ); //write header
	uint16_t nrec = INT16_SWAP( records );
	fwrite( &nrec, 1, 2, f ); //save number of records
	uint32_t table = 8 * records; //offset table size

	uint32_t off = 0;
	uint32_t record[ 2 ] = { 0, 0 };
        for( int i = 0; i < records; i++ )
	{
    	    record[ 0 ] = INT32_SWAP( 78 + table + off ); //record offset
	    fwrite( record, 1, 8, f );
    	    off += block_size;
	}

	FILE* f2 = fopen( src_filename, "rb" );
	for( uint32_t i = 0; i < size; i++ )
	{
    	    fputc( fgetc( f2 ), f );
	}

	fclose( f2 );
	fclose( f );
    }

    free( dest );
}

void pdb2file( char* src_filename )
{
    uint32_t size = get_file_size( src_filename );
    if( size == 0 )
    {
	printf( "ERROR: can't read %s\n", src_filename );
	return;
    }

    uint8_t pdb_header[ 76 ];

    FILE* f = fopen( (const char*)src_filename, "rb" );
    fread( pdb_header, 1, 76, f ); //header
    char dest_filename[ 33 ]; memset( dest_filename, 0, sizeof( dest_filename ) );
    memcpy( dest_filename, pdb_header, 32 );
    for( int i = 0; i < 32; i++ )
    {
	int c = dest_filename[ i ];
	if( c < 0x20 || c > 127 ) dest_filename[ i ] = 0;
    }

    char* dest = check_output_filename( dest_filename );
    printf( "Converting %s -> %s ...\n", src_filename, dest );

    uint16_t nrec = 0; fread( &nrec, 1, 2, f ); nrec = INT16_SWAP( nrec ); //number of records
    uint32_t* rec_offsets = (uint32_t*)malloc( nrec * 4 );
    uint32_t* rec_sizes = (uint32_t*)malloc( nrec * 4 );
    uint8_t* rec_buf = (uint8_t*)malloc( 1024 * 500 );
    for( uint16_t r = 0; r < nrec; r++ )
    {
	uint32_t rec[ 2 ];
	fread( rec, 1, 8, f ); //read the record
	rec_offsets[ r ] = INT32_SWAP( rec[ 0 ] );
	if( r > 0 )
	    rec_sizes[ r - 1 ] = rec_offsets[ r ] - rec_offsets[ r - 1 ];
    }
    rec_sizes[ nrec - 1 ] = size - rec_offsets[ nrec - 1 ];
    FILE* f2 = fopen( dest, "wb" );
    bool psytexx1 = 0;
    for( uint16_t r = 0; r < nrec; r++ )
    {
	uint32_t rec_off = rec_offsets[ r ];
	uint32_t rec_size = rec_sizes[ r ];
	fseek( f, rec_off, 0 );
	fread( rec_buf, 1, rec_size, f );
	if( rec_size == 4 && rec_buf[ 0 ] == 'E' && rec_buf[ 1 ] == 'N' && rec_buf[ 2 ] == 'D' && rec_buf[ 3 ] == '.' )
	{
	    //PsyTexx1 MOD: skip the first record and records of size 2
	    psytexx1 = 1;
	}
	else
	{
	    if( r >= 3 && rec_size == 2 && psytexx1 )
	    {
		//skip the record of size 2
	    }
	    else
	    {
		if( psytexx1 && r == 2 )
		{
		    //Fix the pattern data:
		    uint32_t* pat_data = (uint32_t*)rec_buf;
		    for( int x = 0; x < rec_size / 4; x++ )
		    {
			uint32_t v = pat_data[ x ];
			if( !v ) continue;
			int smp = ( v & ( 15 << 4 ) ) + ( ( v >> ( 8 * 2 + 4 ) ) & 15 );
			if( smp ) continue;
			int period = ( ( v & 15 ) << 8 ) + ( ( v >> 8 ) & 255 );
			if( !period ) continue;
			int eff = ( ( ( v >> 8 * 2 ) & 15 ) << 8 ) | ( ( v >> 8 * 3 ) & 255 );
			//if( eff ) printf( "%x %x %x\n", smp, period, eff );
			//if( ( eff >> 8 ) != 0x8 )
			{
			    v &= 0xFFFF0000; //set period to 0
			    pat_data[ x ] = v;
			}
		    }
		}
		fwrite( rec_buf, 1, rec_size, f2 );
	    }
	}
    }

    fclose( f2 );
    fclose( f );
    free( dest );
    free( rec_offsets );
    free( rec_sizes );
    free( rec_buf );
}

int main( int argc, char* argv[] )
{
    if( argc <= 1 )
    {
	printf( "%s", g_usage );
	return 0;
    }

    int mode = 0; //0 - write PDB; 1 - read PDB;
    int block_size = 54000;
    char* type = (char*)"PSYX";
    for( int i = 1; i < argc; i ++ )
    {
	char* arg = argv[ i ];
    	if( arg[ 0 ] == '-' )
    	{
    	    if( arg[ 1 ] == 't' )
    	    {
    		int len = strlen( arg );
    		if( len >= 3 )
    		{
    		    //set PDB type:
    		    type = arg + 2;
    		}
    	    }
    	    if( arg[ 1 ] == 'w' )
    	    {
    		mode = 0;
    	    }
    	    if( arg[ 1 ] == 'r' )
    	    {
    		mode = 1;
    	    }
    	    if( arg[ 1 ] == 'b' )
    	    {
    		block_size = atoi( &arg[ 2 ] );
    	    }
    	    continue;
    	}
	if( mode == 0 )
	{
	    //File -> PDB:
	    int len = strlen( arg );
	    char* output_filename = (char*)malloc( len + 16 );
	    output_filename[ 0 ] = 0;
	    strcat( output_filename, arg );
	    output_filename[ len ] = '.';
	    output_filename[ len + 1 ] = 'p';
	    output_filename[ len + 2 ] = 'd';
	    output_filename[ len + 3 ] = 'b';
	    output_filename[ len + 4 ] = 0;
	    file2pdb( type, block_size, arg, output_filename );
	    free( output_filename );
	}
	if( mode == 1 )
	{
    	    //PDB -> File:
	    pdb2file( arg );
    	}
    }
    return 0;
}
