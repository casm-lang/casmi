//
//  Copyright (c) 2014-2017 CASM Organization
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

#include "license.h"
#include "version.h"

#include "libpass.h"
#include "libstdhl.h"

#include "libcasm-fe.h"
#include "libcasm-ir.h"
#include "libcasm-tc.h"

/**
    @brief TODO

    TODO
*/

int main( int argc, const char* argv[] )
{
    const char* file_name = 0;
    u1 flag_dump_updates = false;

    libstdhl::Log::DefaultSource = libstdhl::Log::Source(
        [&argv]( void* args ) -> const char* { return argv[ 0 ]; } );

    libstdhl::Args options( argc, argv, libstdhl::Args::DEFAULT,
        [&file_name, &options]( const char* arg ) {
            static int cnt = 0;
            cnt++;

            if( cnt > 1 )
            {
                options.m_error( 1, "too many file names passed" );
            }

            file_name = arg;
        } );

    options.add( 't', "test-case-profile", libstdhl::Args::NONE,
        "Display the unique test profile identifier and exit.",
        [&options]( const char* option ) {
            printf( "%s\n",
                libcasm_tc::Profile::get( libcasm_tc::Profile::INTERPRETER ) );
            exit( 0 );
        } );

#define DESCRIPTION "Corinthian Abstract State Machine (CASM) Interpreter\n"

    options.add( 'h', "help", libstdhl::Args::NONE,
        "Display the program usage and synopsis and exit.",
        [&options]( const char* option ) {
            fprintf( stderr, DESCRIPTION
                "\n"
                "usage: %s [options] <file>\n"
                "\n"
                "options:\n",
                options.programName() );

            options.m_usage();

            exit( 0 );
        } );

    options.add( 'v', "version", libstdhl::Args::NONE,
        "Display interpreter version information",
        [&options]( const char* option ) {
            fprintf( stderr, DESCRIPTION
                "\n"
                "%s: version: %s [ %s %s ]\n"
                "\n"
                "%s",
                options.programName(), VERSION, __DATE__, __TIME__, LICENSE );

            exit( 0 );
        } );

    options.add( 'd', "dump-updates", libstdhl::Args::NONE,
        "TBD DESCRIPTION dump updates (updateset)",
        [&flag_dump_updates](
            const char* option ) { flag_dump_updates = true; } );

    for( auto& p : libpass::PassRegistry::registeredPasses() )
    {
        // PassId    id = p.first;
        libpass::PassInfo& pi = *p.second;

        if( pi.argChar() == 0 && pi.argString() == 0 )
        {
            // internal pass, do not register a cmd line flag
            continue;
        }

        options.add( pi.argChar(), pi.argString(), libstdhl::Args::NONE,
            pi.description(), pi.argAction() );
    }

    options.parse();

    if( !file_name )
    {
        options.m_error( 1, "no input file provided" );
    }

    // TODO: FIXME: the following code should be implemented in the PassManager
    // structure
    // to allow dynamic and possible pass calls etc.

    libpass::PassResult x;

    x.results()[ (void*)2 ]
        = (void*)flag_dump_updates; // TODO: PPA: this will be removed and
                                    // changed to a pass setter option

    auto load_file_pass = std::static_pointer_cast< libpass::LoadFilePass >(
        libpass::PassRegistry::passInfo< libpass::LoadFilePass >()
            .constructPass() );
    load_file_pass->setFileName( file_name );
    if( not load_file_pass->run( x ) )
    {
        return -1;
    }

    libpass::PassInfo src_to_ast
        = libpass::PassRegistry::passInfo< libcasm_fe::SourceToAstPass >();
    if( src_to_ast.constructPass()->run( x ) )
    {
        if( src_to_ast.isArgSelected() )
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }

    libpass::PassInfo ast_check
        = libpass::PassRegistry::passInfo< libcasm_fe::TypeCheckPass >();
    if( ast_check.constructPass()->run( x ) )
    {
        if( ast_check.isArgSelected() )
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }

    libpass::PassInfo ast_dump
        = libpass::PassRegistry::passInfo< libcasm_fe::AstDumpPass >();
    if( ast_dump.isArgSelected() )
    {
        if( not ast_dump.constructPass()->run( x ) )
        {
            return -1;
        }
    }

    libpass::PassInfo ast_exec_sym = libpass::PassRegistry::
        passInfo< libcasm_fe::SymbolicExecutionPass >();
    if( ast_exec_sym.isArgSelected() )
    {
        return ast_exec_sym.constructPass()->run( x ) ? 0 : -1;
    }

    libpass::PassInfo ast_exec_num
        = libpass::PassRegistry::passInfo< libcasm_fe::NumericExecutionPass >();

    libpass::PassInfo ast_to_ir
        = libpass::PassRegistry::passInfo< libcasm_fe::AstToCasmIRPass >();

    libpass::PassInfo ir_dump
        = libpass::PassRegistry::passInfo< libcasm_ir::CasmIRDumpPass >();

    if( ast_exec_num.isArgSelected() )
    {
        return ast_exec_num.constructPass()->run( x ) ? 0 : -1;
    }

    if( not ast_to_ir.isArgSelected() and not ir_dump.isArgSelected() )
    {
        libstdhl::Log::info( "no command provided, using '--ast-exec-num'" );
        return ast_exec_num.constructPass()->run( x ) ? 0 : -1;
    }

    if( ast_to_ir.constructPass()->run( x ) )
    {
        if( ast_to_ir.isArgSelected() )
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }

    if( ir_dump.isArgSelected() )
    {
        return ir_dump.constructPass()->run( x ) ? 0 : -1;
    }

    libstdhl::Log::error( "no valid command provided!" );
    return -1;
}

//
//  Local variables:
//  mode: c++
//  indent-tabs-mode: nil
//  c-basic-offset: 4
//  tab-width: 4
//  End:
//  vim:noexpandtab:sw=4:ts=4:
//
