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

#include "License.h"
#include "casmi/Version"

#include <libcasm-fe/libcasm-fe>
#include <libcasm-ir/libcasm-ir>
#include <libpass/libpass>
#include <libstdhl/libstdhl>
// #include <libcasm-rt/libcasm-rt>

/**
    @brief TODO

    TODO
*/

static const std::string DESCRIPTION = "Corinthian Abstract State Machine (CASM) Interpreter\n";
static const std::string PROFILE = "casmi";

int main( int argc, const char* argv[] )
{
    assert( argc > 0 );
    const std::string app_name = argv[ 0 ];

    libpass::PassManager pm;
    libstdhl::Logger log( pm.stream() );
    log.setSource( libstdhl::Memory::make< libstdhl::Log::Source >( app_name, DESCRIPTION ) );

    auto flush = [&pm, &app_name]() {
        libstdhl::Log::ApplicationFormatter f( app_name );
        libstdhl::Log::OutputStreamSink c( std::cerr, f );
        pm.stream().flush( c );
    };

    std::vector< std::string > files;
    u1 flag_dump_updates = false;

    libstdhl::Args options( argc, argv, libstdhl::Args::DEFAULT, [&files, &log]( const char* arg ) {

        if( files.size() > 0 )
        {
            log.error(
                "too many files, input file '" + files.front() +
                "' cannot be combined with file '" + arg + "'" );
            return 1;
        }

        files.emplace_back( arg );
        return 0;
    } );

    options.add(
        't',
        "test-case-profile",
        libstdhl::Args::NONE,
        "display the unique test profile identifier",
        []( const char* option ) {
            std::cout << PROFILE << "\n";
            return -1;
        } );

    options.add(
        'h',
        "help",
        libstdhl::Args::NONE,
        "display usage and synopsis",
        [&log, &options]( const char* option ) {

            log.output(
                "\n" + DESCRIPTION + "\n" + log.source()->name() + ": usage: [options] <file>\n" +
                "\n" + "options: \n" + options.usage() + "\n" );

            return -1;
        } );

    options.add(
        'v',
        "version",
        libstdhl::Args::NONE,
        "display version information",
        [&log]( const char* option ) {

            log.output(
                "\n" + DESCRIPTION + "\n" + log.source()->name() + ": version: " + casmi::REVTAG +
                " [ " + __DATE__ + " " + __TIME__ + " ]\n" + "\n" + LICENSE );

            return -1;
        } );

    u1 ast_parse_debug = false;
    options.add(
        "ast-parse-debug",
        libstdhl::Args::NONE,
        "display the internal parser debug information",
        [&]( const char* option ) {

            ast_parse_debug = true;
            return 0;
        } );

    options.add(
        'd',
        "dump-updates",
        libstdhl::Args::NONE,
        "TBD DESCRIPTION dump updates (updateset)",
        [&flag_dump_updates]( const char* option ) {

            flag_dump_updates = true;
            return 0;
        } );

    for( auto& p : libpass::PassRegistry::registeredPasses() )
    {
        libpass::PassInfo& pi = *p.second;

        if( pi.argChar() or pi.argString() )
        {
            options.add(
                pi.argChar(),
                pi.argString(),
                libstdhl::Args::NONE,
                pi.description(),
                pi.argAction() );
        }
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

    if( files.size() == 0 )
    {
        log.error( "no input file provided" );
        flush();
        return 2;
    }

    // register all wanted passes
    // and configure their setup hooks if desired

    pm.add< libpass::LoadFilePass >( [&]( libpass::LoadFilePass& pass ) {
        pass.setFilename( files.front() );

    } );

    pm.add< libcasm_fe::SourceToAstPass >(
        [&]( libcasm_fe::SourceToAstPass& pass ) { pass.setDebug( ast_parse_debug ); } );

    pm.add< libcasm_fe::AttributionPass >();
    pm.add< libcasm_fe::SymbolRegistrationPass >();
    pm.add< libcasm_fe::SymbolResolverPass >();
    pm.add< libcasm_fe::TypeInferencePass >();
    pm.add< libcasm_fe::TypeCheckPass >();
    pm.add< libcasm_fe::ConsistencyCheckPass >();
    pm.add< libcasm_fe::FrameSizeDeterminationPass >();

    pm.add< libcasm_fe::AstDumpDotPass >();
    pm.add< libcasm_fe::AstDumpSourcePass >();

    pm.add< libcasm_fe::NumericExecutionPass >(
        [&flag_dump_updates]( libcasm_fe::NumericExecutionPass& pass ) {
            // pass.setDumpUpdates( flag_dump_updates );
        } );
    // pm.add< libcasm_fe::SymbolicExecutionPass >();

    pm.add< libcasm_fe::AstToCasmIRPass >();

    pm.add< libcasm_ir::ConsistencyCheckPass >();
    pm.add< libcasm_ir::IRDumpDebugPass >();
    pm.add< libcasm_ir::IRDumpDotPass >();
    pm.add< libcasm_ir::IRDumpSourcePass >();
    // pm.add< libcasm_ir::BranchEliminationPass >();
    // pm->add< libcasm_ir::ConstantFoldingPass >();

    pm.setDefaultPass< libcasm_fe::NumericExecutionPass >();

    if( not pm.run( flush ) )
    {
        return -1;
    }

    flush();
    return 0;
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
