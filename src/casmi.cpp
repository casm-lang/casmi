//
//  Copyright (c) 2014-2017 CASM Organization
//  All rights reserved.
//
//  Developed by: Philipp Paulweber
//                Emmanuel Pescosta
//                Florian Hahn
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
// #include "libcasm-rt.h"
#include "libcasm-tc.h"

/**
    @brief TODO

    TODO
*/

#define DESCRIPTION "Corinthian Abstract State Machine (CASM) Interpreter\n"

int main( int argc, const char* argv[] )
{
    libpass::PassManager pm;
    libstdhl::Logger log( pm.stream() );
    log.setSource(
        libstdhl::make< libstdhl::Log::Source >( argv[ 0 ], DESCRIPTION ) );

    auto flush = [&pm]() {
        libstdhl::Log::StringFormatter f;
        libstdhl::Log::OutputStreamSink c( std::cerr, f );
        pm.stream().flush( c );
    };

    const char* file_name = 0;
    u1 flag_dump_updates = false;

    libstdhl::Args options( argc, argv, libstdhl::Args::DEFAULT,
        [&file_name, &log]( const char* arg ) {
            static int cnt = 0;
            cnt++;

            if( cnt > 1 )
            {
                log.error( "too many file names passed" );
                return 1;
            }

            file_name = arg;
            return 0;
        } );

    options.add( 't', "test-case-profile", libstdhl::Args::NONE,
        "Display the unique test profile identifier and exit.",
        [&options]( const char* option ) {
            printf( "%s\n",
                libcasm_tc::Profile::get( libcasm_tc::Profile::INTERPRETER ) );
            return -1;
        } );

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
            return -1;
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
            return -1;
        } );

    options.add( 'd', "dump-updates", libstdhl::Args::NONE,
        "TBD DESCRIPTION dump updates (updateset)",
        [&flag_dump_updates]( const char* option ) {
            flag_dump_updates = true;
            return 0;
        } );

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

    if( auto ret = options.parse( log ) )
    {
        flush();

        if( ret >= 0 )
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    if( !file_name )
    {
        log.error( "no input file provided" );
        flush();
        return 2;
    }

    // register all wanted passes
    // and configure their setup hooks if desired

    pm.add< libpass::LoadFilePass >(
        [&file_name]( libpass::LoadFilePass& pass ) {
            pass.setFilename( file_name );

        } );

    pm.add< libcasm_fe::SourceToAstPass >();
    pm.add< libcasm_fe::TypeCheckPass >();
    pm.add< libcasm_fe::AstDumpDotPass >();
    pm.add< libcasm_fe::AstDumpSourcePass >();
    // pm.add< libcasm_fe::NumericExecutionPass >(
    //     [&flag_dump_updates]( libcasm_fe::NumericExecutionPass& pass ) {
    //         pass.setDumpUpdates( flag_dump_updates );

    //     } );
    // pm.add< libcasm_fe::SymbolicExecutionPass >();
    pm.add< libcasm_fe::AstToCasmIRPass >();

    pm.add< libcasm_ir::ConsistencyCheckPass >();
    pm.add< libcasm_ir::IRDumpDebugPass >();
    pm.add< libcasm_ir::IRDumpDotPass >();
    pm.add< libcasm_ir::IRDumpSourcePass >();
    // pm.add< libcasm_ir::BranchEliminationPass >();
    // pm->add< libcasm_ir::ConstantFoldingPass >();

    // pm.setDefaultPass< libcasm_fe::NumericExecutionPass >();

    int result = 0;

    try
    {
        pm.run( flush );
    }
    catch( std::exception& e )
    {
        log.error( "pass manager triggered an exception: '"
                   + std::string( e.what() )
                   + "'" );
        result = -1;
    }

    flush();

    return result;
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
