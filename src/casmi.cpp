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

#include "license.h"
#include "version.h"

#include "libpass.h"
#include "libstdhlcpp.h"

#include "libcasm-fe.all.h"
#include "libcasm-ir.all.h"
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
                options.error( 1, "to many file names passed" );
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
                options.getProgramName() );

            options.usage();

            exit( 0 );
        } );

    options.add( 'v', "version", libstdhl::Args::NONE,
        "Display interpreter version information",
        [&options]( const char* option ) {
            fprintf( stderr, DESCRIPTION
                "\n"
                "%s: version: %s [ %s %s ]\n"
                "\n"
                "%s\n",
                options.getProgramName(), VERSION, __DATE__, __TIME__,
                LICENSE );

            exit( 0 );
        } );

    options.add( 'd', "dump-updates", libstdhl::Args::NONE,
        "TBD DESCRIPTION dump updates (updateset)",
        [&flag_dump_updates](
            const char* option ) { flag_dump_updates = true; } );

    for( auto& p : libpass::PassRegistry::getRegisteredPasses() )
    {
        // PassId    id = p.first;
        libpass::PassInfo& pi = *p.second;

        if( pi.getPassArgChar() == 0 && pi.getPassArgString() == 0 )
        {
            // internal pass, do not register a cmd line flag
            continue;
        }

        options.add( pi.getPassArgChar(), pi.getPassArgString(),
            libstdhl::Args::NONE, pi.getPassDescription(),
            pi.getPassArgAction() );
    }

    options.parse();

    if( !file_name )
    {
        options.error( 1, "no input file provided" );
    }

    // TODO: FIXME: the following code should be implemented in the PassManager
    // structure
    // to allow dynamic and possible pass calls etc.

    libpass::PassResult x;

    x.getResults()[ (void*)2 ]
        = (void*)flag_dump_updates; // TODO: PPA: this will be removed and
                                    // changed to a pass setter option

    libpass::LoadFilePass& load_file_pass
        = static_cast< libpass::LoadFilePass& >(
            *libpass::PassRegistry::getPassInfo< libpass::LoadFilePass >()
                 .constructPass() );
    if( not load_file_pass.setFileName( file_name ).run( x ) )
    {
        return -1;
    }

    libpass::PassInfo ast_parse
        = libpass::PassRegistry::getPassInfo< libcasm_fe::SourceToAstPass >();
    if( ast_parse.constructPass()->run( x ) )
    {
        if( ast_parse.isPassArgSelected() )
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }

    libpass::PassInfo ast_check
        = libpass::PassRegistry::getPassInfo< libcasm_fe::TypeCheckPass >();
    if( ast_check.constructPass()->run( x ) )
    {
        if( ast_check.isPassArgSelected() )
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }

    libpass::PassInfo ast_dump
        = libpass::PassRegistry::getPassInfo< libcasm_fe::AstDumpPass >();
    if( ast_dump.isPassArgSelected() )
    {
        if( not ast_dump.constructPass()->run( x ) )
        {
            return -1;
        }
    }

    libpass::PassInfo ast_exec_sym = libpass::PassRegistry::
        getPassInfo< libcasm_fe::SymbolicExecutionPass >();
    if( ast_exec_sym.isPassArgSelected() )
    {
        return ast_exec_sym.constructPass()->run( x ) ? 0 : -1;
    }

    libpass::PassInfo ast_exec_num = libpass::PassRegistry::
        getPassInfo< libcasm_fe::NumericExecutionPass >();
    if( not ast_exec_num.isPassArgSelected() )
    {
        if( ast_dump.isPassArgSelected() )
        {
            return 0;
        }
        options.info( "no command provided, using '--ast-exec-num'" );
    }
    return ast_exec_num.constructPass()->run( x ) ? 0 : -1;

    // libcasm_ir::AstToCasmIRPass ast2ir;
    // ast2ir.run( x );

    // libcasm_ir::CasmIRDumpPass ir_dump;
    // printf( "\n===--- DUMPING CASM IR ---===\n" );
    // ir_dump.run( x );
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
