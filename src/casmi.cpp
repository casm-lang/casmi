//  
//  Copyright (c) 2016 Philipp Paulweber
//  All rights reserved.
//  
//  Developed by: Philipp Paulweber
//                https://github.com/ppaulweber/casmi
//  
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program. If not, see <http://www.gnu.org/licenses/>.
//  

#include "stdhl/cpp/Default.h"
#include "stdhl/cpp/Args.h"

#include "Pass.h"

#include "libcasm-ir.h"

#include "analyze/AstDumpPass.h"
#include "analyze/TypeCheckPass.h"
#include "analyze/CasmIRDumpPass.h"

#include "transform/SourceToAstPass.h"
#include "transform/AstToCasmIRPass.h"


#include "version.h"
#include "license.h"


/**
    @brief TODO

    TODO
*/


int main( int argc, const char *argv[] )
{
	const char* file_name = 0;
	const char* output_name = 0;

	Args options( argc, argv, Args::ALTERNATE
	, [&file_name,&options]( const char* arg ) 
	{
		static int cnt = 0;
		cnt++;
		
		if( cnt > 1 )
		{
			options.error( 1, "to many file names passed" );
		}
		
		file_name = arg;
	});
	
	options.add( 'o', 0, Args::REQUIRED, "Place the output into <file>"
	, [&options,&output_name]( const char* option )
	{
		static int cnt = 0;
		cnt++;
		
		if( cnt > 1 )
		{
			options.error( 1, "to many output names passed" );
		}
		
		output_name = option;
	}
	, "file");

#define DESCRIPTION											\
	"Corinthian Abstract State Machine (CASM) Interpreter\n"
	
	options.add
	( 'h', "help", Args::NONE, "Display the program usage and synoptis"
	, [&options]( const char* option )
	{
		fprintf
		( stderr
		, DESCRIPTION
		  "\n"
		  "usage: %s [options] <file>\n"
		  "\n"
		  "options:\n"
		, options.getProgramName()
		);
		
		options.usage();
		
		exit( 0 );
	});
	
	options.add( 'v', "version", Args::NONE, "Display interpreter version information"
	, [&options]( const char* option )
	{
		fprintf
		( stderr
		, DESCRIPTION
		  "\n"
		  "%s: version: %s [ %s %s ]\n"
		  "\n"
		  "%s\n"
		, options.getProgramName()
		, VERSION
		, __DATE__
		, __TIME__
		, LICENSE
		);
		
		exit( 0 );
	});
	
	for( auto& p : libpass::PassRegistry::getRegisteredPasses() )
	{
		//PassId    id = p.first;
		libpass::PassInfo& pi = *p.second;
		
		if( pi.getPassArgChar() == 0 && pi.getPassArgString() == 0 )
		{
			// internal pass, do not register a cmd line flag
			continue;
		}
		
		options.add
	   	( pi.getPassArgChar()
		, pi.getPassArgString()
		, Args::NONE
		, pi.getPassDescription()
		, [&pi]( const char* option )
		{
			printf( "add: %s:%s\n", pi.getPassName(), option );
			// add to PassManager the selected pass to run!
		});
	}
	
	options.parse();
	
	if( !file_name )
	{
		options.error( 1, "no input file provided" );
	}
	
	libpass::PassResult x;
	x.getResults()[ 0 ] = (void*)file_name;
	x.getResults()[ (void*)1 ] = (void*)output_name;
	
	libcasm_ir::SourceToAstPass a;
	libcasm_ir::TypeCheckPass b;
	libcasm_ir::AstDumpPass c;
	libcasm_ir::AstToCasmIRPass d; 
	libcasm_ir::CasmIRDumpPass e; 
	
	if( !a.run( x ) )
	{
		return -1;
	}

	if( !b.run( x ) )
	{
		return -1;
	}
	
	c.run( x );
    
	d.run( x );
	
	printf( "\n===--- DUMPING CASM IR ---===\n" );
	e.run( x );
	
    return 0;
}



//  
//  Local variables:
//  mode: c++
//  indent-tabs-mode: t
//  c-basic-offset: 4
//  tab-width: 4
//  End:
//  vim:noexpandtab:sw=4:ts=4:
//  
