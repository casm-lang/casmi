//  
//  Copyright (c) 2014-2016 CASM Organization
//  All rights reserved.
//  
//  Developed by: Florian Hahn
//                Philipp Paulweber
//                Emmanuel Pescosta
//                https://github.com/casm-lang/casmi
//  
//  This file is part of casmi.
//  
//  casmi is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  casmi is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with casmi. If not, see <http://www.gnu.org/licenses/>.
//  

#include "version.h"
#include "license.h"

#include "stdhl/cpp/Default.h"
#include "stdhl/cpp/Args.h"

#include "libpass.h"
#include "libcasm-fe.all.h"
#include "libcasm-ir.all.h"
#include "libcasm-tc.h"



/**
    @brief TODO

    TODO
*/


int main( int argc, const char *argv[] )
{
    const char* file_name = 0;
    //const char* output_name = 0;

    u1 flag_ast_execute_symbolic = false;
    u1 flag_ast_dump = false;
    u1 flag_dump_updates = false;
    
    Args options
    ( argc
    , argv
    , Args::DEFAULT
    , [&file_name,&options]( const char* arg ) 
      {
          static int cnt = 0;
          cnt++;
          
          if( cnt > 1 )
          {
              options.error( 1, "to many file names passed" );
          }
          
          file_name = arg;
      }
    );
	
    options.add
    ( 't'
    , "test-case-profile"
    , Args::NONE
    , "Display the unique test profile identifier and exit."
    , [ &options
		//, &output_name
	  ]( const char* option )
    {
        printf( "%s\n", libcasm_tc::Profile::get( libcasm_tc::Profile::INTERPRETER ) );
        exit( 0 );
    }
    );
    
#define DESCRIPTION                                            \
    "Corinthian Abstract State Machine (CASM) Interpreter\n"
    
    options.add
    ( 'h'
    , "help"
    , Args::NONE
    , "Display the program usage and synopsis and exit."
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
    
    options.add
    ( 'v'
    , "version"
    , Args::NONE
    , "Display interpreter version information"
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
      }
    );
	
    options.add
    ( 'd'
    , "dump-updates"
    , Args::NONE
    , "TBD DESCRIPTION dump updates (updateset)"
    , [&flag_dump_updates]( const char* option )
      {
          flag_dump_updates = true;
      }
    );
	
	
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
        , [ &pi
		  , &flag_ast_dump
		  , &flag_ast_execute_symbolic
		  ]( const char* option )
        {
            // this will be later done in the pass manager implementation
            // which resolves pass dependencies etc.
            // --> for now we make a manual flag management
            
            if( pi.getPassId() == &libcasm_fe::AstDumpPass::id )
			{
			    flag_ast_dump = true;
			}
			if( pi.getPassId() == &libcasm_fe::SymbolicExecutionPass::id )
			{
			    flag_ast_execute_symbolic = true;
			}
        });
    }
    
    options.parse();
    
    if( !file_name )
    {
        options.error( 1, "no input file provided" );
    }
	
	
    // TODO: FIXME: the following code should be implemented in the PassManager structure
    // to allow dynamic and possible pass calls etc. 
    
    libpass::PassResult x;
    x.getResults()[ 0 ] = (void*)file_name;
    //x.getResults()[ (void*)1 ] = (void*)output_name;
    x.getResults()[ (void*)2 ] = (void*)flag_dump_updates;
	
    libcasm_fe::SourceToAstPass src2ast;
    if( !src2ast.run( x ) )
    {
        return -1;
    }
    
    libcasm_fe::TypeCheckPass ast_type;
    if( !ast_type.run( x ) )
    {
        return -1;
    }
	
    if( flag_ast_dump )
    {
		libcasm_fe::AstDumpPass ast_dump;
		return ast_dump.run( x ) ? 0 : -1;
    }
	
    if( flag_ast_execute_symbolic )
    {
        libcasm_fe::SymbolicExecutionPass ast_sym;
        if( not ast_sym.run( x ) )
        {
            return -1;
        }    
    }
    else
    {
        libcasm_fe::NumericExecutionPass ast_num;
        if( not ast_num.run( x ) )
        {
            return -1;
        }    
    }
    
    //libcasm_ir::AstToCasmIRPass ast2ir; 
    //ast2ir.run( x );
    
    //libcasm_ir::CasmIRDumpPass ir_dump; 
    //printf( "\n===--- DUMPING CASM IR ---===\n" );
    //ir_dump.run( x );
    
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
