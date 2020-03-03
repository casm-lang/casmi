//
//  Copyright (C) 2014-2021 CASM Organization <https://casm-lang.org>
//  All rights reserved.
//
//  Developed by: Philipp Paulweber
//                Emmanuel Pescosta
//                Florian Hahn
//                <https://github.com/casm-lang/casmi>
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

#include <casmi/Version>

#include <libcasm-fe/libcasm-fe>
#include <libcasm-ir/libcasm-ir>
#include <libpass/libpass>
#include <libstdhl/libstdhl>

/**
    @brief TODO

    TODO
*/

int main( int argc, const char* argv[] )
{
    assert( argc > 0 );
    const std::string app_name = argv[ 0 ];

    libpass::PassManager pm;
    libstdhl::Logger log( pm.stream() );
    log.setSource(
        libstdhl::Memory::make< libstdhl::Log::Source >( app_name, casmi::DESCRIPTION ) );

    auto flush = [ &pm, &app_name ]() {
        libstdhl::Log::ApplicationFormatter f( app_name );
        libstdhl::Log::OutputStreamSink c( std::cerr, f );
        pm.stream().flush( c );
    };

    std::vector< std::string > files;
    std::vector< std::string > outputPath;
    // u1 flag_dump_updates = false;

    libstdhl::Args options(
        argc, argv, libstdhl::Args::DEFAULT, [ &files, &log ]( const char* arg ) {
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
        []( const char* ) {
            std::cout << casmi::PROFILE << "\n";
            return -1;
        } );

    options.add(
        'h',
        "help",
        libstdhl::Args::NONE,
        "display usage and synopsis",
        [ &log, &options ]( const char* ) {
            log.output(
                "\n" + casmi::DESCRIPTION + "\n" + log.source()->name() +
                ": usage: [options] <file>\n" + "\n" + "options: \n" + options.usage() + "\n" );

            return -1;
        } );

    options.add(
        'v',
        "version",
        libstdhl::Args::NONE,
        "display version information",
        [ &log ]( const char* ) {
            log.output(
                "\n" + casmi::DESCRIPTION + "\n" + log.source()->name() + ": version: " +
                casmi::REVTAG + " [ " + __DATE__ + " " + __TIME__ + " ]\n" + "\n" + casmi::NOTICE );

            return -1;
        } );

    options.add(
        'o',
        "output",
        libstdhl::Args::REQUIRED,
        "define an output <path>",
        [ &log, &outputPath ]( const char* arg ) {
            if( outputPath.size() > 0 )
            {
                log.error( "too many output paths defined" );
                return 1;
            }

            outputPath.emplace_back( arg );
            return 0;
        },
        "path" );

    u1 parse_debug = false;
    options.add(
        "grammar-debug",
        libstdhl::Args::NONE,
        "display the internal grammar debug information",
        [ & ]( const char* option ) {
            parse_debug = true;
            return 0;
        } );

    // options.add(
    //     'd',
    //     "dump-updates",
    //     libstdhl::Args::NONE,
    //     "TBD DESCRIPTION dump updates (updateset)",
    //     [&flag_dump_updates]( const char* option ) {
    //         flag_dump_updates = true;
    //         return 0;
    //     } );

    // add passes to the pass manager to setup command-line options

    pm.add< libcasm_fe::CstDumpPass >();
    pm.add< libcasm_fe::CstEmitPass >();
    pm.add< libcasm_fe::AstDumpPass >();
    pm.add< libcasm_fe::LstDumpPass >();
    pm.add< libcasm_fe::NumericExecutionPass >();
    pm.add< libcasm_fe::SymbolicExecutionPass >();

    for( auto id : pm.passes() )
    {
        libpass::PassInfo& pi = libpass::PassRegistry::passInfo( id );

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

    // parse the command-line

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

    // set default settings

    libpass::PassResult pr;
    pr.setInput< libpass::LoadFilePass >( files.front() );

    pm.setDefaultResult( pr );
    pm.setDefaultPass< libcasm_fe::NumericExecutionPass >();

    // set pass-specific configurations

    pm.set< libcasm_fe::SourceToCstPass >(
        [ & ]( libcasm_fe::SourceToCstPass& pass ) { pass.setDebug( parse_debug ); } );

    pm.set< libcasm_fe::NumericExecutionPass >( [ & ]( libcasm_fe::NumericExecutionPass& pass ) {
        // pass.setDumpUpdates( flag_dump_updates );
    } );

    pm.set< libcasm_fe::SymbolicExecutionPass >( [ & ]( libcasm_fe::SymbolicExecutionPass& pass ) {
        if( outputPath.size() != 0 )
        {
            pass.setOutputPath( outputPath.front() );
        }
    } );

    pm.set< libcasm_fe::CstDumpPass >( [ & ]( libcasm_fe::CstDumpPass& pass ) {
        if( outputPath.size() != 0 )
        {
            pass.setOutputPath( outputPath.front() );
        }
    } );

    pm.set< libcasm_fe::AstDumpPass >( [ & ]( libcasm_fe::AstDumpPass& pass ) {
        if( outputPath.size() != 0 )
        {
            pass.setOutputPath( outputPath.front() );
        }
    } );

    pm.set< libcasm_fe::LstDumpPass >( [ & ]( libcasm_fe::LstDumpPass& pass ) {
        if( outputPath.size() != 0 )
        {
            pass.setOutputPath( outputPath.front() );
        }
    } );

    // run pass pipeline

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
